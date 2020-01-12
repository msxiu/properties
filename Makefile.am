AUTOMAKE_OPTIONS=foreign 
include_HEADERS = libproperties.h

#SUBDIRS     = example

# noinst_LTLIBRARIES      = libproperties.la
lib_LTLIBRARIES           = libproperties.la
libproperties_la_SOURCES    = src/properties.c src/properties_parse.c src/propertiesreflect.c

bin_PROGRAMS     = testproperties
testproperties_SOURCES    = examples/testproperties.c
testproperties_LDFLAGS    = -lz -lpthread -lproperties