# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.28

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\htdocs\libuiohook

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\htdocs\libuiohook\build

# Include any dependencies generated for this target.
include CMakeFiles/ultracap_hook.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ultracap_hook.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ultracap_hook.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ultracap_hook.dir/flags.make

CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj: CMakeFiles/ultracap_hook.dir/flags.make
CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj: CMakeFiles/ultracap_hook.dir/includes_C.rsp
CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj: C:/htdocs/libuiohook/demo/ultracap_hook.c
CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj: CMakeFiles/ultracap_hook.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\htdocs\libuiohook\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj"
	C:\Users\Ramanans\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj -MF CMakeFiles\ultracap_hook.dir\demo\ultracap_hook.c.obj.d -o CMakeFiles\ultracap_hook.dir\demo\ultracap_hook.c.obj -c C:\htdocs\libuiohook\demo\ultracap_hook.c

CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing C source to CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.i"
	C:\Users\Ramanans\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\htdocs\libuiohook\demo\ultracap_hook.c > CMakeFiles\ultracap_hook.dir\demo\ultracap_hook.c.i

CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling C source to assembly CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.s"
	C:\Users\Ramanans\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\htdocs\libuiohook\demo\ultracap_hook.c -o CMakeFiles\ultracap_hook.dir\demo\ultracap_hook.c.s

# Object files for target ultracap_hook
ultracap_hook_OBJECTS = \
"CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj"

# External object files for target ultracap_hook
ultracap_hook_EXTERNAL_OBJECTS =

ultracap_hook.exe: CMakeFiles/ultracap_hook.dir/demo/ultracap_hook.c.obj
ultracap_hook.exe: CMakeFiles/ultracap_hook.dir/build.make
ultracap_hook.exe: libuiohook.dll.a
ultracap_hook.exe: CMakeFiles/ultracap_hook.dir/linkLibs.rsp
ultracap_hook.exe: CMakeFiles/ultracap_hook.dir/objects1.rsp
ultracap_hook.exe: CMakeFiles/ultracap_hook.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\htdocs\libuiohook\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable ultracap_hook.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\ultracap_hook.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ultracap_hook.dir/build: ultracap_hook.exe
.PHONY : CMakeFiles/ultracap_hook.dir/build

CMakeFiles/ultracap_hook.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\ultracap_hook.dir\cmake_clean.cmake
.PHONY : CMakeFiles/ultracap_hook.dir/clean

CMakeFiles/ultracap_hook.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\htdocs\libuiohook C:\htdocs\libuiohook C:\htdocs\libuiohook\build C:\htdocs\libuiohook\build C:\htdocs\libuiohook\build\CMakeFiles\ultracap_hook.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/ultracap_hook.dir/depend
