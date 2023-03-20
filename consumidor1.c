/*
*	AUTORAS: Candela Rodríguez y Laura Silva
*	PRACTICA 2: Sincronización de procesos con semáforos
*	APARTADO 1: CONSUMIDOR
*	FECHA: 02/04/2023
*/

// ARCHIVOS DE CABECERA
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

// CONSTANTES
#define N 10

// VARIABLES GLOBALES
// variable para las posiciones del buffer.
int *cuenta = 0;
// El buffer debe ser del tipo int y funcionar como una pila LIFO
//(Last In First Out), es decir el ultimo en entrar va a ser el primero en salir
int *buffer;	

// FUNCIONES
void quitar_elemento(int *indice)
{
	*indice = (*indice + 1) % N;
}

void consume_item(int elemento)
{
	printf("Consumidor: \n\tSe ha consumido el elemento: %d\n\tContador: %d", elemento, cuenta);
}

int produce_item() {
    return rand() % 100;
}

void insertar_elemento(int elemento) {
    buffer[*cuenta] = elemento;
    (*cuenta)++;
}



// PROGRAMA PRINCIPAL
int main()
{
	// Variables
	int indice = 0;
	int elemento;		//
	pid_t productor;		// pid asociado al proceso del productor
	pid_t consumidor;		// pid asociado al proceso del consumidor
	int i = 0, j = 0;
	
	// 2. Creamos las zonas compartidas para los procesos	
	/** Parametros de mmap:
	* mmap() crea la proyeccion de un archivo, asignar una región de memoria en el espacio de direcciones de un proceso
	*@param addr NULL para que el sistema operativa elija la direccion de memoria
    	* @param len tamaño en bytes de la región de memoria que se desea asignar.
      * @param prot permisos de protección para la región de memoria en este caso se le otorgaran de lectura y escritura
      * @param flags area de memoria compartida
      * @param fd descriptor de archivo para el archivo que se va a asignar a la memoria. Si se está haciendo una asignación anónima, este parámetro debe ser -1
      * @param offset un desplazamiento desde el inicio del archivo de 0
      * @return
	*/
	cuenta = mmap(NULL, sizeof *cuenta, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
	if(cuenta == MAP_FAILED) 	// comprobamos que se haya creado bien
	{
		perror("ERROR en el mmap() de cuenta");
		exit(EXIT_FAILURE);
	}
	// inicializamos el contador, en un primer momento el buffer estara vacio
	*cuenta = 0;
	
	buffer = mmap(NULL, N * sizeof *buffer, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
	if(buffer == MAP_FAILED)
	{
		perror("ERROR en el mmap() de buffer");
		exit(EXIT_FAILURE);
	}
	
	
	// Crearemos los subprocesos asociados al productor y al consumidor, para ello crearemos un proceso hijo. Crearemos antes el proceso asociado al consumidor ya que así nos aseguramos que el productor no inserte un número elevado de elementos antes de que el consumidor pueda eliminarlos. Asi, al empezar el consumidor primero, este estará inactivo hasta que el productor introduzca los primeros elementos en el buffer.
	
	if((consumidor = fork()) == 0)
	{
		// CODIGO ASOCIADO AL PAPEL CONSUMIDOR
		while(i < 100)	// no se deberia usar un bucle infinito
		{
			if(*cuenta == 0)	// si el buffer esta vacio, pasa a inactivo
			{
				printf("Consumidor: \n\tel buffer está vacio, en espera\n");
				sleep(2);
			}
			// si no lo está eliminará un elemento del buffer
			elemento = buffer[indice];
			quitar_elemento(&indice);
			// disminuye la cuenta de elementos en el buffer
			(*cuenta)--;
			printf("Consumidor: \n\telemento %d extraído del buffer\n", elemento);
			
			if(*cuenta == N-1)	// si el buffer esta lleno se despierta al productor
			{
				printf("Consumidor: buffer lleno, despertando al productor\n");
				kill(getppid(), SIGUSR1);
			}
			
			consumir_elemento(elemento);	// imprimimos el elemento
			i++;
		}
		
		
	}
	else if(consumidor == -1)
	{	
		// ERROR
		// cerraremos la memoria compartida
		
		// imprimeremos un mensaje de error
		perror("ERROR al crear el proceso CONSUMIDOR\n");
		exit(EXIT_FAILURE);
	}
	
	if((productor = fork()) == 0)
	{
		// CODIGO ASOCIADO AL PAPEL PRODUCTOR
		while(j < 100)	// no se deberia usar un bucle infinito
		{
			if(*cuenta == N)	// si el buffer esta vacio, pasa a inactivo
			{
				printf("Productor: \n\tel buffer está lleno, en espera\n");
				sleep(1);
			}
			// si no lo está creará un elemento y lo introducirá en el buffer
			elemento = produce_item();
			insertar_elemento(elemento);
			// aumenta la cuenta de elementos en el buffer
			(*cuenta)++;
			printf("Productor: \n\telemento %d insertado en el buffer\n", elemento);
			
			if(*cuenta == 5)	// si el buffer esta lleno se despierta al consumidor
			{
				printf("Productor: buffer lleno, despertando al consumidor\n");
				kill(getppid(), SIGUSR1);
			}
			
			consumir_elemento(elemento);	// imprimimos el elemento
			i++;
		}
	else if(productor == -1)
	{
		// ERROR
		// cerraremos la memoria compartida
		
		// imprimeremos un mensaje de error
		perror("ERROR al crear el proceso PRODUCTOR\n");
		exit(EXIT_FAILURE);
	
	}
		
// cerramos la region de memoria compartida
if (munmap(cuenta, sizeof(*cuenta)))
{
	perror("ERROR al cerrar la region de memoria compartida del contador");
}
if(munmap(buffer, N*sizeof(buffer))
{
	perror("ERROR al cerrar la region de memoria compartida del buffer");
}
	
	
	
	
	



}
