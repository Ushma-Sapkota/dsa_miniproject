CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -lws2_32
TARGET = campus_server
SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean