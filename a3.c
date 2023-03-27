/*
*	AUTORAS: Candela Rodríguez y Laura Silva
*	PRACTICA 2: Sincronización de procesos con semáforos
*	APARTADO 3: CONSUMIDOR-PRODUCTOR con hilos
*	FECHA: 02/04/2023
*/

// ARCHIVOS DE CABECERA
#include <sys/mman.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

// CONSTANTES
#define N 10		        // tamaño del buffer
#define BUCLE 10		    // numero de iteraciones finito
#define CYAN "\e[1;36m"     // color del productor
#define BCYAN "\e[1;96m"
#define MAG "\e[1;35m"	    // color del consumidor
#define BMAG "\e[1;95m"
#define YEL "\e[0;33m"	    // color buffer
#define RESET "\e[0m"	    // color reset

// VARIABLES GLOBALES
int *buffer = NULL;	        // El buffer debe ser tipo int y funcionar como una pila LIFO
                            // el tamaño del buffer debe ser N=10
int *cuenta = 0;	        // contador para el buffer
//int *cuenta = 0;           // contador para producir elementos en orden
//int *cuenta = 0;
int contador = 0;

// 3 semaforos:
sem_t *mutex;	            // para asegurar que el productor y el consumidor no tengan
                            // acceso al buffer al mismo tiempo
sem_t *vacias;	            // para contabilizar el numero de ranuras vacias
sem_t *llenas;	            // para contabilizar el numero de ranuras llenas


// FUNCIONES
/** La funcion imprimeBuffer() imprime los elementos del buffer
 *
 */
void imprimeBuffer()
{
    int i;
    printf(YEL"\nBUFFER:"RESET);
    printf(YEL"\t-"RESET);
    for(i=0; i < N; i++)
    {
        printf(YEL"------"RESET);
    }
    printf(YEL"\n\t|"RESET);
    for(i=0; i < N; i++)
    {
        printf(" %-3d "YEL"|"RESET, buffer[i]);
    }
    printf(YEL"\n\t-"RESET);
    for(i=0; i < N; i++)
    {
        printf(YEL"------"RESET);
    }
    printf("\n\n");

}

int remove_item(int *cuent)
{
    int elemento = buffer[*cuent - 1];
    if(*cuent > 0)
    {
        buffer[(*cuent) - 1] = -1;
        (*cuent)--;
    }
    return elemento;
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
    return (contador++);
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

void *productorFuncion()
{
    // 6. Abrimos los semaforos
    mutex  = sem_open("mutex", 0);
    vacias = sem_open("vacias", 0);
    llenas = sem_open("llenas", 0);
    if((mutex == SEM_FAILED) || (vacias == SEM_FAILED) || (llenas == SEM_FAILED))
    {
        perror("ERROR al abrir algun semaforo");
        exit(EXIT_FAILURE);

    }

    // 6.1 creamos ciclo "infinito" con 100 iteraciones
    for(int i=0; i < BUCLE; i++) {

        //sleep(1);
        srand(time(NULL));
        sleep(rand() % 5);
        // 6.2 producimos el elemento
        int elemento = produce_item();
        printf("\nEl elemento producido es: %d\n", elemento);

        // 6.3 disminuye la cuenta de ranuras llenas
        /** La funcion sem_wait() decrementa en uno el valor del semaforo si este es mayor que 0,
        *   si el valor del semaforo es 0, sem_wait se bloquea hasta que el semaforo sea mayor que 0.
        * @param sem especifica el semaforo que se va a bloquear
        *
        */
        if (sem_wait(vacias) < 0) {
            perror("Error al disminuir el sem llenas");
            exit(EXIT_FAILURE);
        }

        // ENTRAMOS EN LA REGION CRITICA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // 6.4 disminuimos el semaforo del mutex
        if (sem_wait(mutex) < 0) {
            perror("Error al disminuir el sem mutex");
            exit(EXIT_FAILURE);
        }

        // 6.5 insertamos el elemento en el buffer
        insert_item(elemento);

        // 6.6 aumenta la cuenta de ranuras llenas
        /** La funcion sem_post() incrementa en uno el valor del semaforo
        * @param sem especifica el semaforo que se va a bloquear
        *
        */
        if (sem_post(mutex) < 0) {
            perror("Error al aumentar el sem mutex");
            exit(EXIT_FAILURE);
        }
        if (sem_post(llenas) < 0) {
            perror("Error al aumentar el sem llenas");
            exit(EXIT_FAILURE);
        }
    }
    pthread_exit(NULL);
}


void *consumidorFuncion(){
    // 5. Abrimos los semaforos
    mutex  = sem_open("mutex", 0);
    vacias = sem_open("vacias", 0);
    llenas = sem_open("llenas", 0);
    if((mutex == SEM_FAILED) || (vacias == SEM_FAILED) || (llenas == SEM_FAILED))
    {
        perror("ERROR al abrir algun semaforo");
        exit(EXIT_FAILURE);

    }

    // 5.1 creamos "ciclo infinito" en este caso será un for con 100
    // iteraciones, no es recomendable hacer un bucle infinito
    for(int i=0; i < BUCLE; i++)
    {
        srand(time(NULL));
        sleep(rand() % 3);
        // 5.2 disminuye la cuenta de ranuras llenas
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
        // 5.3 disminuimos el semaforo del mutex
        if(sem_wait(mutex) < 0)
        {
            perror("Error al disminuir el sem mutex");
            exit(EXIT_FAILURE);
        }

        // 5.4 saca el elemento del buffer
        //int elemento = buffer[*cuenta];
        int elemento = remove_item(cuenta);
        printf("\nEl elemento eliminado es: %d\n", elemento);
        // imprimimos el elemento consumido
        ////consume_item(elemento);
        // SALIMOS DE LA REGION CRITICA ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // 5.5 incrementa la cuenta del mutex
        /** La funcion sem_post() incrementa en uno el valor del semaforo si este es mayor que 0
        * @param sem especifica el semaforo que se va a bloquear
        *
        */
        if(sem_post(mutex) < 0)
        {
            perror("Error al aumentar el sem mutex");
            exit(EXIT_FAILURE);
        }
        // 5.6 incrementa la cuenta de ranuras vacias
        if(sem_post(vacias) < 0)
        {
            perror("Error al aumentar el sem vacias");
            exit(EXIT_FAILURE);
        }
        // 5.7 hace algo con el elemento
        consume_item(elemento);
    }
    pthread_exit(NULL);
}


// PROGRAMA PRINCIPAL
int main()
{
    // Variables
    pthread_t productor;
    pthread_t consumidor;

    srand(time(NULL));      // semilla para generar numeros random
    // reservamos memoria para el buffer
    buffer = (int*) malloc (N * sizeof(int));
    if(buffer == NULL)      // comprobamos que se haya reservado bien memoria
    {
        perror("Error en la reserva de memoria del buffer");
        exit(EXIT_FAILURE);
    }
    // 2.1 Inicializacion del buffer, debe estar inicializado a -1
    for(int i=0; i < N; i++)
    {
        buffer[i] = -1;
    }

    // inicializamos el contador, en un primer momento el buffer estara vacio
    cuenta = (int*) malloc(sizeof (int));

    // 3. Inicializamos los semáforos
    // El proceso padre debe crear e inicializar. Los procesos que usen ese
    // semaforo posteriormente deben abrirlo pero no inicializarlo.
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
    if(vacias == SEM_FAILED)
    {
        perror("Error al crear el semaforo de vacias ");
        exit(EXIT_FAILURE);
    }

    llenas = sem_open("llenas", O_CREAT, 0700, 0);
    if(llenas == SEM_FAILED)
    {
        perror("Error al crear el semaforo de llenas");
    }
    // creamos los hilos
    if((pthread_create(&productor, NULL, productorFuncion, NULL)) != 0)
    {
        perror("Error al crear el hilo del productor");
        exit(EXIT_FAILURE);
    }
    if((pthread_create(&consumidor, NULL, consumidorFuncion, NULL)) != 0)
    {
        perror("Error al crear el hilo del consumidor");
        exit(EXIT_FAILURE);
    }




    // 9. Cerramos los semaforos
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

    // esperamos a los hilos
    if((pthread_join(productor, NULL)) < 0)
    {
        perror("Error en el pthread_join del productor");
        exit(EXIT_FAILURE);
    }
    if((pthread_join(consumidor, NULL)) < 0)
    {
        perror("Error en el pthread_join del consumidor");
        exit(EXIT_FAILURE);
    }

    // 10. Eliminamos los semaforos
    /** La funcion sem_unlink() elimina un semaforo
    * @param sem especifica el semaforo que se va a eliminar
    *
    */
    if(sem_unlink("mutex") < 0)
    {
        perror("Error al eliminar el sem mutex");
        exit(EXIT_FAILURE);
    }
    if(sem_unlink("vacias") < 0)
    {
        perror("Error al eliminar el sem vacias");
        exit(EXIT_FAILURE);
    }
    if(sem_unlink("llenas") < 0)
    {
        perror("Error al eliminar el sem llenas");
        exit(EXIT_FAILURE);
    }

    // liberamos la memoria reservada
    free(buffer);
    free(cuenta);

    // 11. Terminamos con exito la ejecucion del programa
    return(EXIT_SUCCESS);

}