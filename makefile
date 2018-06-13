# client code directory name
CLIENTDIR := examples/simpleChat/client

# server code directory name
SERVERDIR := examples/simpleChat/server

# common code directory name
COMMONDIR := examples/simpleChat/common

# binary output directory name
OUTPUTDIR := bin

# name for the output(binary) of the client code
CLIENTNAME := client

# name for the output(binary) of the server code
SERVERNAME := server

# libary code directory name
LIBDIR := lib

# add C flags, like: CFLAGS := -Wall -Werror -Wconversion -Wextra
CFLAGS :=

# add load flags, others like -pthread
LDLIB := -lm -lallegro -lallegro_image -lallegro_primitives -lallegro_font -lallegro_ttf

CC := gcc -std=c99
RM := rm -f
MK := mkdir -p

EXT := c
INC := -I $(LIBDIR) -I $(COMMONDIR)

CLIENTSOURCES := $(shell find $(CLIENTDIR) -type f -name *.$(EXT))
SERVERSOURCES := $(shell find $(SERVERDIR) -type f -name *.$(EXT))
LIBSOURCES := $(shell find $(LIBDIR) -type f -name *.$(EXT))
COMMONSOURCES := $(shell find $(COMMONDIR) -type f -name *.$(EXT))


CLIENTOBJS := $(subst .$(EXT),.o,$(CLIENTSOURCES))
SERVEROBJS := $(subst .$(EXT),.o,$(SERVERSOURCES))
LIBOBJS := $(subst .$(EXT),.o,$(LIBSOURCES))
COMMONOBJS := $(subst .$(EXT),.o,$(COMMONSOURCES))

all: mkdirs buildServer buildClient clean

server: mkdirs buildServer clean runServer

client: mkdirs buildClient clean runClient

buildClient: $(LIBOBJS) $(CLIENTOBJS) $(COMMONOBJS)
	@echo "\n  Linking $(CLIENTNAME)..."
	$(CC) -o $(OUTPUTDIR)/$(CLIENTNAME) $(LIBOBJS) $(COMMONOBJS) $(CLIENTOBJS) $(LDLIB) $(CFLAGS)
	@echo "\n"

buildServer: $(LIBOBJS) $(SERVEROBJS) $(COMMONOBJS)
	@echo "\n  Linking $(SERVERNAME)..."
	$(CC) -o $(OUTPUTDIR)/$(SERVERNAME) $(LIBOBJS) $(COMMONOBJS) $(SERVEROBJS) $(LDLIB) $(CFLAGS)
	@echo "\n"

%.o : %.$(EXT)	
	$(CC) -c $< -o $@ $(INC) $(CFLAGS)

mkdirs:
	$(MK) $(OUTPUTDIR)

clean:
	@echo "  Cleaning..."
	$(RM) $(LIBOBJS) $(CLIENTOBJS) $(SERVEROBJS) $(COMMONOBJS)

runClient:
	@echo "\n  Starting to run $(CLIENTNAME)...\n"; ./$(OUTPUTDIR)/$(CLIENTNAME)

runServer:
	@echo "\n  Starting to run $(SERVERNAME)...\n"; ./$(OUTPUTDIR)/$(SERVERNAME) 