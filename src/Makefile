CC ?=gcc
AR ?=ar


WMSG = -Wall

TARGETBIN ?= ../bin
TARGETDIR ?= $(TARGETBIN)/lib
LDFLAGS ?= -lz -lpthread -lssl -L$(TARGETDIR) -lproperties
CXXFLAGS ?= -g -rdynamic -I ../ $(MACROS)

LIBDIRS = .
LIBSRC = $(patsubst %, %/*.c, $(LIBDIRS))
OBJECTS = $(patsubst %.c, %.o, $(wildcard ./*.c))


.PHONY: all clean cleanall

all: static
	@mkdir -p $(TARGETBIN)/include
	cp -f ../libproperties.h 		$(TARGETBIN)/include/

static:  $(OBJECTS) 
	@mkdir -p $(TARGETDIR)
	@echo "AR $(TARGETDIR)/libproperties.a"
	@$(AR) rc $(TARGETDIR)/libproperties.a $(OBJECTS)
	$(CC) $(WMSG) -o $(TARGETBIN)/testproperties ../examples/testproperties.cc $(CXXFLAGS) $(LDFLAGS)
	@rm -f $(OBJECTS)
	@echo ""

$(OBJECTS): %.o : %.c
	@echo "CC $@"
	@$(CC) $(WMSG) -c $< -o $@  $(CXXFLAGS) 

clean:
	@rm -f  $(OBJECTS) 

cleanall:clean
	@rm -f $(OBJECTS)

