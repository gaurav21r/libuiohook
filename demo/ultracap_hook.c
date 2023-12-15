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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <uiohook.h>
#include <wchar.h>
#include <time.h>

long long getTimeStampInMilliseconds() {
    time_t currentTime;
    struct timespec spec;

    // Get current time in seconds since the Epoch
    currentTime = time(NULL);

    // Convert seconds to milliseconds
    long long milliseconds;

    if (clock_gettime(CLOCK_REALTIME, &spec) == 0) {
        milliseconds = spec.tv_sec * 1000LL + spec.tv_nsec / 1000000;
    } else {
        milliseconds = currentTime * 1000LL;
    }

    return milliseconds;
}

bool logger_proc(unsigned int level, const char *format, ...) {
    bool status = false;
    va_list args;
    switch (level) {
        case LOG_LEVEL_INFO:
            va_start(args, format);
            status = vfprintf(stdout, format, args) >= 0;
            va_end(args);
            break;

        case LOG_LEVEL_WARN:
        case LOG_LEVEL_ERROR:
            va_start(args, format);
            status = vfprintf(stderr, format, args) >= 0;
            va_end(args);
            break;
    }
    
    return status;
}

// NOTE: The following callback executes on the same thread that hook_run() is called 
// from.  This is important because hook_run() attaches to the operating systems
// event dispatcher and may delay event delivery to the target application.
// Furthermore, some operating systems may choose to disable your hook if it 
// takes too long to process.  If you need to do any extended processing, please 
// do so by copying the event to your own queued dispatch thread.
void dispatch_proc(uiohook_event * const event) {

    // Diabling this as weird characters like `\r\ in `keychar` 
    // interferes with JSON.parse() over at Electron

    if (event->type != EVENT_KEY_TYPED){
        char buffer[256] = { 0 };

        // JS Compatible timestamp
        long long timestamp = getTimeStampInMilliseconds();
        size_t length = snprintf(buffer, sizeof(buffer), 
                "{id:%i,when:%" PRIu64 ",mask:0x%X,time:%lld", 
                event->type, event->time, event->mask, timestamp);


        switch (event->type) {
            case EVENT_KEY_PRESSED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",keycode:%u,rawcode:0x%X,event:'KEY_PRESSED'",
                    event->data.keyboard.keycode, event->data.keyboard.rawcode);
                break;

            case EVENT_KEY_RELEASED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",keycode:%u,rawcode:0x%X,event:'KEY_RELEASED'",
                    event->data.keyboard.keycode, event->data.keyboard.rawcode);
                break;

            // Diabling this as weird characters like `\r\ in `keychar` 
            // interferes with JSON.parse() over at Electron
            // case EVENT_KEY_TYPED:
            //     snprintf(buffer + length, sizeof(buffer) - length, 
            //         ",keychar:'%lc',rawcode:%u,event:'KEY_TYPED'",
            //         (wint_t) event->data.keyboard.keychar,
            //         event->data.keyboard.rawcode);
            //     break;

            case EVENT_MOUSE_PRESSED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",x:%i,y:%i,button:%i,clicks:%i,event:'MOUSE_PRESSED'",
                    event->data.mouse.x, event->data.mouse.y,
                    event->data.mouse.button, event->data.mouse.clicks);
                break;
            case EVENT_MOUSE_RELEASED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",x:%i,y:%i,button:%i,clicks:%i,event:'MOUSE_RELEASED'",
                    event->data.mouse.x, event->data.mouse.y,
                    event->data.mouse.button, event->data.mouse.clicks);
                break;
            case EVENT_MOUSE_CLICKED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",x:%i,y:%i,button:%i,clicks:%i,event:'MOUSE_CLICKED'",
                    event->data.mouse.x, event->data.mouse.y,
                    event->data.mouse.button, event->data.mouse.clicks);
                break;
            case EVENT_MOUSE_MOVED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",x:%i,y:%i,button:%i,clicks:%i,event:'MOUSE_MOVED'",
                    event->data.mouse.x, event->data.mouse.y,
                    event->data.mouse.button, event->data.mouse.clicks);
                break;
            case EVENT_MOUSE_DRAGGED:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",x:%i,y:%i,button:%i,clicks:%i,event:'MOUSE_DRAGGED'",
                    event->data.mouse.x, event->data.mouse.y,
                    event->data.mouse.button, event->data.mouse.clicks);
                break;

            case EVENT_MOUSE_WHEEL:
                snprintf(buffer + length, sizeof(buffer) - length, 
                    ",type:%i,amount:%i,rotation:%i,event:'MOUSE_WHEEL'",
                    event->data.wheel.type, event->data.wheel.amount,
                    event->data.wheel.rotation);
                break;

            default:
                break;
        } 

        fprintf(stdout, "%s}\n",     buffer);
        // Without this, Node JS's spawn is not able to pickup information in realtime.
        fflush(stdout);
    }
    
}

int main() {
    // Set the logger callback for library output.
    hook_set_logger_proc(&logger_proc);
    
    // Set the event callback for uiohook events.
    hook_set_dispatch_proc(&dispatch_proc);

    // Start the hook and block.
    // NOTE If EVENT_HOOK_ENABLED was delivered, the status will always succeed.
    int status = hook_run();
    switch (status) {
        case UIOHOOK_SUCCESS:
            // Everything is ok.
            break;

        // System level errors.
        case UIOHOOK_ERROR_OUT_OF_MEMORY:
            logger_proc(LOG_LEVEL_ERROR, "Failed to allocate memory. (%#X)", status);
            break;


        // X11 specific errors.
        case UIOHOOK_ERROR_X_OPEN_DISPLAY:
            logger_proc(LOG_LEVEL_ERROR, "Failed to open X11 display. (%#X)", status);
            break;

        case UIOHOOK_ERROR_X_RECORD_NOT_FOUND:
            logger_proc(LOG_LEVEL_ERROR, "Unable to locate XRecord extension. (%#X)", status);
            break;

        case UIOHOOK_ERROR_X_RECORD_ALLOC_RANGE:
            logger_proc(LOG_LEVEL_ERROR, "Unable to allocate XRecord range. (%#X)", status);
            break;

        case UIOHOOK_ERROR_X_RECORD_CREATE_CONTEXT:
            logger_proc(LOG_LEVEL_ERROR, "Unable to allocate XRecord context. (%#X)", status);
            break;

        case UIOHOOK_ERROR_X_RECORD_ENABLE_CONTEXT:
            logger_proc(LOG_LEVEL_ERROR, "Failed to enable XRecord context. (%#X)", status);
            break;

            
        // Windows specific errors.
        case UIOHOOK_ERROR_SET_WINDOWS_HOOK_EX:
            logger_proc(LOG_LEVEL_ERROR, "Failed to register low level windows hook. (%#X)", status);
            break;


        // Darwin specific errors.
        case UIOHOOK_ERROR_AXAPI_DISABLED:
            logger_proc(LOG_LEVEL_ERROR, "Failed to enable access for assistive devices. (%#X)", status);
            break;

        case UIOHOOK_ERROR_CREATE_EVENT_PORT:
            logger_proc(LOG_LEVEL_ERROR, "Failed to create apple event port. (%#X)", status);
            break;

        case UIOHOOK_ERROR_CREATE_RUN_LOOP_SOURCE:
            logger_proc(LOG_LEVEL_ERROR, "Failed to create apple run loop source. (%#X)", status);
            break;

        case UIOHOOK_ERROR_GET_RUNLOOP:
            logger_proc(LOG_LEVEL_ERROR, "Failed to acquire apple run loop. (%#X)", status);
            break;

        case UIOHOOK_ERROR_CREATE_OBSERVER:
            logger_proc(LOG_LEVEL_ERROR, "Failed to create apple run loop observer. (%#X)", status);
            break;

        // Default error.
        case UIOHOOK_FAILURE:
        default:
            logger_proc(LOG_LEVEL_ERROR, "An unknown hook error occurred. (%#X)", status);
            break;
    }

    return status;
}
