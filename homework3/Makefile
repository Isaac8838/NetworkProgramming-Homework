CC = gcc
CFLAGS = -Wno-implict-function-declearation -g -MD
LFLAGS = -lreadline -lhiredis
INC = -Iinclude

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

.PHONY: depend clean all
DEPS:= $(OBJS:.o=.d)

all: my_server

-include $(DEPS)

my_server: $(OBJS)
	@$(CC) $^ -o $@ $(LFLAGS)

obj/%.o:src/%.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@rm obj/*.o obj/*.d ./my_server