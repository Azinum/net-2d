# config.mk

CC=gcc

PROG=net-2d

INC=include

BUILD=build

X11_LIB=/usr/X11R6/lib

X11_INC=/usr/include/X11

SRC=${wildcard src/*.c}

LIB=-lm -lpthread -I${X11_INC} -L${X11_LIB} -lX11

C_FLAGS=-o ${BUILD}/${PROG} -I${INC} -Wall

O_DEBUG=-O0 -g

O_RELEASE=-O2
