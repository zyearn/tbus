CC=gcc
SOURCES=$(wildcard *.c)
OBJECTS=$(patsubst %.c, ../objs/%.o,$(SOURCES))

TARGET=tbusmgr

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(LIBS)

../objs/%.o: %.c
	$(CC) -c -o $@ $<

clean:
	rm ../objs/*.o
	rm $(TARGET)
