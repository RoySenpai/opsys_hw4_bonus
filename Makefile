#################################################################################
# 	Operation Systems (OSs) Course Assignment 4 Bonus Makefile		#
# 	Authors: Roy Simanovich and Linor Ronen (c) 2023			#
# 	Description: This Makefile compiles the programs and libraries 		#
# 				Date: 2023-06					#
# 			Course: Operating Systems				#
# 				Assignment: 4 (Bonus)				#
# 				Compiler: gcc					#
# 				OS: Linux					#
# 			IDE: Visual Studio Code					#
#################################################################################

# Flags for the compiler and linker.
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -g -pedantic
SFLAGS = -shared
TFLAGS = -pthread
HFILE = proactor.h reactor.h settings.h
LIBREACTOR = st_reactor.so
LIBPROACTOR = st_proactor.so
RM = rm -f

# Phony targets - targets that are not files but commands to be executed by make.
.PHONY: all default clean

# Default target - compile everything and create the executables and libraries.
all: proactor_server

# Alias for the default target.
default: all


############
# Programs #
############
proactor_server: proactor_server.o $(LIBREACTOR) $(LIBPROACTOR)
	$(CC) $(CFLAGS) -o $@ $< ./$(LIBREACTOR) ./$(LIBPROACTOR) $(TFLAGS)

##################################
# Libraries and shared libraries #
##################################
$(LIBREACTOR): st_reactor.o
	$(CC) $(CFLAGS) $(SFLAGS) -o $@ $^ $(TFLAGS)

st_reactor.o: st_reactor.c $(HFILE)
	$(CC) $(CFLAGS) -fPIC -c $<

$(LIBPROACTOR): st_proactor.o
	$(CC) $(CFLAGS) $(SFLAGS) -o $@ $^ $(TFLAGS)

st_proactor.o: st_proactor.c $(HFILE)
	$(CC) $(CFLAGS) -fPIC -c $<


################
# Object files #
################
%.o: %.c $(HFILE)
	$(CC) $(CFLAGS) -c $<
	
#################
# Cleanup files #
#################
clean:
	$(RM) *.o *.so proactor_server