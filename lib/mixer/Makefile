tests:=info echo

all: release debug
	@:

release: configure.release build

debug: configure.debug build

configure.%:
	@node-gyp configure --$*

build:
	@node-gyp build

clean:
	@node-gyp clean

.PHONY: build
