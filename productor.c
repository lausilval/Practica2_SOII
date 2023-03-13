#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 10


int cuenta = 0;
int buffer[N];
int tope = -1;


//función para producir un elemento
int producir_elemento() {
	return rand() % 100;
}


//función para insertar un elemento en el buffer
void insertar_elemento(int elemento) {
	tope++;
	pila[tope] = elemento;
}

//función del productor
void productor(void) {
	int elemento;
	
	while(TRUE) {
		elemento = producir_elemento();
		if(cuenta==N) sleep();
		insertar_elemento(elemento);
		cuenta+=1;
		if(cuenta==1) wakeup(consumidor);
	}
}
