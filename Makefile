#
# STACKO
# the useless stacklang
#

all: stacko

clean:
	rm stacko

test: stacko
	@./runexamples.sh
