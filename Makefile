#
# STACKO
# the useless stacklang
#

all: stacko

clean:
	rm stacko

test: stacko
	@./stacko "{ 1 - dup print \"hola!!\" print 1 find 1 find {exec} exch if } dup 7 exch exec"
	@echo "PROGRAMA DE PRUEBA QUE IMPRIME 4 VECES HOLA E IMPLEMENTA REPEAT!"
	@echo
	@./stacko "{ 1 - 1 find 1 find {exec 2 find exec} {pop pop} 3 -1 roll ifelse } dup {\"hola\" print} 5 3 -1 roll exec"
	@echo "Disan(10) count"
	@echo
	@echo 10 | ./stacko "0 {1 + {dup print \"is even\" print} {dup print \"is odd\" print} 2 find ifelse} read tonumber times"
