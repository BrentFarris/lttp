# make       -> compile the shared library "libfoo.so"
# make clean -> remove the library file and all object files (.o)
# make all   -> clean and compile
CC = gcc

INCLUDES := -I./src

LIBS := -ldl \
	-lm \
	-lpthread \
	-lncurses

SONAME	= lttp
SRC_CLIENT	= $(wildcard ./src/*.c) \
	  $(wildcard ./src/client/*.c)

# Compilation options
#CFLAGS  = -O2 -g -W -Wall -Wno-unused-parameter -Wbad-function-cast -fPIC -std=c11
#CFLAGS  = -O2 -g -W -Wall -Wno-unused-parameter -Wbad-function-cast -fPIC -std=gnu11
CFLAGS = -O0 -g -W -Wall -Wbad-function-cast -fPIC -std=gnu11

# How to compile individual object files
OBJS	= $(SRC_CLIENT:.c=.o)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) -c $< -o $@

.PHONY: all clean

# Library compilation
$(SONAME): $(OBJS) $(SRC_CLIENT)
	$(CC) -g $(OBJS) $(INCLUDES) $(LIBS) -o $(SONAME)

# Cleaning rule
clean:
	rm -f $(OBJS) $(SONAME) *~

# additional rule
all: clean lib
