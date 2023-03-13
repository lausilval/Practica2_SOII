#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>

#define N 10 //número de elementos del buffer
#define iteraciones 100


int cuenta = 0; //numero de elementos del búfer en cada momento
int buffer[N]; //LIFO
int tope = -1; //tope de la pila


//función para producir un elemento
int produce_item() {
	return rand() % 100; //genera un número aleatorio entre 0 y 99
}


//función para insertar un elemento en el buffer
void insert_item(int elemento) {
	tope++;
	buffer[tope] = elemento;
}

//función del productor
void productor(void) {
	int elemento;
	int i = 0;
	
	while(i < iteraciones) {
		elemento = produce_item();
		if(cuenta==N) sleep();
		//se despierta e inserta (el consumidor ha quitado un elemento del buffer)
		insert_item(elemento);
		printf("[Productor]: Se ha producido el elemento %d y se ha introducido en el buffer\n", elemento);
		cuenta+=1;
		if(cuenta==1) wakeup(consumidor);
		i++;
	}
}
