#!/bin/bash
for FILE in `ls examples`; do
	echo "RUNNING EXAMPLE $FILE"
	./stacko "`cat examples/$FILE`"
done
