#
# STACKO
# the useless stacklang
#

CFLAGS += -std=c2x

all: stacko

clean:
	rm stacko

test: stacko
	@./runexamples.sh
