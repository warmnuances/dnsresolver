# Sample Makefile
# CC - compiler
# OBJ - compiled source files that should be linked
# COPT - compiler flags
# BIN - binary
CC=clang
OBJ=helper1.o
COPT=-Wall -Wpedantic -g 
BIN_PHASE1=phase1
BIN_PHASE2=dns_query

RUN_CMD = < packets/2.comp30023.req.raw
# DEPS = utils.c dns.c 
RUN_SERVER = ./dns_svr 172.26.129.247 5353

DEPS = server.o


# Running "make" with no argument will make the first target in the file
# all: $(BIN_PHASE1) $(BIN_PHASE2)
all: main.c ${DEPS} utils.o 
	$(CC) -o dns_svr main.c utils.o ${DEPS} $(COPT)


# Rules of the form
#     target_to_be_made : dependencies_to_be_up-to-date_first
#     <tab>commands_to_make_target
# (Note that spaces will not work.)

# $(BIN_PHASE2): main.c 
# 	$(CC) -o $(BIN_PHASE2) main.c  $(COPT)

# $(BIN_PHASE1): phase1.c 
# 	$(CC) -o $(BIN_PHASE1) phase1.c $(OBJ) $(COPT)
	

main: main.c ${DEPS} utils.o
	$(CC) -o dns_svr main.c utils.o ${DEPS} $(COPT) && ${RUN_SERVER}

server: server.c server.h utils.o
	$(CC) -o server server.c utils.o $(COPT)

utils: utils.c utils.h
	$(CC) -o utils utils.c $(COPT)

client: client.c
	$(CC) -o client client.c $(COPT)

runClient: 
	./client 127.0.0.1 8080


# Wildcard rule to make any  .o  file,
# given a .c and .h file with the same leading filename component
%.o: %.c %.h
	$(CC) -c $< $(COPT) -g

format:
	clang-format -i *.c *.h

clean:
# implement this, to remove $(BIN_PHASE[12]) and any .o files
	rm *.o || true
	rm dns_query || true
	rm phase1 || true
	rm utils || true
	rm dns || true
	rm server || true
	rm dns_svr || true

# This .PHONY command declares the "clean" rule as a phony one, i.e., it means
# that the clean rule will run instructions but it wont create a file called
# "clean" like a normal rule would (e.g., the util.o rule results in a file
# called util.o being created).
.PHONY: all clean
