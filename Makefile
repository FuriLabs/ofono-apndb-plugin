PROJECT_NAME := ofono-apndb-plugin
VERSION := 0.20240825

CC := gcc
CFLAGS := -fPIC -Wall -Wextra -fvisibility=hidden -Wno-unused-parameter
LDFLAGS := -shared

SYSTEM_APNDB_PATH := /usr/share/lineageos-apn-conf/apns-conf.xml
CUSTOM_APNDB_PATH := /usr/share/lineageos-apn-conf/apns-conf.xml

CFLAGS += $(shell pkg-config --cflags glib-2.0 ofono)
LIBS := $(shell pkg-config --libs glib-2.0)

SOURCES := src/ubuntu-apndb.c src/apndb-provision.c
OBJECTS := $(SOURCES:.c=.o)

TARGET := apndbplugin.so

INSTALL := install
OFONO_PLUGINDIR := $(shell pkg-config --variable=plugindir ofono)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -DSYSTEM_APNDB_PATH=\"$(SYSTEM_APNDB_PATH)\" -DCUSTOM_APNDB_PATH=\"$(CUSTOM_APNDB_PATH)\" -c $< -o $@

install: $(TARGET)
	$(INSTALL) -d $(DESTDIR)$(OFONO_PLUGINDIR)
	$(INSTALL) -m 644 $(TARGET) $(DESTDIR)$(OFONO_PLUGINDIR)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all install clean
