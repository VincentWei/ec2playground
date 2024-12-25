all:
	./sh/build.sh
	emrun ./

build:
	./sh/build.sh
run:
	emrun ./
cloc:
	./sh/cloc.sh