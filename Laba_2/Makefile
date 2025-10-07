CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

# Флаги линковщика: подключаем библиотеку потоков
LDFLAGS = -pthread

EXECUTABLE = parallel_trace
SOURCES = parallel_trace.cpp
all: $(EXECUTABLE)
$(EXECUTABLE): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(EXECUTABLE)

.PHONY: all clean