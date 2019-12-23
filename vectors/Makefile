.PHONY: build

all: build

clean:
	rm -rf build

mkdirp:
	mkdir -p build

build: mkdirp
	g++ vectors.c -Wall -o build/vectors -lOpenCL -lrt
