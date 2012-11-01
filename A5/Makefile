####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                  Makefile                        #
####################################################

COMPILER=gcc
OPTS=-Wall -pedantic -std=gnu99
LIBS=-lsctp
DEBUG=-D__DEBUG_EN -g

# Append generic sources here
SOURCES_GEN=sctp_utils.c addresses.c protocol.c game.c linked_list.c client_command_line.c
# Append server sources here
SOURCES_SRV=sctps.c $(SOURCES_GEN)
# Append client sources here
SOURCES_CLI=sctpc.c $(SOURCES_GEN)

RUNTEST=-D__RUN_TESTS
SRVNAME=sctps
CLINAME=sctpc
TESTSRVNAME=test$(SRVNAME)
TESTCLINAME=test$(CLINAME)

all:
	$(COMPILER) $(SOURCES_SRV) $(OPTS) $(LIBS) -o $(SRVNAME)
	$(COMPILER) $(SOURCES_CLI) $(OPTS) $(LIBS) -o $(CLINAME)

debug:
	$(COMPILER) $(SOURCES_SRV) $(OPTS) $(LIBS) $(DEBUG) -o $(SRVNAME)
	$(COMPILER) $(SOURCES_CLI) $(OPTS) $(LIBS) $(DEBUG) -o $(CLINAME)

tests:
	$(COMPILER) $(SOURCES_SRV) $(OPTS) $(LIBS) $(DEBUG) $(RUNTEST) -o $(TESTSRVNAME)
	$(COMPILER) $(SOURCES_CLI) $(OPTS) $(LIBS) $(DEBUG) $(RUNTEST) -o $(TESTCLINAME)

clean:
	@if [ $(SRVNAME) ] ; then rm -rf $(SRVNAME) ; fi
	@if [ $(CLINAME) ] ; then rm -rf $(CLINAME) ; fi
	@if [ $(TESTSRVNAME) ] ; then rm -rf $(TESTSRVNAME) ; fi
	@if [ $(TESTCLINAME) ] ; then rm -rf $(TESTCLINAME) ; fi

release: all

