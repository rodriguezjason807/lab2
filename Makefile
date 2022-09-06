
all: lab2

lab2: lab2.cpp
	g++ lab2.cpp -Wall -olab2 -lX11 -lGL -lGLU -lm
	g++ lab2_old.cpp -Wall -olab2_old -lX11 -lGL -lGLU -lm

clean:
	rm -f lab2
	rm -f lab2_old

