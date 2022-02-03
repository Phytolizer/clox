.PHONY: all configure build clean

SRC := $(wildcard $(SRC_DIR)/*.c) clox.c

all: configure build

configure:
	cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build: configure $(SRC)
	cmake --build build

clean:
	rm -rvf build
