#!/bin/bash


malloc_num=$(ltrace ./test 2>&1 >/dev/null| grep -e alloc -e fopen | wc -l)

for (( i=0 ; i<=$malloc_num ; i++ ));
do
		echo
		echo "======================"
		echo "$i-th *alloc will fail"
		echo "======================"
		echo
		./test "$i"
		if [ $? != "0" ]
		then
				echo "test failed when $i-th *alloc or fopen functin returned NULL"
				echo "Run \"gdb ./test\" and enter command \"run $i\" to reproduce"
				echo "you can set breakpoint at beginning of test using \"break [TEST_NAME]\""
				exit 1
		fi

		if [ $? == "123" ]
		then
				echo "Segfault when $i-th *alloc or fopen returned NULL"
				echo "Run \"gdb ./test\" and enter command \"run $i\" to reproduce"
				exit 1
		fi
done
