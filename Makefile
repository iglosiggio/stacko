#
# STACKO
# the useless stacklang
#

all: stacko

clean:
	rm stacko

test: stacko
	./stacko "{ 1 - dup print \"hola!!\" print 1 find 1 find {exec} exch if } dup 7 exch exec"
