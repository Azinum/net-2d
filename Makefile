# Makefile

include config.mk

all: prepare compile run

prepare:
	mkdir -p ${BUILD}

compile:
	${CC} ${SRC} ${C_FLAGS} ${LIB} ${O_RELEASE}

debug:
	${CC} ${SRC} ${C_FLAGS} ${LIB} ${O_DEBUG}
	gdb ./${BUILD}/${PROG}

run:
	./${BUILD}/${PROG}

server:
	./${BUILD}/${PROG} server
