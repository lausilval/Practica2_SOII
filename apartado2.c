/*
*	AUTORAS: Candela Rodríguez y Laura Silva
*	PRACTICA 2: Sincronización de procesos con semáforos
*	APARTADO 2: PRODUCTO-CONSUMIDOR usando semáforos
*	FECHA: 02/04/2023
*/

// ARCHIVOS DE CABECERA
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

// CONSTANTES
#define N 10	// tamaño del buffer
#define BUCLE 100 // numero de iteraciones finito

// VARIABLES GLOBALES
int *buffer;	// El buffer debe ser tipo int y funcionar como una pila LIFO
			// el tamaño del buffer debe ser N=10
int *cuenta;	// contador para el buffer
// 3 semaforos:
sem_t *mutex;	// para asegurar que el productor y el consumidor no tengan 	
			// acceso al buffer al mismo tiempo
sem_t *vacias;	// para contabilizar el numero de ranuras vacias
sem_t *llenas;	// para contabilizar el numero de ranuras llenas
			

// FUNCIONES
/** La funcion produce_item() debe generar un elemento (un entero).
*
*
*/
void produce_item();


/** La funcion insert_item() debe colocar el elemento en el buffer
*
*
*/
void insert_item();

/** La funcion consume_item() debe retirar un entero del buffer y sustituirlo 
* por -1. Además debe sumar el elemento extraído con el anterior, solo si la 
* posicion extraída es mayor que 0
*
*
*/
void consume_item();


// PROGRAMA PRINCIPAL
int main(int argc, *char argv[])
{
	// 1. Declaracion de Variables
	pid_t productor;
	pid_t consumidor;
	int elemento;
	
	// 2. Inicializacion del buffer, debe estar inicializado a -1
	for(int i=0; i < N; i++)
	{
		buffer[i] = -1;
	}


	// 3. Creamos las zonas compartidas para los procesos
	/** Parametros de mmap:
	* mmap() crea la proyeccion de un archivo, asignar una región de memoria en el espacio de direcciones de un proceso
	* @param addr NULL para que el sistema operativa elija la direccion de memoria
    	* @param len tamaño en bytes de la región de memoria que se desea asignar.
        * @param prot permisos de protección para la región de memoria en este caso se le otorgaran de lectura y escritura
      	* @param flags area de memoria compartida
      	* @param fd descriptor de archivo para el archivo que se va a asignar a la memoria. Si se está haciendo una asignación anónima, este parámetro debe ser -1
     	 * @param offset un desplazamiento desde el inicio del archivo de 0
      	* @return
	*/
	buffer = mmap(NULL, N * sizeof *buffer, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
	if(buffer == MAP_FAILED)	// comprobacion de que se crea correctamente
	{
		perror("ERROR en el mmap() de buffer");
		exit(EXIT_FAILURE);
	}
	
	cuenta = mmap(NULL, sizeof *cuenta, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
	if(cuenta == MAP_FAILED) 	// comprobamos que se haya creado bien
	{
		perror("ERROR en el mmap() de cuenta");
		exit(EXIT_FAILURE);
	}

	// inicializamos el contador, en un primer momento el buffer estara vacio
	*cuenta = 0;
	
	// 4. Inicializamos los semáforos
	/** Parametros de sem_open():
	* sem_open() crea un semaforo, establece una conexion entre un semaforo con nombre y un proceso
	* @param name apunta a una serie que da nombre a un objeto de semaforo
	* @param oflag
	* @param mode
	* @param value
	*/
	mutex = sem_open("mutex", O_CREAT, 0700, 1);
	if(mutex == SEM_FAILED)	// comprobacion de que se ha creado correctamente
	{
		perror("Error al crear el semaforo de mutex");
		exit(EXIT_FAILURE);
	}
	
	vacias = sem_open("vacias", O_CREAT, 0700, N);
	if(vacio == SEM_FAILED)
	{
		perror("Error al crear el semaforo de vacias ");
		exit(EXIT_FAILURE);
	}
	
	llenas = sem_open("llenas", O_CREAT, 0700, 0);
	if(llenas == SEM_FAILED)
	{
		perror("Error al crear el semaforo de llenas");
	}

	// 5. Creamos los procesos asociados al productor y consumidor
	// - Crearemos un proceso hijo para cada uno
	// - Crearemos antes el proceso asociado al consumidor ya que así nos 
	//   	aseguramos que el productor no inserte un numero elevado de elementos
	//   	antes de que el consumidor pueda eliminarlos. Asi, al empezar el consumidor
	//   	primero, este estará inactivo hasta que el productor instroduzca los
	//   	primero elementos en el buffer
	
	if((consumidor = fork()) == 0 )
	{
		// CODIGO ASOCIADO AL PAPEL CONSUMIDOR
		
		// while(TRUE)
		// { 
		//	down(&llenas); 	/*disminuye la cuenta de ranuras llenas*/
		//	down(&mutex); 	/*entra a la región crítica*/
		//	elemento = quitar_elemento(); /*saca el elemento del búfer*/
		//	up(&mutex); 	/*sale de la región crítica */
		//	up(&vacias); 	/*incrementa la cuenta de ranuras vacías */
		//	consumir_elemento(elemento); /* hace algo con el elemento */
		// }
		
		// 6. Abrimos los semaforos
		mutex  = sem_open("mutex", 0);
		vacias = sem_open("vacias", 0);
		llenas = sem_open("llenas", 0);
		if((mutex == SEM_FAILED) || (vacias == SEM_FAILED) || (llenas == SEM_FAILED))
		{
			perror("ERROR al abrir algun semaforo");
			exit(EXIT_FAILURE);
		
		}
		
		// 6.1 creamos "ciclo infinito" en este caso será un for con 100 
		// iteraciones, no es recomendable hacer un bucle infinito
		for(int i=0; i < BUCLE; i++)
		{
			
			// 6.2 disminuye la cuenta de ranuras llenas
			/** La funcion sem_wait() decrementa en uno el valor del semaforo si este es mayor que 0, 
			*   si el valor del semaforo es 0, sem_wait se bloquea hasta que el semaforo sea mayor que 0.
			* @param sem especifica el semaforo que se va a bloquear
			*
			*/
			if(sem_wait(llenas) < 0)
			{
				perror("Error al disminuir el sem llenas");
				exit(EXIT_FAILURE);
			}
			// ENTRAMOS EN LA REGION CRITICA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// 6.3 disminuimos el semaforo del mutex
			if(sem_wait(mutex) < 0)
			{
				perror("Error al disminuir el sem mutex");
				exit(EXIT_FAILURE);
			}
			
			// 6.4 saca el elemento del buffer
///------------------------>??? elemento = remove_item();

			// SALIMOS DE LA REGION CRITICA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// 6.5 incrementa la cuenta del mutex
			/** La funcion sem_post() incrementa en uno el valor del semaforo si este es mayor que 0
			* @param sem especifica el semaforo que se va a bloquear
			*
			*/
			if(sem_post(mutex) < 0)
			{
				perror("Error al aumentar el sem mutex");
				exit(EXIT_FAILURE);
			}
			// 6.6 incrementa la cuenta de ranuras vacias
			if(sem_post(vacias) < 0)
			{
				perror("Error al aumentar el sem vacias");
				exit(EXIT_FAILURE);
			}
			// 6.7 hace algo con el elemento
			consume_item(elemento);
		}
		
	}// final if consumidor
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
		// 7. Abrimos los semaforos
		mutex  = sem_open("mutex", 0);
		vacias = sem_open("vacias", 0);
		llenas = sem_open("llenas", 0);
		if((mutex == SEM_FAILED) || (vacias == SEM_FAILED) || (llenas == SEM_FAILED))
		{
			perror("ERROR al abrir algun semaforo");
			exit(EXIT_FAILURE);
		
		}
		
	} // final if productor
	else if(productor == -1)
	{
		// ERROR
		// cerraremos la memoria compartida
		
		// imprimeremos un mensaje de error
		perror("ERROR al crear el proceso PRODUCTOR\n");
		exit(EXIT_FAILURE);
	
	}
	
	// 8. Deberemos esperar por los procesos ¿waitpid??
	
	// 9. Cerramos las zonas de memoria compartidas
	
	// 10. Cerramos los semaforos
	/** La funcion sem_close() cierra un semaforo
	* @param sem especifica el semaforo que se va a cerrar
	*
	*/
	if(sem_close(mutex) < 0)
	{
		perror("Error al cerrar el sem mutex");
		exit(EXIT_FAILURE);
	}
	if(sem_close(vacias) < 0)
	{
		perror("Error al cerrar el sem vacias");
		exit(EXIT_FAILURE);
	}
	if(sem_close(llenas) < 0)
	{
		perror("Error al cerrar el sem llenas");
		exit(EXIT_FAILURE);
	}
	
	// 11. Eliminamos los semaforos
	/** La funcion sem_unlink() elimina un semaforo
	* @param sem especifica el semaforo que se va a eliminar
	*
	*/
	if(sem_unlink(mutex) < 0)
	{
		perror("Error al eliminar el sem mutex");
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(vacias) < 0)
	{
		perror("Error al eliminar el sem vacias");
		exit(EXIT_FAILURE);
	}
	if(sem_unlink(llenas) < 0)
	{
		perror("Error al eliminar el sem llenas");
		exit(EXIT_FAILURE);
	}

	// 12. Terminamos con exito la ejecucion del programa
	return(EXIT_SUCCESSFUL);
} 
