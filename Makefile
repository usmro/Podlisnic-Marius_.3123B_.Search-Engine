CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = search_engine
SRCS = main.cpp Document.cpp Index.cpp ConsoleUI.cpp Terminal.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run