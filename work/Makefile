#------------------------------------------------------------------------------
# Copyright (c) 2016, 2017 Oracle and/or its affiliates.  All rights reserved.
# This program is free software: you can modify it and/or redistribute it
# under the terms of:
#
# (i)  the Universal Permissive License v 1.0 or at your option, any
#      later version (http://oss.oracle.com/licenses/upl); and/or
#
# (ii) the Apache License v 2.0. (http://www.apache.org/licenses/LICENSE-2.0)
#
#------------------------------------------------------------------------------
#
# Sample Makefile if you wish to build the ODPI-C sample executables.
#
# Look at README.md for information on how to build and run the samples.
#------------------------------------------------------------------------------

BUILD_DIR = build
INCLUDE_DIR = ../include
LIB_DIR = ../lib

CC=gcc
LD=gcc
CFLAGS=-I../include -O2 -g -Wall
LIBS=-L../lib -lodpic
COMMON_OBJS = $(BUILD_DIR)/SampleLib.o

SOURCES = TestCallProc.c

BINARIES = $(SOURCES:%.c=$(BUILD_DIR)/%)

all: $(BUILD_DIR) $(BINARIES)

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c ../include/dpi.h 
	$(CC) -c $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%: $(BUILD_DIR)/%.o 
	$(LD) $(LDFLAGS) $< -o $@ $(COMMON_OBJS) $(LIBS)

