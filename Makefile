
all: lab2

lab2: lab2.cpp
	g++ lab2.cpp -Wall -olab2 -lX11 -lGL -lGLU -lm -Wextra libggfonts.a
clean:
	rm -f lab2
	rm -f *.o

