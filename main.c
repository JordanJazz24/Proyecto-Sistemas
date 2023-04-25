
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

//Definimos la estructura de datos que utilizaremos para enviar los argumentos a los hilos
typedef struct {
    long *vector;
    long m;
    long n;
} quicksort_args;

//Función para intercambiar 2 elementos
void swap(long *x, long *y) {
    long temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

//Función para seleccionar como el pivote el elemento central del vector
long choose_pivot(long i, long j) {
    return ((i + j) / 2);
}

//Función que muestra el vector en pantalla
void printvector(long vector[], long n) {
    long i;
    for (i = 0; i < n; i++)
        printf("%ld %ld\n", i, vector[i]);
}

//Función para obtener el tiempo de duración de ordenamiento del vector.
double timeval_diff(struct timeval *a, struct timeval *b) {
    return
            (double) (a->tv_sec + (double) a->tv_usec / 1000000) -
            (double) (b->tv_sec + (double) b->tv_usec / 1000000);
}

//Función que realiza el ordenamiento mediante el Quicksort
void quicksort(long vector[], long m, long n) {
    long key, i, j, k;
    if (m < n) {
        k = choose_pivot(m, n);
        key = vector[m];
        i = m + 1;
        j = n;
        while (i <= j) {
            while ((i <= n) && (vector[i] <= key))
                i++;
            while ((j >= m) && (vector[j] > key))
                j--;
            if (i < j)
                swap(&vector[i], &vector[j]);
        }
        swap(&vector[m], &vector[j]);
        quicksort(vector, m, j - 1);
        quicksort(vector, j + 1, n);
    }
}

//Función que se ejecuta en el hilo 1
void *quicksort_thread1(void *arguments) {
    quicksort_args *args = (quicksort_args *) arguments;
    quicksort(args->vector, args->m, args->n / 2);
    pthread_exit(NULL);
}

//Función que se ejecuta en el hilo 2
void *quicksort_thread2(void *arguments) {
    quicksort_args *args = (quicksort_args *) arguments;
    quicksort(args->vector, args->n / 2 + 1, args->n);
    pthread_exit(NULL);
}

//===============================================
//Función Principal

int main() {
    //Tamaño del vector
    long const MAX_ELEMENTS = 10;
    //Reserva un espacio de memoria X cantidad de elementos de tipo long
    long *vector = malloc(MAX_ELEMENTS * sizeof(long));

    struct timeval t_ini, t_fin;
    double secs;
    long i = 0;

    //Genera los números aleatorios para llenar el vector
    long iseed = (long) time(NULL);
    srand(iseed);
    for (i = 0; i < MAX_ELEMENTS; i++) {
        vector[i] = rand() % 999999999999; //Números grandes (12 dígitos)
        //vector[i] = rand() % 99; //Números (2 dígitos)
    }

    //Muestra el vector antes de ordenarse
    printf("\nMuestra el vector antes de ordenarse:\n");
    printvector(vector, MAX_ELEMENTS);

    //Creamos los argumentos para los hilos
    quicksort_args args1 = {vector, 0, MAX_ELEMENTS - 1};
    quicksort_args args2 = {vector, 0, MAX_ELEMENTS - 1};

    //Creamos los hilos y los ejecutamos
    pthread_t thread1, thread2;
    gettimeofday(&t_ini, NULL);
    pthread_create(&thread1, NULL, quicksort_thread1, (void *) &args1);
    pthread_create(&thread2, NULL, quicksort_thread2, (void *) &args2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    gettimeofday(&t_fin, NULL);

    //Mostramos el vector ordenado
    printf("\nMuestra el vector despues de ordenarse:\n");
    printvector(vector, MAX_ELEMENTS - 1);

    //Calculamos el tiempo de ejecución
    secs = timeval_diff(&t_ini, &t_fin);
    printf("Tiempo de ejecucion: %f segundos\n", (secs * 1000.0) * -1);

    //Liberamos la memoria reservada para el vector
    free(vector);

    return 0;
}