.PHONY: build

all: build

clean:
	make -C vectors clean; \
	make -C saxpy clean;

build: 
	make -C vectors build; \
	make -C saxpy build;
