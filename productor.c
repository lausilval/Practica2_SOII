#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 10


int cuenta = 0;

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



