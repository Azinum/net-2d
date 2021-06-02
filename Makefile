# Makefile

include config.mk

all: prepare compile run

prepare:
	mkdir -p ${BUILD}

compile:
	${CC} ${SRC} ${C_FLAGS} ${LIB}

run:
	./${BUILD}/${PROG}
