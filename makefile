CC = g++
CFLAGS = -g -Wall -O3 -pthread -std=c++17 -ffast-math
TARGET = main

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)
