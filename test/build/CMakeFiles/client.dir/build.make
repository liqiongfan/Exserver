# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.11.2/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.11.2/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/josin/CLionProjects/sockets/test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/josin/CLionProjects/sockets/test/build

# Include any dependencies generated for this target.
include CMakeFiles/client.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/client.dir/flags.make

CMakeFiles/client.dir/socket_client.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/socket_client.c.o: ../socket_client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/josin/CLionProjects/sockets/test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/client.dir/socket_client.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/socket_client.c.o   -c /Users/josin/CLionProjects/sockets/test/socket_client.c

CMakeFiles/client.dir/socket_client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/socket_client.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/josin/CLionProjects/sockets/test/socket_client.c > CMakeFiles/client.dir/socket_client.c.i

CMakeFiles/client.dir/socket_client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/socket_client.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/josin/CLionProjects/sockets/test/socket_client.c -o CMakeFiles/client.dir/socket_client.c.s

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o: /Users/josin/CLionProjects/sockets/kernel/exlist.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/josin/CLionProjects/sockets/test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o   -c /Users/josin/CLionProjects/sockets/kernel/exlist.c

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/josin/CLionProjects/sockets/kernel/exlist.c > CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.i

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/josin/CLionProjects/sockets/kernel/exlist.c -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.s

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o: /Users/josin/CLionProjects/sockets/kernel/http_stream.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/josin/CLionProjects/sockets/test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o   -c /Users/josin/CLionProjects/sockets/kernel/http_stream.c

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/josin/CLionProjects/sockets/kernel/http_stream.c > CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.i

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/josin/CLionProjects/sockets/kernel/http_stream.c -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.s

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o: CMakeFiles/client.dir/flags.make
CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o: /Users/josin/CLionProjects/sockets/kernel/net/socket.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/josin/CLionProjects/sockets/test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o   -c /Users/josin/CLionProjects/sockets/kernel/net/socket.c

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/josin/CLionProjects/sockets/kernel/net/socket.c > CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.i

CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/josin/CLionProjects/sockets/kernel/net/socket.c -o CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.s

# Object files for target client
client_OBJECTS = \
"CMakeFiles/client.dir/socket_client.c.o" \
"CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o" \
"CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o" \
"CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o"

# External object files for target client
client_EXTERNAL_OBJECTS =

client: CMakeFiles/client.dir/socket_client.c.o
client: CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/exlist.c.o
client: CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/http_stream.c.o
client: CMakeFiles/client.dir/Users/josin/CLionProjects/sockets/kernel/net/socket.c.o
client: CMakeFiles/client.dir/build.make
client: CMakeFiles/client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/josin/CLionProjects/sockets/test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking C executable client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/client.dir/build: client

.PHONY : CMakeFiles/client.dir/build

CMakeFiles/client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/client.dir/clean

CMakeFiles/client.dir/depend:
	cd /Users/josin/CLionProjects/sockets/test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/josin/CLionProjects/sockets/test /Users/josin/CLionProjects/sockets/test /Users/josin/CLionProjects/sockets/test/build /Users/josin/CLionProjects/sockets/test/build /Users/josin/CLionProjects/sockets/test/build/CMakeFiles/client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/client.dir/depend

