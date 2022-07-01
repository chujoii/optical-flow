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

all: unit_testing


HEADERS = *.h

# create object file
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

# ln -s ABMI.00242 directed-graph
UNIT_TESTING_SRC=unit-testing.o ../../../ABMI.00232/src/capture.o ../../../ABMI.00232/src/image.o ../../../ABMI.00232/src/speckled-band.o ../../../ABMI.00232/src/lookout.o ../../../ABMI.00232/src/gui.o ../../../ABMI.00232/src/util.o
unit_testing : $(UNIT_TESTING_SRC)
	$(CC) $(CFLAGS) $(UNIT_TESTING_SRC) $(FFMPEG) $(MATH) $(THREAD) $(GUI) $(JPEG)  -o $@

debug :
	# Cppcheck for a static code analysis
	cppcheck --enable=all --inconclusive --std=posix main.c capture.c image.c gui.c speckled-band.c util.c wavelet.c B-splines.c lookout.c net-sock.c ../../ABMI.00238/src/cw.[ch] ../../ABMI.00242/src/directed-graph.[ch] *.h
	# Valgrind for memory debugging, memory leak detection, and profiling
	rm -f valgrind.log
	valgrind --tool=memcheck --leak-check=yes --track-origins=yes --leak-check=full --show-leak-kinds=all --log-file=valgrind.log ./abmi_00232 -f w -z -d 'rtsp://admin:qwerty123@192.168.100.101/Streaming/channels/101' -c 1590,598-1536,643-50 -b 1200,400 -v 2

# doxygen
doc :
	doxygen

# installation process
#install :

# uninstall distribution
#uninstall :

# clean directory
clean :
	rm *.o unit-testing
