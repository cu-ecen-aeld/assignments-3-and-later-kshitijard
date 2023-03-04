CC = $(CROSS_COMPILE)gcc

CFLAGS = -Wall -Werror -g
LDFLAG = -pthread -lrt 

SRCS = aesdsocket.c

#Object
OBJS = $(SRCS:.c=.o)

# Target
TARGET = aesdsocket

all: $(TARGET)
default: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAG) $^ -o $@

.PHONY: clean

clean:
	$(RM) $(TARGET) *.o

#valgrind --leak-check=yes -s --track-origins=yes ./aesdsocket
#valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose ./aesdsocket
