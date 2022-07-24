CC = gcc
#-std=gnu99

#CC = clang

# march:
# gcc -c -Q -march=native --help=target
# so for beagle correct: -march=armv7-a
# or auto:               -march=native
#
# -pipe: has no effect on the generated code,
#        but it makes the compilation process faster
#
# -g: produce debugging information in the operating system’s native
#     format. Debug symbols for gdb and valgrind.
#     debug info should NOT slow down your code,
#     but increase size of the executable (~KB)
# also recommended for debug: -Og -ggdb

CFLAGS = -Wall -Wextra -pedantic -O2 -pipe -D_GNU_SOURCE -g
PROFILER = -pg
MATH = -lgsl -lm
THREAD = -lpthread
GUI = -lglfw -lGL -lX11 -lXrandr -lXi -lXinerama -ldl
FFMPEG = -lavcodec -lavdevice -lavfilter -lavformat -lavutil -logg -lswscale -lx264 -lx265 -lvorbis
# -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -lXinerama -ldl
# -lglfw3 -pthread -ldl -lGLU -lGL -lrt -lXrandr -lXxf86vm -lXi -lXinerama -lX11
JPEG = -ljpeg

UNAME := $(shell uname -a)
ifneq ($(filter x86%,$(UNAME)),)
	CFLAGS += -march=native
endif
ifneq ($(filter armv7l,$(UNAME)),)
	CFLAGS += -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard
endif

all: optical_flow unit_testing


HEADERS = *.h

# create object file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(PROFILER) -c $<

# quotate all GLSL files
glsl :
	./quotate-glsl.sh

OPTICAL_FLOW_SRC=main.o capture.o image.o gui.o block-matching.o util.o
optical_flow : glsl $(OPTICAL_FLOW_SRC)
	$(CC) $(CFLAGS) $(PROFILER) $(OPTICAL_FLOW_SRC) $(FFMPEG) $(MATH) $(THREAD) $(GUI) $(JPEG)  -o $@
	echo for profile run ./optical_flow ...
	echo gprof -b optical_flow gmon.out

UNIT_TESTING_SRC=unit-testing.o block-matching.o image.o gui.o util.o
unit_testing : $(UNIT_TESTING_SRC)
	$(CC) $(CFLAGS) $(PROFILER) $(UNIT_TESTING_SRC) $(FFMPEG) $(MATH) $(THREAD) $(GUI) $(JPEG)  -o $@
	echo for profile run ./unit_testing ...
	echo gprof -b unit_testing gmon.out

debug :
	# Cppcheck for a static code analysis
	cppcheck --enable=all --inconclusive --std=posix main.c capture.c image.c gui.c util.c *.h
	# Valgrind for memory debugging, memory leak detection, and profiling
	rm -f valgrind.log
	valgrind --tool=memcheck --leak-check=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./optical_flow -f w -z -d '/dev/video0' -c 1590,598-1536,643-50 -b 1200,400 -v 2

# doxygen
doc :
	doxygen

# installation process
#install :

# uninstall distribution
#uninstall :

# clean directory
clean :
	rm *.o optical_flow unit_testing
