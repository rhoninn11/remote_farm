PROG = remote_farm

SOURCES = $(PROG).c mongoose/mongoose.c cJSON/cJSON.c
CFLAGS = -g -W -Wall -I./ -Wno-unused-function
LIBS = -lm -pthread
CC = gcc


$(PROG): $(SOURCES)
	$(CC) $(SOURCES) -o $@ $(CFLAGS) $(LIBS)

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out $(PROG)

