/* libUIOHook: Cross-platform keyboard and mouse hooking from userland.
 * Copyright (C) 2006-2023 Alexander Barker.  All Rights Reserved.
 * https://github.com/kwhat/libuiohook/
 *
 * libUIOHook is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libUIOHook is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <inttypes.h>
#include <limits.h>

#include <stdio.h> // FIXME Remove
#include <glob.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>



#ifdef USE_XRECORD_ASYNC
#include <pthread.h>
#endif

#include <stdint.h>
#include <uiohook.h>

#include <xcb/xkb.h>
#include <X11/XKBlib.h>

#include <X11/keysym.h>
#include <X11/Xlibint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h>

#if defined(USE_XINERAMA) && !defined(USE_XRANDR)
#include <X11/extensions/Xinerama.h>
#elif defined(USE_XRANDR)
#include <X11/extensions/Xrandr.h>
#else
// TODO We may need to fallback to the xf86vm extension for things like TwinView.
#pragma message("*** Warning: Xinerama or XRandR support is required to produce cross-platform mouse coordinates for multi-head configurations!")
#pragma message("... Assuming single-head display.")
#endif

#include "dispatch_event.h"
#include "input_helper.h"
#include "logger.h"

// Thread and hook handles.
#ifdef USE_XRECORD_ASYNC
static bool running;

static pthread_cond_t hook_xrecord_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t hook_xrecord_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif



// Virtual event pointer.
static uiohook_event event;


void hook_event_proc(XPointer closeure, XRecordInterceptData *recorded_data) {

    XEvent event;
    wire_data_to_event(recorded_data, &event);

    XRecordDatum *data = (XRecordDatum *) recorded_data->data;
    switch (recorded_data->category) {
        case XRecordStartOfData:
            dispatch_hook_enabled((XAnyEvent *) &event);
            break;

        case XRecordEndOfData:
            dispatch_hook_disabled((XAnyEvent *) &event);
            break;

        //case XRecordFromClient: // TODO Should we be listening for Client Events?
        case XRecordFromServer:
            switch (data->type) {
                case KeyPress:
                    dispatch_key_press((XKeyPressedEvent *) &event);
                    break;

                case KeyRelease:
                    dispatch_key_release((XKeyReleasedEvent *) &event);
                    break;

                case ButtonPress:
                    dispatch_mouse_press((XButtonPressedEvent *) &event);
                    break;

                case ButtonRelease:
                    dispatch_mouse_release((XButtonReleasedEvent *) &event);
                    break;

                case MotionNotify:
                    dispatch_mouse_move((XMotionEvent *) &event);
                    break;

                case MappingNotify:
                    // FIXME
                    // event with a request member of MappingKeyboard or MappingModifier occurs
                    //XRefreshKeyboardMapping(event_map)
                    //XMappingEvent *event_map;
                    break;

                default:
                    logger(LOG_LEVEL_DEBUG, "%s [%u]: Unhandled X11 event: %#X.\n",
                            __FUNCTION__, __LINE__,
                            (unsigned int) data->type);
            }
            break;

        default:
            logger(LOG_LEVEL_WARN, "%s [%u]: Unhandled X11 hook category! (%#X)\n",
                    __FUNCTION__, __LINE__, recorded_data->category);
    }

    // TODO There is no way to consume the XRecord event.
}

static int xrecord_block() {
    int status = UIOHOOK_FAILURE;

    /*

    // Save the data display associated with this hook so it is passed to each event.
    //XPointer closeure = (XPointer) (ctrl_display);
    XPointer closeure = NULL;

    #ifdef USE_XRECORD_ASYNC
    // Async requires that we loop so that our thread does not return.
    if (XRecordEnableContextAsync(hook->data.display, context, hook_event_proc, closeure) != 0) {
        // Time in MS to sleep the runloop.
        int timesleep = 100;

        // Allow the thread loop to block.
        pthread_mutex_lock(&hook_xrecord_mutex);
        running = true;

        do {
            // Unlock the mutex from the previous iteration.
            pthread_mutex_unlock(&hook_xrecord_mutex);

            XRecordProcessReplies(hook->data.display);

            // Prevent 100% CPU utilization.
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct timespec ts;
            ts.tv_sec = time(NULL) + timesleep / 1000;
            ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timesleep % 1000);
            ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
            ts.tv_nsec %= (1000 * 1000 * 1000);

            pthread_mutex_lock(&hook_xrecord_mutex);
            pthread_cond_timedwait(&hook_xrecord_cond, &hook_xrecord_mutex, &ts);
        } while (running);

        // Unlock after loop exit.
        pthread_mutex_unlock(&hook_xrecord_mutex);

        // Set the exit status.
        status = NULL;
    }
    #else
    // Sync blocks until XRecordDisableContext() is called.
    if (XRecordEnableContext(hook->data.display, hook->ctrl.context, hook_event_proc, closeure) != 0) {
        status = UIOHOOK_SUCCESS;
    }
    #endif
    else {
        logger(LOG_LEVEL_ERROR, "%s [%u]: XRecordEnableContext failure!\n",
                __FUNCTION__, __LINE__);

        #ifdef USE_XRECORD_ASYNC
        // Reset the running state.
        pthread_mutex_lock(&hook_xrecord_mutex);
        running = false;
        pthread_mutex_unlock(&hook_xrecord_mutex);
        #endif

        // Set the exit status.
        status = UIOHOOK_ERROR_X_RECORD_ENABLE_CONTEXT;
    }
    */

    return status;
}

static int xrecord_alloc() {
    int status = UIOHOOK_FAILURE;

/*
    // Make sure the data display is synchronized to prevent late event delivery!
    // See Bug 42356 for more information.
    // https://bugs.freedesktop.org/show_bug.cgi?id=42356#c4
    XSynchronize(hook->data.display, True);

    // Setup XRecord range.
    XRecordClientSpec clients = XRecordAllClients;

    hook->data.range = XRecordAllocRange();
    if (hook->data.range != NULL) {
        logger(LOG_LEVEL_DEBUG, "%s [%u]: XRecordAllocRange successful.\n",
                __FUNCTION__, __LINE__);

        hook->data.range->device_events.first = KeyPress;
        hook->data.range->device_events.last = MappingNotify;

        // Note that the documentation for this function is incorrect,
        // hook->data.display should be used!
        // See: http://www.x.org/releases/X11R7.6/doc/libXtst/recordlib.txt
        hook->ctrl.context = XRecordCreateContext(hook->data.display, XRecordFromServerTime, &clients, 1, &hook->data.range, 1);
        if (hook->ctrl.context != 0) {
            logger(LOG_LEVEL_DEBUG, "%s [%u]: XRecordCreateContext successful.\n",
                    __FUNCTION__, __LINE__);

            // Block until hook_stop() is called.
            status = xrecord_block();

            // Free up the context if it was set.
            XRecordFreeContext(hook->data.display, hook->ctrl.context);
        } else {
            logger(LOG_LEVEL_ERROR, "%s [%u]: XRecordCreateContext failure!\n",
                    __FUNCTION__, __LINE__);

            // Set the exit status.
            status = UIOHOOK_ERROR_X_RECORD_CREATE_CONTEXT;
        }

        // Free the XRecord range.
        XFree(hook->data.range);
    } else {
        logger(LOG_LEVEL_ERROR, "%s [%u]: XRecordAllocRange failure!\n",
                __FUNCTION__, __LINE__);

        // Set the exit status.
        status = UIOHOOK_ERROR_X_RECORD_ALLOC_RANGE;
    }
*/

    return status;
}

static int xrecord_query() {
    int status = UIOHOOK_FAILURE;

    /*
    // Check to make sure XRecord is installed and enabled.
    int major, minor;
    if (XRecordQueryVersion(hook->ctrl.display, &major, &minor) != 0) {
        logger(LOG_LEVEL_DEBUG, "%s [%u]: XRecord version: %i.%i.\n",
                __FUNCTION__, __LINE__, major, minor);

        status = xrecord_alloc();
    } else {
        logger(LOG_LEVEL_ERROR, "%s [%u]: XRecord is not currently available!\n",
                __FUNCTION__, __LINE__);

        status = UIOHOOK_ERROR_X_RECORD_NOT_FOUND;
    }
    */

    return status;
}

static int xrecord_start() {
    int status = UIOHOOK_FAILURE;

    /*
    // Use the helper display for XRecord.
    hook->ctrl.display = XOpenDisplay(NULL);

    // Open a data display for XRecord.
    // NOTE This display must be opened on the same thread as XRecord.
    hook->data.display = XOpenDisplay(NULL);
    if (hook->ctrl.display != NULL && hook->data.display != NULL) {
        logger(LOG_LEVEL_DEBUG, "%s [%u]: XOpenDisplay successful.\n",
                __FUNCTION__, __LINE__);

        bool is_auto_repeat = enable_key_repeat();
        if (is_auto_repeat) {
            logger(LOG_LEVEL_DEBUG, "%s [%u]: Successfully enabled detectable auto-repeat.\n",
                    __FUNCTION__, __LINE__);
        } else {
            logger(LOG_LEVEL_WARN, "%s [%u]: Could not enable detectable auto-repeat!\n",
                    __FUNCTION__, __LINE__);
        }

        // Initialize starting modifiers.
        // FIXME initialize_modifiers(); should happen somewhere else

        status = xrecord_query();
    } else {
        logger(LOG_LEVEL_ERROR, "%s [%u]: XOpenDisplay failure!\n",
                __FUNCTION__, __LINE__);

        status = UIOHOOK_ERROR_X_OPEN_DISPLAY;
    }

    // Close down the XRecord data display.
    if (hook->data.display != NULL) {
        XCloseDisplay(hook->data.display);
        hook->data.display = NULL;
    }

    // Close down the XRecord control display.
    if (hook->ctrl.display) {
        XCloseDisplay(hook->ctrl.display);
        hook->ctrl.display = NULL;
    }
    */

    return status;
}


struct hook_info {
    struct libevdev *evdev;
    struct libevdev_uinput *uinput;
};

UIOHOOK_API int hook_run() {
    glob_t glob_buffer;
    int glob_status = glob("/dev/input/event*",  GLOB_ERR | GLOB_NOSORT | GLOB_NOESCAPE, NULL, &glob_buffer); // /dev/input/by-path/*event? TODO use a compile time variable
    switch (glob_status) {
        case GLOB_NOSPACE:
            // FIXME logging
            return UIOHOOK_ERROR_OUT_OF_MEMORY;

        case GLOB_NOMATCH:
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to locate any event devices at %s!\n",
                    __FUNCTION__, __LINE__,
                    "/dev/input");
            // FIXME No event devices found!
            return UIOHOOK_FAILURE;

        case GLOB_ABORTED:
            // FIXME logging
            return UIOHOOK_FAILURE;
    }


    struct hook_info *hook_buffer = malloc(sizeof(struct hook_info) * glob_buffer.gl_pathc);
    if (hook_buffer == NULL) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for evdev buffer!\n",
                __FUNCTION__, __LINE__);

        return UIOHOOK_ERROR_OUT_OF_MEMORY;
    }

    struct epoll_event *event_buffer = malloc(sizeof(struct epoll_event) * glob_buffer.gl_pathc);
    if (event_buffer == NULL) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for epoll event buffer!\n",
                __FUNCTION__, __LINE__);

        return UIOHOOK_ERROR_OUT_OF_MEMORY;
    }


    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        // FIXME Error
        fprintf(stderr, "Failed to create epoll file descriptor\n");
        return UIOHOOK_ERROR_EPOLL_CREATE;
    }


    int found = 0, err = 0;
    for (int i = 0; i < glob_buffer.gl_pathc; i++) {
        int fd = open(glob_buffer.gl_pathv[i], O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            // FIXME Error log errno
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to open file: %s! (%#X) (errno)\n",
                    __FUNCTION__, __LINE__,
                    glob_buffer.gl_pathv[i], O_RDONLY | O_NONBLOCK);
            continue;
        }

        err = libevdev_new_from_fd(fd, &hook_buffer[found].evdev);
        if (err < 0) {
            // FIXME Error log err
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to create evdev from file descriptor! (%d)\n",
                    __FUNCTION__, __LINE__,
                    err);
            close(fd);
            continue;
        }


        err = libevdev_uinput_create_from_device(hook_buffer[found].evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &hook_buffer[found].uinput);
        if (err < 0) {
            // FIXME Error log err
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to create uinput from device! (%d)\n",
                    __FUNCTION__, __LINE__,
                    err);

            //libevdev_free(hook_buffer[found].evdev);
            //close(fd);
            continue;
        }

        if (libevdev_has_event_type(hook_buffer[found].evdev, EV_REP) && libevdev_has_event_code(hook_buffer[found].evdev, EV_KEY, KEY_ESC)) {
            printf("Found Keyboard Device: %s\n", glob_buffer.gl_pathv[i]);
        } else if (libevdev_has_event_type(hook_buffer[found].evdev, EV_REL) && libevdev_has_event_code(hook_buffer[found].evdev, EV_KEY, BTN_LEFT)) {
            printf("Found Pointing Device: %s\n", glob_buffer.gl_pathv[i]);
        } else {
            printf("Unsupported Device: %s\n", glob_buffer.gl_pathv[i]);
            // Unsupported device, ignore.
            libevdev_uinput_destroy(hook_buffer[found].uinput);
            libevdev_free(hook_buffer[found].evdev);
            close(fd);
            continue;
        }

        event_buffer[found].events = EPOLLIN;
        event_buffer[found].data.fd = fd;
        event_buffer[found].data.ptr = &hook_buffer[found];

        printf("Testing adding %p %d\n", &hook_buffer[found], event_buffer[found].data.fd);

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event_buffer[found]))
        {
            // FIXME Error
            fprintf(stderr, "Failed to add file descriptor to epoll\n");

            // Unsupported device, ignore.
            libevdev_uinput_destroy(hook_buffer[found].uinput);
            libevdev_free(hook_buffer[found].evdev);
            close(fd);
            continue;
        }

        found++;
    }
    globfree(&glob_buffer);


    struct hook_info *hooks = realloc(hook_buffer, sizeof(struct hook_info) * found);
    if (hooks == NULL) {
        // FIXME Warning,
        hooks = hook_buffer;
    }

    struct epoll_event *devices = realloc(event_buffer, sizeof(struct epoll_event) * found);
    if (devices == NULL) {
        // FIXME Warning,
        devices = event_buffer;
    }


    // Now we implement epoll on the remaining FDs?
    struct epoll_event events[16];
    int event_count;

    struct input_event ev;
    while (1) {
		printf("\nPolling for input...\n");
		event_count = epoll_wait(epoll_fd, events, 16, 30000);
		printf("%d ready events\n", event_count);
		for (int i = 0; i < event_count; i++) {
			printf("Reading file descriptor %p '%d' -- ", events[i].data.ptr, events[i].data.fd);

            struct hook_info *device = (struct hook_info *) events[i].data.ptr;

            err = libevdev_next_event(device->evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
            if (err == LIBEVDEV_READ_STATUS_SUCCESS) {
                /*
                if (ev.type == EV_KEY && ev.code == KEY_HOME) {
                    ev.code = KEY_B;
                }
                */

                //libevdev_uinput_write_event(device->uinput, ev.type, ev.code, ev.value);
                printf("LIBEVDEV_READ_STATUS_SUCCESS\n");
            } else if (err == LIBEVDEV_READ_STATUS_SYNC) {
                do {
                    err = libevdev_next_event(device->evdev, LIBEVDEV_READ_FLAG_SYNC, &ev);
                } while (err != -EAGAIN);

                printf("LIBEVDEV_READ_STATUS_SYNC\n");
            } else if (err == -EAGAIN) {
                printf("EAGAIN\n");
                continue;
            } else {
                printf("????\n");
                // Error reading event
                break;
            }
		}
	}

	if (close(epoll_fd)) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}

    if (hook_buffer != NULL) {
        free(hook_buffer);
    }

    if (event_buffer != NULL) {
        free(event_buffer);
    }

    logger(LOG_LEVEL_DEBUG, "%s [%u]: Something, something, something, complete.\n",
            __FUNCTION__, __LINE__);

    return UIOHOOK_SUCCESS;
}


UIOHOOK_API int hook_stop() {
    int status = UIOHOOK_FAILURE;

/*
    if (hook != NULL && hook->ctrl.display != NULL && hook->ctrl.context != 0) {
        // We need to make sure the context is still valid.
        XRecordState *state = malloc(sizeof(XRecordState));
        if (state != NULL) {
            if (XRecordGetContext(hook->ctrl.display, hook->ctrl.context, &state) != 0) {
                // Try to exit the thread naturally.
                if (state->enabled && XRecordDisableContext(hook->ctrl.display, hook->ctrl.context) != 0) {
                    #ifdef USE_XRECORD_ASYNC
                    pthread_mutex_lock(&hook_xrecord_mutex);
                    running = false;
                    pthread_cond_signal(&hook_xrecord_cond);
                    pthread_mutex_unlock(&hook_xrecord_mutex);
                    #endif

                    // See Bug 42356 for more information.
                    // https://bugs.freedesktop.org/show_bug.cgi?id=42356#c4
                    //XFlush(hook->ctrl.display);
                    XSync(hook->ctrl.display, False);

                    status = UIOHOOK_SUCCESS;
                }
            } else {
                logger(LOG_LEVEL_ERROR, "%s [%u]: XRecordGetContext failure!\n",
                        __FUNCTION__, __LINE__);

                status = UIOHOOK_ERROR_X_RECORD_GET_CONTEXT;
            }

            free(state);
        } else {
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for XRecordState!\n",
                    __FUNCTION__, __LINE__);

            status = UIOHOOK_ERROR_OUT_OF_MEMORY;
        }
    }
    */

    logger(LOG_LEVEL_DEBUG, "%s [%u]: Status: %#X.\n",
            __FUNCTION__, __LINE__, status);

    return status;
}
