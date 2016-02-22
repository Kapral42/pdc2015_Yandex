prog := life_ompi
prog_objs := life_ompi.o

CC := mpicc
CFLAGS := -std=c99 -Wall -O2
LDFLAGS := -lm

.PHONY: all clean

all: $(prog)

$(prog): $(prog_objs)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

life_ompi.o: life_ompi.c

clean:
	@rm -rf *.o $(prog)
