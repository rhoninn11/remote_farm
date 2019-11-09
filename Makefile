PROG = remote_farm

SOURCES = $(PROG).c mongoose/mongoose.c cJSON/cJSON.c
CFLAGS = -g -W -Wall -I./ -Wno-unused-function
CC = gcc


$(PROG): $(SOURCES)
	$(CC) $(SOURCES) -o $@ $(CFLAGS)

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out $(PROG)

