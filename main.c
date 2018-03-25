/**
 * Name:    Francesco
 * Surname: Longo
 * ID:      223428
 * Lab:     2
 * Ex:      3
 *
 * Implement a C program that
 * - takes from the command line two integer numbers n1, n2;
 * - allocates two vectors v1 and v2, of dimensions n1 and n2, respectively;
 * - fills v1 with n1 random even integer numbers between 10-100;
 * - fills v2 with n2 random odd integer numbers between 21-101;
 * - save the content of vectors v1 and v2 in two binary files fv1.b and fv2.b, respectively;
 *
 * Implement a concurrent program in C language, using Pthreads, which creates two client threads,
 * then it acts as a server.
 * A client thread loops reading the next number from the binary file (fv1.b and fv2.b, respectively),
 * and storing it in a global variable g. Then, it performs a signals on a semaphore to indicate to the server that
 * the variable is ready, and it waits on a semaphore a signal from the server indicating that the number has been
 * processed (simply multiplied by 3), finally, it prints the result and its identifier.
 * The server loops waiting the signal of the clients, doing the multiplication, storing the results on the same
 * global variable g, and signalling to the client that the string is ready to be printed.
 * The main thread waits the end on the threads, and prints the total number of served requests.
 **/

/* INCLUDE */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

/* PROTOTYPES */
void* Malloc(int size);
void fill_array_with_rand_number(int *array, int array_size, int min_rand, int max_rand, bool even);

/* MAIN */
int main( int argc, char *argv[] ){
    FILE *fpb1, *fpb2;
    int n1, n2;
    int *v1, *v2;

    if (argc != 3) {
        fprintf(stdout, "Expected 3 argument: <prog_name> <n1> <n2>\n");
        exit(-1);
    }

    // take lenght of array
    n1 = atoi(argv[1]);
    n2 = atoi(argv[2]);

    // control if n1,n2 are valid number
    if ( n1 <= 0 || n2 <= 0 ){
        //error in input
        printf( "you must insert a valid number!\n");
        exit(-1);
    }

    // allocate array and control in Malloc
    v1 = (int*)Malloc(n1*sizeof(int));
    v2 = (int*)Malloc(n2*sizeof(int));

    // fill array
    fill_array_with_rand_number(v1, n1, 10, 100, true);
    fill_array_with_rand_number(v2, n2, 21, 101, false);

    // open file
    fpb1 = fopen("fv1.b","wb");
    fpb2 = fopen("fv2.b","wb");

    // control opened files
    if( fpb1 == NULL || fpb2 == NULL ) {
        //error
        printf("error opening file\n");
        exit(-1);
    }

    // write binary files
    fwrite(v1, sizeof(int), (size_t) n1, fpb1);
    fwrite(v2, sizeof(int), (size_t) n2, fpb2);

    // free memory
    free(v1);
    free(v2);

    //close opened files
    fclose(fpb1);
    fclose(fpb2);

    return 0;
}

/* WRAPPING FUNCTION */
void* Malloc(int size){
    void* ptr = malloc((size_t) size);

    if(ptr == NULL){
        printf("error allocating array\n");
        exit(-1);
    }

    return ptr;
}

/* FILL ARRAY */
void fill_array_with_rand_number(int *array, int array_size, int min_rand, int max_rand, bool even){
    for(int i=0; i<array_size; i++){
        int random = (rand()%(max_rand-min_rand+1)) + min_rand;

        if (even) {
            // even
            if (random % 2 != 0) {
                random++;
            }
        } else {
            // odd
            if (random % 2 == 0) {
                random++;
            }
        }

        array[i] = random;
    }
}
