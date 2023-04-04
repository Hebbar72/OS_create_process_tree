a.out : helper.o driver.c PTree.h
	gcc helper.o driver.c
	./a.out

helper.o : helper.c
	gcc -c helper.c

clean: 
	rm *.out *.o