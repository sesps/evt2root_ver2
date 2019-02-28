CC=g++
CFLAGS=-c -g -Wall `root-config --cflags`
LDFLAGS=`root-config --glibs`
SOURCES=SPSevt2root.cpp ADCUnpacker.cpp mTDCUnpacker.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=evt2root

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm ./*.o ./evt2root
