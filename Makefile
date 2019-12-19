CFLAGS_DBUS = $(shell pkg-config --cflags --libs dbus-1)

CFLAGS = -g -Wall -Wno-unused-variable

all: wpa-client

wpa-client: wpa-client.c
	gcc $< -o $@ $(CFLAGS) $(CFLAGS_DBUS)

clean:
	rm -f wpa-client

.PHONY: all clean
