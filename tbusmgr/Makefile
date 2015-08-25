CC=gcc
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c, ./objs/%.o,$(SOURCES))

TARGET=tbusmgr

all: $(TARGET)

$(TARGET): build $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBS)

./objs/%.o: %.c
	$(CC) -c -o $@ $<

build:
	@mkdir -p objs/src/

clean:
	rm ./objs/*.o
	rm $(TARGET)
