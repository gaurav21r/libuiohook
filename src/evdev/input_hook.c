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



struct hook_info {
    int fd;
    struct libevdev *evdev;
    struct libevdev_uinput *uinput;
};

static int create_hook_info(char *path, struct hook_info **info) {
    *info = malloc(sizeof(struct hook_info));
    if (info == NULL) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for evdev buffer!\n",
                __FUNCTION__, __LINE__);

        return UIOHOOK_ERROR_OUT_OF_MEMORY;
    }

    int test = open(path, O_RDONLY | O_NONBLOCK);
    (*info)->fd = test;
    if ((*info)->fd < 0) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to open file: %s! (%d)\n",
                __FUNCTION__, __LINE__,
                path, errno);

        return UIOHOOK_FAILURE;
    }

    fprintf(stderr, "Open FD Testing: %d\n", test);


    int err = libevdev_new_from_fd((*info)->fd, &(*info)->evdev);
    if (err < 0) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to create evdev from file descriptor! (%d)\n",
                __FUNCTION__, __LINE__,
                err);

        return UIOHOOK_FAILURE;
    }

    err = libevdev_uinput_create_from_device((*info)->evdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &(*info)->uinput);
    if (err < 0) {
        logger(LOG_LEVEL_WARN, "%s [%u]: Failed to create uinput from device! (%d)\n",
                __FUNCTION__, __LINE__,
                err);

        (*info)->uinput = NULL;
    }

    char *label;
    if (libevdev_has_event_type((*info)->evdev, EV_REP) && libevdev_has_event_code((*info)->evdev, EV_KEY, KEY_ESC)) {
        label = "keyboard";
    } else if (libevdev_has_event_type((*info)->evdev, EV_REL) && libevdev_has_event_code((*info)->evdev, EV_KEY, BTN_LEFT)) {
        label = "pointing";
    } else {
        logger(LOG_LEVEL_DEBUG, "%s [%u]: Unsupported input device: %s.\n",
                __FUNCTION__, __LINE__,
                path);

        return UIOHOOK_FAILURE;
    }

    logger(LOG_LEVEL_INFO, "%s [%u]: Found %s device: %s.\n",
            __FUNCTION__, __LINE__,
            label, path);

    return UIOHOOK_SUCCESS;
}

static void destroy_hook_info(struct hook_info *info) {
    if (info != NULL) {
        if (info->fd >= 0) {
            close(info->fd);
            info->fd = -1;
        }

        if (info->evdev != NULL) {
            libevdev_free(info->evdev);
            info->evdev = NULL;
        }

        if (info->uinput) {
            libevdev_uinput_destroy(info->uinput);
            info->uinput = NULL;
        }

        free(info);
    }
}

/**********************************************************************************************************************/
#define EVENT_GLOB_PATTERN "/dev/input/event*"
static int create_glob(glob_t *glob_buffer) {
    int status = glob(EVENT_GLOB_PATTERN,  GLOB_ERR | GLOB_NOSORT | GLOB_NOESCAPE, NULL, glob_buffer);
    switch (status) {
        case GLOB_NOSPACE:
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for glob!\n",
                    __FUNCTION__, __LINE__);
            return UIOHOOK_ERROR_OUT_OF_MEMORY;

        default:
            logger(LOG_LEVEL_ERROR, "%s [%u]: Failed response for glob! (%d)\n",
                    __FUNCTION__, __LINE__,
                    status);
            return UIOHOOK_FAILURE;

        case 0:
            // Success
    }

   return UIOHOOK_SUCCESS;
}

static void destroy_glob(glob_t *glob_buffer) {
    globfree(glob_buffer);
}
/**********************************************************************************************************************/


static int create_input_devices(int epoll_fd, struct epoll_event **devices) {
    glob_t glob_buffer;
    int status = create_glob(&glob_buffer);
    if (status != UIOHOOK_SUCCESS) {
        return status;
    }

    *devices = malloc(sizeof(struct epoll_event) * glob_buffer.gl_pathc);
    if (*devices == NULL) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for epoll event devices!\n",
                __FUNCTION__, __LINE__);

        destroy_glob(&glob_buffer);
        return UIOHOOK_ERROR_OUT_OF_MEMORY;
    }

    int found = 0;
    for (int i = 0; i < glob_buffer.gl_pathc; i++) {
        fprintf(stderr, "Testing: %s... ", glob_buffer.gl_pathv[i]);

        struct hook_info *info = devices[found]->data.ptr;
        if (create_hook_info(glob_buffer.gl_pathv[i], &info) != UIOHOOK_SUCCESS) {
            // FIXME LOG Error
            fprintf(stderr, "fail\n");

            destroy_hook_info(info);
            devices[found]->data.ptr = NULL;
            continue;
        }
        fprintf(stderr, "ok\n");

        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, info->fd, devices[found]) < 0) {
            // FIXME LOG Error
            fprintf(stderr, "Failed to add file descriptor to epoll %d\n", info->fd);
            
            destroy_hook_info(info);
            devices[found]->data.ptr = NULL;
            continue;
        }

        devices[found]->events = EPOLLIN;
        found++;
    }

    destroy_glob(&glob_buffer);

    struct epoll_event *new_devices = realloc(*devices, sizeof(struct epoll_event *) * found);
    if (new_devices != NULL) {
        *devices = new_devices;
        new_devices = NULL;
    }

    return UIOHOOK_SUCCESS;
}



UIOHOOK_API int hook_run() {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        // FIXME Error
        fprintf(stderr, "Failed to create epoll file descriptor\n");

        return UIOHOOK_ERROR_EPOLL_CREATE;
    }

    struct epoll_event *devices = NULL;
    int status = create_input_devices(epoll_fd, &devices);
    if (status != UIOHOOK_SUCCESS) {
        // TODO Log Failure?
        close(epoll_fd);

        return status;
    }


    #define EVENT_BUFFER_SIZE 8
    #define EVENT_BUFFER_WAIT 30

    int event_count;
    struct epoll_event *event_buffer = malloc(sizeof(struct epoll_event) * EVENT_BUFFER_SIZE);
    if (event_buffer == NULL) {
        logger(LOG_LEVEL_ERROR, "%s [%u]: Failed to allocate memory for device buffer!\n",
                __FUNCTION__, __LINE__);

        close(epoll_fd);
        return UIOHOOK_ERROR_OUT_OF_MEMORY;
    }


    int err;
    struct input_event ev;

    while (1) {
		printf("\nPolling for input...\n");
		event_count = epoll_wait(epoll_fd, event_buffer, EVENT_BUFFER_SIZE, EVENT_BUFFER_WAIT * 1000);
		printf("%d ready events\n", event_count);
		for (int i = 0; i < event_count; i++) {
			printf("Reading file descriptor %p '%d' -- ", event_buffer[i].data.ptr, event_buffer[i].data.fd);

            struct hook_info *device = (struct hook_info *) event_buffer[i].data.ptr;

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


    logger(LOG_LEVEL_DEBUG, "%s [%u]: Something, something, something, complete.\n",
            __FUNCTION__, __LINE__);

    return UIOHOOK_SUCCESS;
}


UIOHOOK_API int hook_stop() {
    int status = UIOHOOK_FAILURE;

    // FIXME Implement

    logger(LOG_LEVEL_DEBUG, "%s [%u]: Status: %#X.\n",
            __FUNCTION__, __LINE__, status);

    return status;
}
