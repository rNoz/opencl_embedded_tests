.PHONY: build

all: build

clean:
	rm -rf build

mkdirp:
	mkdir -p build

build: mkdirp
	g++ saxpy.cpp -Wall -o build/saxpy -lOpenCL -lrt
