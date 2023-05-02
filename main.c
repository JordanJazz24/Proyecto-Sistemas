#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
//#include <windows.h>

//#define MAX_THREADS 8

// Estructura para almacenar los parámetros que se le pasan a cada hilo
struct thread_data
{
    long *vector;
    long m;
    long n;
};

// Función para intercambiar dos elementos
void swap(long *x, long *y)
{
    long temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

// Función para seleccionar como el pivote el ELEMENTO CENTRAL del vector
long choose_pivot(long i, long j)
{
    return ((i + j) / 2);
}

// Función para obtener el tiempo de duración de ordenamiento del vector.
double timeval_diff(struct timeval *a, struct timeval *b)
{
    return
            (double) (a->tv_sec + (double) a->tv_usec / 1000000) -
            (double) (b->tv_sec + (double) b->tv_usec / 1000000);
}


// Función para realizar el ordenamiento mediante el Quicksort
void quicksort(long vector[], long m, long n)
{
    long key, i, j, k;

    if (m < n)
    {
        k = choose_pivot(m, n);//k será PIVOTE
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
void *quicksort_threaded(void *args)
{
    // Aquí recibimos la sublista para volverla a ordenar
    // con QuickSort. <args> contiene la sublista en sí
    // Luego, mandamos esa sublista a la función quicksort

    // Obtener los datos del argumento pasados a este hilo
    struct thread_data *data = (struct thread_data *) args;
    long *vector = data->vector;
    long m = data->m;
    long n = data->n;

    // Ordenar la sublista utilizando el Quicksort
    quicksort(vector, m, n);

    // Terminar el hilo
    pthread_exit(NULL);
}

void quickSort_N_elementos_y_N_hilos(long cantidadElementos, short cantidadHilos)
{
    /* CREANDO VECTOR */
    //Tamaño del vector
    //long const MAX_ELEMENTS = 10;//cantidadElementos (BORRAR ESTO)

    // Reserva un espacio de memoria X cantidad de elementos de tipo long
    long *vector = malloc(cantidadElementos * sizeof(long));

    // Genera los números aleatorios para llenar el vector
    long iseed = (long) time(NULL);
    srand(iseed);
    for (long i = 0; i < cantidadElementos; i++)
    {
        vector[i] = rand() % 999999999999; //Números grandes (12 dígitos)
    }

    // Muestra el vector antes de ordenarse
    /*printf("\nMuestra el vector antes de ordenarse:\n");
    for (long i = 0; i < cantidadElementos; i++)
    {
        printf("%ld %ld\n", i, vector[i]);
    }*/




    /* GENERANDO FUNCIONES Y VARIABLES DE HILOS Y DURACIÓN DE ESTOS */

    // Declarar variables para medir el tiempo de ejecución
    struct timeval t_ini, t_fin;//Todavía no tienen ningún valor
    double secs;

    // Variables para manejar los hilos
    pthread_t threads[cantidadHilos];
    struct thread_data thread_data_array[cantidadHilos];
    long chunk_size = cantidadElementos / cantidadHilos;

    // Iniciar el contador de tiempo
    gettimeofday(&t_ini, NULL);

    // Crear los hilos
    for (long i = 0; i < cantidadHilos; i++)
    {
        // Configurar los parámetros para el hilo actual
        long m = i * chunk_size;
        long n = (i == cantidadHilos - 1) ? cantidadElementos - 1 : m + chunk_size - 1;
        thread_data_array[i].vector = vector;
        thread_data_array[i].m = m;
        thread_data_array[i].n = n;

        // Crear el hilo
        int rc = pthread_create(&threads[i], NULL, quicksort_threaded, (void *) &thread_data_array[i]);
        if (rc)
        {
            printf("Error: no se pudo crear el hilo %ld.\n", i);
            exit(-1);
        }
    }

    // Esperar a que terminen todos los hilos
    for (long i = 0; i < cantidadHilos; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Combinar las sub-listas ordenadas por los hilos en un solo vector
    long sorted_vector[cantidadElementos];
    long pos[cantidadHilos];
    for (long i = 0; i < cantidadHilos; i++)
    {
        pos[i] = thread_data_array[i].m;
    }
    for (long i = 0; i < cantidadElementos; i++)
    {
        long min_val = vector[cantidadElementos - 1];
        long min_idx = -1;
        for (long j = 0; j < cantidadHilos; j++) {
            if (pos[j] <= thread_data_array[j].n && vector[pos[j]] < min_val) {
                min_val = vector[pos[j]];
                min_idx = j;
            }
        }
        sorted_vector[i] = min_val;
        pos[min_idx]++;
    }

    // Copiar el vector ordenado en el vector original
    for (long i = 0; i < cantidadElementos; i++)
    {
        vector[i] = sorted_vector[i];
    }

    // Detener el contador de tiempo y calcular el tiempo de ejecución
    gettimeofday(&t_fin, NULL);
    secs = timeval_diff(&t_fin, &t_ini);

    // Muestra el vector después de ordenarse
    /*printf("\nMuestra el vector después de ordenarse:\n");
    for (long i = 0; i < cantidadElementos; i++)
    {
        printf("%ld %ld\n", i, vector[i]);
    }*/

    // Muestra el tiempo de duración del ordenamiento
    printf("%.4g milliseconds\n", secs * 1000.0);

    // Libera el espacio de memoria reservado para el vector
    free(vector);
}

void quickSortMultipleTesteo(short repeticiones, long cantidadElementos, short cantidadHilos)
{
    printf("Pruebas:   %d\n",  repeticiones);
    printf("Elementos: %ld\n", cantidadElementos);
    printf("Hilos:     %d\n",  cantidadHilos);
    for(int i = 0; i<repeticiones; i++)
    {
        printf("%d: ",i+1); quickSort_N_elementos_y_N_hilos(cantidadElementos,cantidadHilos);
    }
}


// Función principal
int main(void)
{
    //Argumentos: (Repeticiones, cantidad elementos, hilos)
    quickSortMultipleTesteo(2, 1000000, 1);

    return 0;
}
