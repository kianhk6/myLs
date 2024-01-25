all:
	gcc -g -o myls myls.c
clean:
	$(RM) myls