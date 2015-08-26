CC=gcc
SOURCES=$(wildcard src/*.c)
GSOURCES=$(wildcard ../src/*.c)

NOTDIR=$(notdir $(SOURCES))
GNOTDIR=$(notdir $(GSOURCES))

OBJECTS=$(patsubst %.c, ./objs/%.o,$(NOTDIR))
GOBJECTS=$(patsubst %.c, ./gobjs/%.o,$(GNOTDIR))


TARGET=test

all: $(TARGET)

$(TARGET): build $(OBJECTS) $(GOBJECTS)
	$(CC) -g -o $(TARGET) -I../src/ $(OBJECTS) $(GOBJECTS) $(LIBS)

./objs/%.o: src/%.c
	$(CC) -c -o $@ -I../src/ $<
	
./gobjs/%.o: ../src/%.c
	$(CC) -c -o $@ $<

build:
	@mkdir -p objs/
	@mkdir -p gobjs/

clean:
	@rm -rf ./objs
	@rm -rf ./gobjs
	@rm $(TARGET)
