#####################################################################
# Source Files
#####################################################################
 
# TODO: Add any new program headers here:

HEADERS = structs.h utils.h protos.h
 
# TODO: Add any new program object files to be built here:

OBJS = comm.o act.comm.o act.informative.o act.movement.o \
crafts.o objects.o act.offensive.o act.other.o wounds.o money.o \
staff.o handler.o db.o commands.o utility.o limits.o mobact.o \
fight.o weather.o constants.o olc.o create_mobile.o hash.o \
roomprogs.o nanny.o save.o magic.o mobprogs.o mysql.o somatics.o \
clans.o larg_prog.o\

#####################################################################

CC=gcc

# Uncomment the appropriate CFLAGS for your platform
CFLAGS = -ggdb -DLINUX -O -Wall
OPT_CFLAGS = -ggdb -DLINUX -O2 -Wuninitialized
# CFLAGS = -ggdb -DMACOSX -O -Wuninitialized -Wno-long-double

# First line is for debugging, second one is for production
# -lz needed by MACOSX
#LDLIBS = -lz -lmysqlclient -llua -lMallocDebug
LDLIBS =  -lz -lmysqlclient -llua

# directory where lua files are located
LUA= .
LUAINC= -I$(LUA)/include
LUALIB= -L$(LUA)/lib

# do not change these
CFLAGS+=$(LUAINC)
LDFLAGS=$(LUALIB)


# These directory listings should be able to find the requisite libraries
# on any *NIX-based system, including OS X and Free BSD.
# Generally your compiler knows where to find them if they are root-installed
# unless they are placed in a local heirarchy.

INCLUDE = -I/usr/local/mysql/include/ -I/usr/include/mysql/ -I/usr/include/openssl/ -I/usr/local/mysql/ $(LUAINC)

LIBDIRS = -L/Library/ -L/usr/lib/ -L/usr/local/mysql/lib/ -L/usr/lib/ -L/usr/local/mysql/lib/ $(LUALIB)
#####################################################################
# Makefile Rules
#####################################################################
	
../bin/server : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBDIRS) \
	$(LDFLAGS) -o ../bin/server \
	$(LDLIBS) $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(INCLUDE) $(CPPFLAGS) $<
 
clean:
	rm -f *.o ../bin/server

tags:
	ctags *.c *.h
