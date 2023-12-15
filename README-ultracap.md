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