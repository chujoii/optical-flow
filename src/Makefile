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
FSANITIZE = 
# FSANITIZE not work with valgrind, so comment/uncomment next string
# -fsanitize=[sanitizer_name] [additional_options] [-g] [-OX]")
# For [sanitizer_name] use one of the following:
#    address for AddressSanitizer
#    leak for LeakSanitizer
#    thread for ThreadSanitizer
#    undefined for UndefinedBehaviorSanitizer (other options are also available, see the UBSan section)
#    memory for MemorySanitizer
#FSANITIZE = -fsanitize=address -fsanitize=leak -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
PROFILER = -pg
LINKER_OPTIMIZATION = -ffunction-sections -fdata-sections -Wl,--gc-sections -Wl,--print-gc-sections
MATH = -lgsl -lm

THREAD = -lpthread
# -pthread tells the compiler to link in the pthread library as well as configure the compilation for threads.
# Using the -lpthread option only causes the pthread library to be linked - the pre-defined macros don't get defined.
# Bottom line: you should use the -pthread option.


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
	$(CC) $(CFLAGS) $(FSANITIZE) $(PROFILER) $(LINKER_OPTIMIZATION) -c $<

# quotate all GLSL files
glsl :
	./quotate-glsl.sh

OPTICAL_FLOW_SRC=main.o capture.o image.o gui.o block-matching.o util.o
optical_flow : glsl $(OPTICAL_FLOW_SRC)
	$(CC) $(CFLAGS) $(FSANITIZE) $(PROFILER) $(OPTICAL_FLOW_SRC) $(FFMPEG) $(MATH) $(THREAD) $(GUI) $(JPEG)  -o $@
	echo for profile run ./optical_flow ...
	echo gprof -b optical_flow gmon.out

UNIT_TESTING_SRC=unit-testing.o block-matching.o image.o gui.o util.o
unit_testing : $(UNIT_TESTING_SRC)
	$(CC) $(CFLAGS) $(FSANITIZE) $(PROFILER) $(UNIT_TESTING_SRC) $(FFMPEG) $(MATH) $(THREAD) $(GUI) $(JPEG)  -o $@
	echo for profile run ./unit_testing ...
	echo gprof -b unit_testing gmon.out

debug :
	# Cppcheck for a static code analysis
	cppcheck --enable=all --inconclusive --std=posix main.c capture.c image.c gui.c util.c *.h
	# Valgrind for memory debugging, memory leak detection, and profiling
	rm -f valgrind.log
	valgrind --tool=memcheck --leak-check=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./optical_flow -d '/dev/video0' -v 2
	echo gprof -b optical_flow gmon.out | less

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
