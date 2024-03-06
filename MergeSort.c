/*
Tecnológico de Costa Rica
Tarea #1 - Principios de Sistemas Operativos  | Semestre 1 -2024
Elaborado Por:
Adrián Herrera Segura
Ronaldo Vindas Barboza 
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define MAX_THREADS 4 

typedef struct {
    int *array;
    int left;
    int right;
} ThreadArgs;


typedef struct {
    int *arr;
    int start;
    int end;
} ThreadModaArgs;

typedef struct {
    int moda;
    int frecuencia;
} ModaResultado;




void generadorNumerosAleatorios(int *arr, int num) {
    srand(time(NULL));

    for (int i = 0; i < num; i++) {
        int numeroAleatorio = rand() % (1000 - (-1000) + 1) + -1000;
        arr[i] = numeroAleatorio;
    }
}

void merge(int arr[], int left, int middle, int right) {
    int i, j, k;
    int n1 = middle - left + 1;
    int n2 = right - middle;
    
    int L[n1], R[n2];

    for (i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }

    for (j = 0; j < n2; j++) {
        R[j] = arr[middle + 1 + j];
    }

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSortRecursivo(int arr[], int left, int right) {
    if (left < right) {
        int middle = left + (right - left) / 2;

        mergeSortRecursivo(arr, left, middle);
        mergeSortRecursivo(arr, middle + 1, right);

        merge(arr, left, middle, right);
    }
}

void* desarrolloMergeSortParalelizado(void *args) {
    ThreadArgs *threadArgs = (ThreadArgs *)args;
    int left = threadArgs->left;
    int right = threadArgs->right;
    int *arr = threadArgs->array;

    if (left < right) {
       
        int mid = left + (right - left) / 2;

        ThreadArgs leftArgs = {arr, left, mid};
        ThreadArgs rightArgs = {arr, mid + 1, right};

        pthread_t leftThread, rightThread;
        pthread_create(&leftThread, NULL, desarrolloMergeSortParalelizado, (void *)&leftArgs);
        pthread_create(&rightThread, NULL, desarrolloMergeSortParalelizado, (void *)&rightArgs);

        pthread_join(leftThread, NULL);
        pthread_join(rightThread, NULL);

        merge(arr, left, mid, right);
    }

    pthread_exit(NULL);
}

void mergeSortParalelizado(int arr[], int size, int cantHilos) {
    ThreadArgs args = {arr, 0, size - 1};

    pthread_t mainThread[cantHilos];


     for (int i = 0; i < cantHilos; i++) {
        pthread_create(&mainThread[i], NULL, desarrolloMergeSortParalelizado, (void *)&args);
    }

    for (int i = 0; i < cantHilos; i++) {
        pthread_join(mainThread[i], NULL);
    }
}


/*int calcularModa(int arr[], int size) {
    int maxCant = 0;
    int moda = -1;

    for (int i = 0; i < size; i++) {
        int cant = 0;

        for (int j = 0; j < size; j++) {
            if (arr[j] == arr[i]) {
                cant++;
            }
        }

        if (cant > maxCant) {
            maxCant = cant;
            moda = arr[i];
        }
    }

    return moda;
}*/

void *calcularModaParcial(void *args) {
    ThreadModaArgs *threadArgs = (ThreadModaArgs *)args;
    int *arr = threadArgs->arr;
    int start = threadArgs->start;
    int end = threadArgs->end;

    int moda = -1;
    int frecuencia = 0;

    for (int i = start; i <= end; i++) {
        int count = 0;

        for (int j = start; j <= end; j++) {
            if (arr[j] == arr[i]) {
                count++;
            }
        }

        if (count > frecuencia) {
            frecuencia = count;
            moda = arr[i];
        }
    }

    ModaResultado *resultado = (ModaResultado *)malloc(sizeof(ModaResultado));
    resultado->moda = moda;
    resultado->frecuencia = frecuencia;

    pthread_exit((void *)resultado);
}

int calcularModa(int *arr, int size, int numThreads) {
    pthread_t threads[MAX_THREADS];
    ThreadModaArgs threadArgs[MAX_THREADS];
    ModaResultado *results[MAX_THREADS];

    int chunkSize = size / numThreads;
    int remainder = size % numThreads;
    int modaGlobal = -1;
    int frecuenciaGlobal = 0;

    
    for (int i = 0; i < numThreads; i++) {
        int start = i * chunkSize;
        int end = (i == numThreads - 1) ? (start + chunkSize + remainder - 1) : (start + chunkSize - 1);

        threadArgs[i].arr = arr;
        threadArgs[i].start = start;
        threadArgs[i].end = end;

        pthread_create(&threads[i], NULL, calcularModaParcial, (void *)&threadArgs[i]);
    }

    
    for (int i = 0; i < numThreads; i++) {                  
        pthread_join(threads[i], (void **)&results[i]);
    }

    
    for (int i = 0; i < numThreads; i++) {                  //Encuentra la moda global
        if (results[i]->frecuencia > frecuenciaGlobal) {
            modaGlobal = results[i]->moda;
            frecuenciaGlobal = results[i]->frecuencia;
        }
        free(results[i]);
    }

    return modaGlobal;
}






int main() {

    int cantidadNumeros;
    int numHilos;

    printf("Ingrese la cantidad de números: ");
    scanf("%d", &cantidadNumeros);

    printf("Ingrese la cantidad de hilos: ");
    scanf("%d", &numHilos);

    clock_t inicioRecursivo, finRecursivo;
    clock_t inicioParalelizado, finParalelizado;

    double tiempoTranscurridoRecursivo;
    double tiempoTranscurridoParalelizado;

    int vector[cantidadNumeros];
    int vectorCopia[cantidadNumeros];

    generadorNumerosAleatorios(vector, cantidadNumeros);
    memcpy(vectorCopia, vector, sizeof(vector));

    /* printf("Vector sin ordenar:\n");
    for (int i = 0; i < cantidadNumeros; i++) {
        printf("Numero Aleatorio: %d\n", vector[i]);
    } */

    inicioRecursivo = clock();
    mergeSortRecursivo(vector, 0, cantidadNumeros -1);
    finRecursivo = clock();

    tiempoTranscurridoRecursivo = ((double) (finRecursivo - inicioRecursivo)) / CLOCKS_PER_SEC;

    printf("Tiempo transcurrido del MergeSort resursivo: %f segundos\n", tiempoTranscurridoRecursivo);

    inicioParalelizado = clock();
    mergeSortParalelizado(vectorCopia, cantidadNumeros, numHilos);
    finParalelizado = clock();

    tiempoTranscurridoParalelizado = ((double) (finParalelizado - inicioParalelizado)) / CLOCKS_PER_SEC;

    printf("Tiempo transcurrido del MergeSort paralelizado: %f segundos\n", tiempoTranscurridoParalelizado);


    //int moda = calcularModa(vectorCopia, cantidadNumeros);

    int moda = calcularModa(vectorCopia, cantidadNumeros, 2);


    if (moda != -1) {
        printf("La moda es: %d\n", moda);
    } else {
        printf("No hay moda en el arreglo.\n");
    }

    /* printf("Vector Paralelizado:\n");
    for (int i = 0; i < cantidadNumeros; i++) {
        printf("Numero Aleatorio: %d\n", vector[i]);
    }

    printf("\nVector Recursivo:\n");
    for (int i = 0; i < cantidadNumeros; i++) {
        printf("Numero Aleatorio: %d\n", vectorCopia[i]);
    } */

    return 0;
}