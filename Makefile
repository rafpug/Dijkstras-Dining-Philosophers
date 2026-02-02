CC = gcc
CFLAGS = -std=c89 -Wall -Wextra -pedantic
LDFLAGS = -lpthread 

OBJS = phil.o
TARGET = phil

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

phil.o : phil.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
