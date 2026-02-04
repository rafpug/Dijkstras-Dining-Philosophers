CC = gcc
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lpthread -lrt 

OBJS = dine.o
TARGET = dine

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

phil.o : phil.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
