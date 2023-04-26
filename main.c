#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_THREADS 8

// Estructura para almacenar los parámetros que se le pasan a cada hilo
struct thread_data {
    long *vector;
    long m;
    long n;
};

// Función para intercambiar dos elementos
void swap(long *x, long *y) {
    long temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

// Función para seleccionar como el pivote el elemento central del vector
long choose_pivot(long i, long j) {
    return ((i + j) / 2);
}

// Función para obtener el tiempo de duración de ordenamiento del vector.
double timeval_diff(struct timeval *a, struct timeval *b) {
    return
            (double) (a->tv_sec + (double) a->tv_usec / 1000000) -
            (double) (b->tv_sec + (double) b->tv_usec / 1000000);
}

// Función para realizar el ordenamiento mediante el Quicksort
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
                //Intercambia (swap) dos elementos
                swap(&vector[i], &vector[j]);
        }
        //Intercambia (swap) dos elementos
        swap(&vector[m], &vector[j]);
        // Efectúa los llamados recursivos a ambos vectores
        quicksort(vector, m, j - 1);
        quicksort(vector, j + 1, n);
    }
}

// Función para realizar el ordenamiento en paralelo mediante hilos
void *quicksort_threaded(void *args) {
    // Obtener los argumentos pasados al hilo
    struct thread_data *data = (struct thread_data *) args;
    long *vector = data->vector;
    long m = data->m;
    long n = data->n;

    // Ordenar la sublista utilizando el Quicksort
    quicksort(vector, m, n);

    // Terminar el hilo
    pthread_exit(NULL);
}






// Función principal
int main(void) {
// Tamaño del vector
    long const MAX_ELEMENTS = 10;

// Reserva un espacio de memoria X cantidad de elementos de tipo long
    long *vector = malloc(MAX_ELEMENTS * sizeof(long));

// Genera los números aleatorios para llenar el vector
    long iseed = (long) time(NULL);
    srand(iseed);
    for (long i = 0; i < MAX_ELEMENTS; i++) {
        vector[i] = rand() % 999999999999; //Números grandes (12 dígitos)
    }

// Muestra el vector antes de ordenarse
    printf("\nMuestra el vector antes de ordenarse:\n");
    for (long i = 0; i < MAX_ELEMENTS; i++) {
        printf("%ld %ld\n", i, vector[i]);
    }

// Declarar variables para medir el tiempo de ejecución
    struct timeval t_ini, t_fin;
    double secs;

// Variables para manejar los hilos
    pthread_t threads[MAX_THREADS];
    struct thread_data thread_data_array[MAX_THREADS];
    long chunk_size = MAX_ELEMENTS / MAX_THREADS;

// Iniciar el contador de tiempo
    gettimeofday(&t_ini, NULL);

// Crear los hilos
    for (long i = 0; i < MAX_THREADS; i++) {
// Configurar los parámetros para el hilo actual
        long m = i * chunk_size;
        long n = (i == MAX_THREADS - 1) ? MAX_ELEMENTS - 1 : m + chunk_size - 1;
        thread_data_array[i].vector = vector;
        thread_data_array[i].m = m;
        thread_data_array[i].n = n;
        // Crear el hilo
        int rc = pthread_create(&threads[i], NULL, quicksort_threaded, (void *) &thread_data_array[i]);
        if (rc) {
            printf("Error: no se pudo crear el hilo %ld.\n", i);
            exit(-1);
        }
    }
// Esperar a que terminen todos los hilos
    for (long i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

// Combinar las sub-listas ordenadas por los hilos en un solo vector
    long sorted_vector[MAX_ELEMENTS];
    long pos[MAX_THREADS];
    for (long i = 0; i < MAX_THREADS; i++) {
        pos[i] = thread_data_array[i].m;
    }
    for (long i = 0; i < MAX_ELEMENTS; i++) {
        long min_val = vector[MAX_ELEMENTS - 1];
        long min_idx = -1;
        for (long j = 0; j < MAX_THREADS; j++) {
            if (pos[j] <= thread_data_array[j].n && vector[pos[j]] < min_val) {
                min_val = vector[pos[j]];
                min_idx = j;
            }
        }
        sorted_vector[i] = min_val;
        pos[min_idx]++;
    }

// Copiar el vector ordenado en el vector original
    for (long i = 0; i < MAX_ELEMENTS; i++) {
        vector[i] = sorted_vector[i];
    }

// Detener el contador de tiempo y calcular el tiempo de ejecución
    gettimeofday(&t_fin, NULL);
    secs = timeval_diff(&t_fin, &t_ini);

// Muestra el vector después de ordenarse
    printf("\nMuestra el vector después de ordenarse:\n");
    for (long i = 0; i < MAX_ELEMENTS; i++) {
        printf("%ld %ld\n", i, vector[i]);
    }

// Muestra el tiempo de duración del ordenamiento
    printf("%.4g milliseconds\n", secs * 1000.0);
// Libera el espacio de memoria reservado para el vector
    free(vector);

    return 0;
}
