# Makefile for Simple File Transfer

CC         = cc
CFLAGS     = -c -O3

SOURCES_CL = src/client.c
OBJECTS_CL = $(SOURCES_CL:.c=.o)
BIN_CL     = sft-client

SOURCES_SV = src/server.c
OBJECTS_SV = $(SOURCES_SV:.c=.o)
BIN_SV     = sft-server

.PHONY: all
all: $(SOURCES_CL) $(SOURCES_SV) $(BIN_CL) $(BIN_SV) done

$(BIN_CL): $(OBJECTS_CL)
	@echo Linking Client...
	$(CC) $(OBJECTS_CL) -o $@
	strip $(BIN_CL)
	chmod 755 $(BIN_CL)

$(BIN_SV): $(OBJECTS_SV)
	@echo Linking Server...
	$(CC) $(OBJECTS_SV) -o $@
	strip $(BIN_SV)
	chmod 755 $(BIN_SV)

.c.o:
	@echo Compiling...
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo Cleaning...
	find . -name "*.o"   -type f -delete
	find . -name "*.gch" -type f -delete

.PHONY: done
done:
	@echo -n

.PHONY: install
install:
	@echo Installing...
ifeq ($(shell id -u), 0)
	mkdir -p /usr/bin/
	cp $(BIN_CL) /usr/bin/
	cp $(BIN_SV) /usr/bin/
else
	mkdir -p ~/bin/
	cp $(BIN_CL) ~/bin/
	cp $(BIN_SV) ~/bin/
endif

# end of Makefile
