CC = g++

ToreVBOShader: ToreVBOShader.o shader.o 
	$(CC) -o Launch ToreVBOShader.o shader.o -lGL -lGLU -lGLEW -lglut -lm 
all: ToreVBOShader
