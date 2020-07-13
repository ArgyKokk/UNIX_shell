#Variables
RM=rm -rf

all:
	gcc myShell.c -o myShell

clean:
	$(RM) *o myShell
