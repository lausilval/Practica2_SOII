/*
*	AUTORAS: Candela Rodríguez y Laura Silva
*	PRACTICA 2: Sincronización de procesos con semáforos
*	APARTADO 1: CONSUMIDOR Y PRODUCTOR
*	FECHA: 02/04/2023
*/

// ARCHIVOS DE CABECERA
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

// CONSTANTES
#define N 10		// tamaño del buffer
#define BUCLE 10		// numero de iteraciones finito
#define CYAN "\e[1;36m" // color del productor
#define BCYAN "\e[1;96m"
#define MAG "\e[1;35m"	// color del consumidor
#define BMAG "\e[1;95m"
#define YEL "\e[0;33m"	// color buffer
#define RESET "\e[0m"	// color reset

// VARIABLES GLOBALES
// variable para las posiciones del buffer.
int *cuenta = NULL;
// El buffer debe ser del tipo int y funcionar como una pila LIFO
//(Last In First Out), es decir el ultimo en entrar va a ser el primero en salir
int *buffer = NULL;	
// contador para producir elementos en orden
int contador = 0;

// FUNCIONES
void imprimeBuffer()
{
	printf(YEL"\nBUFFER: ["RESET);
	for(int k=0; k<N; k++)
	{
		printf(YEL"%d"RESET, buffer[k]);
		if(k != N-1)
		{
			printf(YEL", "RESET);
		}
		else
		{
			printf(YEL" ]\n"RESET);
		}
	}
}

void remove_item(int *cuenta)
{
	buffer[(*cuenta) -1] = -1;
	(*cuenta)--;
}

/** La funcion consume_item() debe retirar un entero del buffer y sustituirlo 
* por -1. Además debe sumar el elemento extraído con el anterior, solo si la 
* posicion extraída es mayor que 0
*
*
*/
void consume_item(int elemento)
{
	printf(MAG"Consumidor: \n\tSe ha consumido el elemento: %d\n\tContador: %d\n"RESET, elemento, *cuenta);
	imprimeBuffer();
}

/** La funcion produce_item() debe generar un elemento (un entero).
*
*
*/
int produce_item() 
{
    //return rand() % 100;
    return contador++;
}

/** La funcion insert_item() debe colocar el elemento en el buffer
*
*
*/
void insert_item(int elemento) 
{
    printf(CYAN"Productor: \n\tSe ha producido el elemento: %d\n\tContador: %d\n"RESET, elemento, *cuenta);
    buffer[*cuenta] = elemento;
    (*cuenta)++;
    imprimeBuffer();
}



// PROGRAMA PRINCIPAL
int main()
{
	// Variables
	//int indice = 0;
	//int elemento;		//
	pid_t productor;		// pid asociado al proceso del productor
	pid_t consumidor;		// pid asociado al proceso del consumidor
	//int i = 0, j = 0;
	int status;			// guardamos el estado de los procesos
	
	// 2. Creamos las zonas compartidas para los procesos	
	/** Parametros de mmap:
	* mmap() crea la proyeccion de un archivo, asignar una región de memoria en el espacio de direcciones de un proceso
	*@param addr NULL para que el sistema operativa elija la direccion de memoria
    	* @param len tamaño en bytes de la región de memoria que se desea asignar.
      * @param prot permisos de protección para la región de memoria en este caso se le otorgaran de lectura y escritura
      * @param flags area de memoria compartida, tendremos que poner mapa anonimo porque el fd es -1
      * @param fd descriptor de archivo para el archivo que se va a asignar a la memoria. Si se está haciendo una asignación anónima, este parámetro debe ser -1
      * @param offset un desplazamiento desde el inicio del archivo de 0
      * @return
	*/
	cuenta = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(cuenta == MAP_FAILED) 	// comprobamos que se haya creado bien
	{
		perror("ERROR en el mmap() de cuenta");
		exit(EXIT_FAILURE);
	}
	// inicializamos el contador, en un primer momento el buffer estara vacio
	*cuenta = 0;
	
	buffer = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(buffer == MAP_FAILED)
	{
		perror("ERROR en el mmap() de buffer");
		exit(EXIT_FAILURE);
	}
	// inicializamos el buffer, los elementos deben ser -1
	for(int i=0; i < N; i++)
	{
		buffer[i] = -1;
	} 
	
	// Crearemos los subprocesos asociados al productor y al consumidor, para 
	// ello crearemos un proceso hijo. Crearemos antes el proceso asociado al 
	// consumidor ya que así nos aseguramos que el productor no inserte un 
	// número elevado de elementos antes de que el consumidor pueda eliminarlos. 
	// Asi, al empezar el consumidor primero, este estará inactivo hasta que el 
	// productor introduzca los primeros elementos en el buffer.
	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// CONSUMIDOR
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	consumidor = fork();
	if(consumidor == 0)
	{
	printf(BMAG"El pid del consumidor es: %d\n"RESET, getpid());
		// CODIGO ASOCIADO AL PAPEL CONSUMIDOR
		for(int i=0; i < BUCLE; i++)
		{
			while(*cuenta == 0)	// si el buffer esta vacio, pasa a inactivo
			{
				//printf(MAG"Consumidor: \n\tel buffer está vacio, en espera\n"RESET);
				//sleep(2);
			}
			// si no lo está eliminará un elemento del buffer
			int elemento = buffer[*cuenta];
			remove_item(cuenta);
			// disminuye la cuenta de elementos en el buffer
			//(*cuenta)--;
			printf(MAG"Consumidor: \n\telemento %d extraído del buffer\n"RESET, elemento);
			
			if(*cuenta == N-1)	// si el buffer esta lleno se despierta al productor
			{
				printf(BMAG"Consumidor: buffer lleno, despertando al productor\n"RESET);
				kill(productor, SIGUSR1);
			}
			
			consume_item(elemento);	// imprimimos el elemento
		}
		
		
	}
	else if(consumidor == -1) 	// ERROR
	{	
		// cerraremos la memoria compartida
		if (munmap(cuenta, sizeof(*cuenta)))
		{
			perror("ERROR al cerrar la region de memoria compartida del contador");
		}
		if(munmap(buffer, N*sizeof(buffer)))
		{
			perror("ERROR al cerrar la region de memoria compartida del buffer");
		}
		// imprimeremos un mensaje de error
		perror("ERROR al crear el proceso CONSUMIDOR\n");
		exit(EXIT_FAILURE);
	}
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// PRODUCTOR
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	productor = fork();
	if(productor == 0)
	{
	printf(BCYAN"El pid del productor es: %d\n"RESET, getpid());
		// CODIGO ASOCIADO AL PAPEL PRODUCTOR
		for(int j=0; j < BUCLE; j++)	// no se deberia usar un bucle infinito
		{
			while(*cuenta == N)	// si el buffer esta lleno, pasa a inactivo
			{
				//printf(BCYAN"Productor: \n\tel buffer está lleno, en espera\n"RESET);
				//sleep(1);
			}
			// si no lo está creará un elemento y lo introducirá en el buffer
			int elemento = produce_item();
			insert_item(elemento);
			// aumenta la cuenta de elementos en el buffer
			//(*cuenta)++;
			//printf(CYAN"Productor: \n\telemento %d insertado en el buffer\n"RESET, elemento);
			
			if(*cuenta == N)	// si el buffer esta lleno se despierta al consumidor
			{
				printf(BCYAN"Productor: buffer lleno, despertando al consumidor\n"RESET);
				kill(consumidor, SIGUSR1);
			}
		
		}
	}
	else if(productor == -1)	// ERROR
	{
		// cerraremos la memoria compartida
		if (munmap(cuenta, sizeof(*cuenta)))
		{
			perror("ERROR al cerrar la region de memoria compartida del contador");
		}
		if(munmap(buffer, N*sizeof(buffer)))
		{
			perror("ERROR al cerrar la region de memoria compartida del buffer");
		}
		// imprimeremos un mensaje de error
		perror("ERROR al crear el proceso PRODUCTOR\n");
		exit(EXIT_FAILURE);
	}
	
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// PROCESO PADRE
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	// deberá esperar para cerrar las memorias a sus hijos
	waitpid(consumidor, &status, 0);
	waitpid(productor, &status, 0);
		
	// cerramos la region de memoria compartida
	if (munmap(cuenta, sizeof(int)) < 0)
	{
		perror("ERROR al cerrar la region de memoria compartida del contador");
	}
	if(munmap(buffer, N*sizeof(int)) < 0)
	{
		perror("ERROR al cerrar la region de memoria compartida del buffer");
	}	
	
	return(EXIT_SUCCESS);
}
