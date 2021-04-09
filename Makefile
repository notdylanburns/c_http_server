CC := gcc
CFLAGS := -Wall -g
TARGET := main

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $^
	rm -f $^
%.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	rm -rf $(TARGET) *.o

debug: $(TARGET)
	gdb --quiet -ex run -ex quit $(TARGET)

.PHONY: all clean
