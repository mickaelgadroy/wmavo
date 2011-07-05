#
#	wiiusecpp Makefile
#

#
# Change this to your GCC version.
#
CC = g++

####################################################
#
# You should not need to edit below this line.
#
####################################################

#
# Universal cflags
#
CFLAGS = -Wall -pipe -fPIC -funroll-loops

ifeq ($(debug),1)
	OBJ_PREFIX = debug
	CFLAGS += -g -pg -DWITH_WIIUSECPP_DEBUG

else
	OBJ_PREFIX = release
	CFLAGS += -O2
endif

OBJ_DIR = $(OBJ_PREFIX)-$(shell $(CC) -v 2>&1|grep ^Target:|cut -d' ' -f2)

#
# Linking flags
#
LDFLAGS = -shared -lwiiuse
#LDFLAGS = -shared -L../../wiiuse/src/$(OBJ_DIR)/ -lwiiuse

#
# Target binaries (always created as BIN)
#
BIN = ./$(OBJ_DIR)/libwiiusecpp.so

#
# Inclusion paths.
#
INCLUDES = -I../src/
#INCLUDES = -Isrc/ -I../../wiiuse/src/

#
# Generate a list of object files
#
OBJS = \
	$(OBJ_DIR)/wiiusecpp.o

###############################
#
# Build targets.
#
###############################

all: $(BIN)

clean:
	@-rm $(OBJS) 2> /dev/null

distclean:
	@-rm -rf debug-* release-* 2> /dev/null

install:
	@if [ -e $(BIN) ]; then \
		cp -v $(BIN) /usr/lib ; \
	fi
	@cp -v wiiusecpp.h /usr/include

$(BIN): mkdir $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(BIN)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

mkdir:
	@if [ ! -d $(OBJ_DIR) ]; then \
		mkdir $(OBJ_DIR); \
	fi
