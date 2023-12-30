## Compiling
Prerequisites: 
 * [cmake](https://cmake.org)
 * gcc, clang or msvc
 * x11 dependencies:
   * libx11-dev
   * libxtst-dev
   * libxt-dev
   * libxinerama-dev
   * libx11-xcb-dev
   * libxkbcommon-dev
   * libxkbcommon-x11-dev
   * libxkbfile-dev 

```
$ git clone https://github.com/kwhat/libuiohook
$ cd uiohook
$ mkdir build && cd build
$ cmake -S .. -D BUILD_SHARED_LIBS=ON -D BUILD_DEMO=ON -DCMAKE_INSTALL_PREFIX=../dist
$ cmake --build . --parallel 2 --target install   
```

Added for Ultra cap
```
$ cp ultracap_hook ../../../htdocs/ultracap-new/src/main/Recorder/ultracap_hook
```

Instructions for Windows

Install MinGW and CMake for Windows.

Run
```
make -S .. -D BUILD_SHARED_LIBS=ON -D BUILD_DEMO=ON -DCMAKE_INSTALL_PREFIX=../dist -G "MinGW Makefiles"
```

Output
```
-- The C compiler identification is GNU 13.2.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: C:/Users/Ramanans/mingw64/bin/gcc.exe - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed
-- Looking for pthread_create in pthreads
-- Looking for pthread_create in pthreads - not found
-- Looking for pthread_create in pthread
-- Looking for pthread_create in pthread - found
-- Found Threads: TRUE  
-- Configuring done (166.3s)
-- Generating done (2.6s)
-- Build files have been written to: C:/htdocs/libuiohook/build

```

Then
``````
cmake --build . --parallel 2 --target install  

```
[  6%] [ 12%] Building C object CMakeFiles/uiohook.dir/src/logger.c.obj
Building C object CMakeFiles/uiohook.dir/src/windows/input_helper.c.obj
[ 18%] Building C object CMakeFiles/uiohook.dir/src/windows/input_hook.c.obj
[ 31%] [ 31%] Building C object CMakeFiles/uiohook.dir/src/windows/post_event.c.objBuilding C object CMakeFiles/uiohook.dir/src/windows/system_properties.c.obj

[ 37%] Linking C shared library libuiohook.dll
[ 37%] Built target uiohook
[ 43%] [ 50%] Building C object CMakeFiles/demo_hook.dir/demo/demo_hook.c.objBuilding C object CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj

[ 56%] Linking C executable demo_hook.exe
[ 62%] Linking C executable ultracap_hook.exe
[ 62%] Built target demo_hook
[ 68%] Building C object CMakeFiles/demo_hook_async.dir/demo/demo_hook_async.c.obj
[ 68%] Built target ultracap_hook
[ 75%] Building C object CMakeFiles/demo_post.dir/demo/demo_post.c.obj
[ 81%] Linking C executable demo_post.exe
[ 87%] Linking C executable demo_hook_async.exe
[ 87%] Built target demo_hook_async
[ 87%] Built target demo_post
[ 93%] Building C object CMakeFiles/demo_properties.dir/demo/demo_properties.c.obj
[100%] Linking C executable demo_properties.exe
[100%] Built target demo_properties
Install the project...
-- Install configuration: ""
-- Installing: C:/htdocs/libuiohook/dist/lib/libuiohook.dll.a
-- Installing: C:/htdocs/libuiohook/dist/bin/libuiohook.dll
-- Up-to-date: C:/htdocs/libuiohook/dist/include/uiohook.h
-- Installing: C:/htdocs/libuiohook/dist/lib/cmake/uiohook/uiohook-config.cmake
-- Installing: C:/htdocs/libuiohook/dist/lib/cmake/uiohook/uiohook-config-noconfig.cmake
-- Installing: C:/htdocs/libuiohook/dist/bin/ultracap_hook.exe
-- Installing: C:/htdocs/libuiohook/dist/bin/demo_hook.exe
-- Installing: C:/htdocs/libuiohook/dist/bin/demo_hook_async.exe
-- Installing: C:/htdocs/libuiohook/dist/bin/demo_post.exe
-- Installing: C:/htdocs/libuiohook/dist/bin/demo_properties.exe
-- Installing: C:/htdocs/libuiohook/dist/lib/pkgconfig/uiohook.pc
```