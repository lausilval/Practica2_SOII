#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 10 //número de elementos del buffer


int cuenta = 0; //numero de elementos del búfer en cada momento
int buffer[N]; //LIFO
int tope = -1; //tope de la pila


//función para producir un elemento
int producir_elemento() {
	return rand() % 100; //genera un número aleatorio entre 0 y 99
}


//función para insertar un elemento en el buffer
void insertar_elemento(int elemento) {
	tope++;
	pila[tope] = elemento;
}

//función del productor
void productor(void) {
	int elemento;
	
	while(1) {
		elemento = producir_elemento();
		if(cuenta==N) sleep();
		//se despierta e inserta (el consumidor ha quitado un elemento del buffer)
		insertar_elemento(elemento);
		cuenta+=1;
		if(cuenta==1) wakeup(consumidor);
	}
}
