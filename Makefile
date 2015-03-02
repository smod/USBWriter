CC = gcc
WINDRES = windres
STRIP = strip
CFLAGS = -Wall -O2 -DNDEBUG -DWINVER=0x0500
LDFLAGS = -mwindows

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o) resource.o
EXE = USBWriter.exe

all: $(EXE)
 
$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $(EXE)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

resource.o: resource.rc
	$(WINDRES) resource.rc -o resource.o

clean:
	del $(OBJ)
	del $(EXE)