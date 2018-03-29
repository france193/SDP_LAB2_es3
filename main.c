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
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 50
#define THREADS 2

// arguments for thread
typedef struct {
    bool active;
    char filename[N];
} Box;

sem_t *client_me, *full, *empty;

int g, tot_requests;
Box box1, box2;

/* PROTOTYPES */
void* Malloc(int size);
void fill_array_with_rand_number(int *array, int array_size, int min_rand, int max_rand, bool even);
void *client(void *box_passed);
void first_part(int argc, char *argv[]);

/* MAIN */
int main(int argc, char *argv[]){
    first_part(argc, argv);

    /**
     * 1 Server = 1 Consumer
     * 2 Clients = 2 Producers
     *
     * SEMAPHORES INITIALIZATION
     * full: 0
     * empty: 1 (max 1 per time)
     * client_me: 1
     * **/

    int rc1, rc2;
    pthread_t thread[THREADS];

    // set up boxes to pass to threads
    box1.active = true;
    strcpy(box1.filename, "fv1.b");

    box2.active = true;
    strcpy(box2.filename, "fv2.b");

    // semaphores allocation
    client_me = (sem_t *)malloc(sizeof(sem_t));
    full = (sem_t *)malloc(sizeof(sem_t));
    empty = (sem_t *)malloc(sizeof(sem_t));

    // error check allocations
    if (client_me == NULL || empty == NULL || full == NULL) {
        // error
        perror("\n - (e) Error allocating semaphores!");
        exit(-1);
    }

    // init at 0
    if (sem_init(client_me, 0, 1) == -1) {
        // error
        perror("\n - (e) Error creating semaphore1!");
        exit(-1);
    }

    // init at 0
    if (sem_init(full, 0, 0) == -1) {
        // error
        perror("\n - (e) Error creating semaphore2!");
        exit(-1);
    }

    // init at 0
    if (sem_init(empty, 0, 0) == -1 ) {
        // error
        perror("\n - (e) Error creating semaphore3!");
        exit(-1);
    }

    // create 2 clients
    if ((rc1 = pthread_create(&thread[0], NULL, client, (void *) &box1)) != 0) {
        // error
        perror("\n - (e) Error creating thread1!");
        exit(-1);
    }

    if ((rc2 = pthread_create(&thread[1], NULL, client, (void *) &box2)) != 0) {
        // error
        perror("\n - (e) Error creating thread2!");
        exit(-1);
    }

    tot_requests = 0;;

    sem_post(empty);

    // start server
    while (true) {
        if (box1.active || box2.active) {
            //printf("Main %lu - 1\n", pthread_self());
            sem_wait(full);

            g = g * 3;
            //printf("Main %lu - 2\n", pthread_self());
            tot_requests++;

            sem_post(empty);
            //printf("Main %lu - 3\n", pthread_self());
        } else {
            printf("Main %lu - exiting\n", pthread_self());
            break;
        }
    }

    printf("Main %lu - exited\n", pthread_self());

    for (int i=0; i<THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    // release all variables
    sem_destroy(client_me);
    sem_destroy(full);
    sem_destroy(empty);

    fprintf(stdout, "All client terminated, total served requests: %d\n", tot_requests-2);

    pthread_exit(0);
}

void first_part(int argc, char *argv[]) {
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
    fpb1 = fopen("fv1.b", "wb");
    fpb2 = fopen("fv2.b", "wb");

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
void fill_array_with_rand_number(int *array, int array_size, int min_rand, int max_rand, bool even) {
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

void *client(void *box_passed) {
    Box *box;
    FILE *fp;
    int x;

    box = (Box *) box_passed;

    if ((fp = fopen(box->filename, "rb")) == NULL) {
        //error
        printf("error opening file 1\n");
        exit(-1);
    }

    while (fread(&x, sizeof(int), 1, fp) == 1) {
        sem_wait(empty);

        sem_wait(client_me);
        g = x;
        sem_post(client_me);

        sem_post(full);

        printf("Client %lu - before server process: %d\n", pthread_self(), x);
        printf("Client %lu - after server process %d\n", pthread_self(), g);
    }

    printf("Client %lu - exited\n", pthread_self());

    box->active = false;

    fclose(fp);

    pthread_exit(0);
}
