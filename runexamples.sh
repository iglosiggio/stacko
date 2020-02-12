#!/bin/bash
for FILE in `ls examples`; do
	echo "RUNNING EXAMPLE $FILE"
	./stacko "examples/$FILE"
done
