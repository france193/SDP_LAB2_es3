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
#include <stdbool.h>

#define N 50

// arguments for thread
typedef struct {
    bool active;
    char filename[N];
} Box;

sem_t *client_me, *request, *reply;
pthread_mutex_t mutex;

int g, tot_requests;
Box box1, box2;

/* PROTOTYPES */
void* Malloc(int size);
void fill_array_with_rand_number(int *array, int array_size, int min_rand, int max_rand, bool even);
void *client(void *box_passed);
void first_part(int argc, char *argv[]);
void second_part(void);

/* MAIN */
int main(int argc, char *argv[]){
    first_part(argc, argv);
    second_part();

    pthread_exit(0);
}

/* SERVER AND CLIENT CODE */
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

void second_part(void) {
    //mutex init & error_check
    if(pthread_mutex_init(&mutex, NULL)) {
        perror("MUTEX ERROR");
    }

    int rc1, rc2;
    pthread_t thread1, thread2;

    // set up boxes to pass to threads
    box1.active = true;
    strcpy(box1.filename, "fv1.b");

    box2.active = true;
    strcpy(box2.filename, "fv2.b");

    // semaphores allocation
    client_me = (sem_t *)malloc(sizeof(sem_t));
    request = (sem_t *)malloc(sizeof(sem_t));
    reply = (sem_t *)malloc(sizeof(sem_t));

    // error check allocations
    if (client_me == NULL || request == NULL || reply == NULL) {
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
    if (sem_init(request, 0, 0) == -1) {
        // error
        perror("\n - (e) Error creating semaphore2!");
        exit(-1);
    }

    // init at 0
    if (sem_init(reply, 0, 0) == -1 ) {
        // error
        perror("\n - (e) Error creating semaphore3!");
        exit(-1);
    }

    // create 2 clients
    if ((rc1 = pthread_create(&thread1, NULL, client, (void *) &box1)) != 0) {
        // error
        perror("\n - (e) Error creating thread1!");
        exit(-1);
    }

    if ((rc2 = pthread_create(&thread2, NULL, client, (void *) &box2)) != 0) {
        // error
        perror("\n - (e) Error creating thread2!");
        exit(-1);
    }

    tot_requests = -1;

    // start server
    while (true) {
        //wait for a request from client
        sem_wait(request);

        //if at leat one of client is active
        if(box1.active ||box2.active ){

            //edit global variable in ME
            pthread_mutex_lock(&mutex);
            g = g * 3;
            //sleep(1);
            pthread_mutex_unlock(&mutex);

            //reply to client
            sem_post(reply);

            tot_requests++;

            //exit from process
        }else{
            sem_post(reply);
            break;
        }
    }

    //synchronization at exit
    sem_wait(client_me);

    //destroy all semaphores and mutex
    sem_destroy(client_me);
    sem_destroy(request);
    sem_destroy(reply);
    pthread_mutex_destroy(&mutex);

    //wait for the end of all threads (to capture the status pass the variable exitstatus)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    //print request served
    fprintf(stdout, "All client terminated, total served requests: %d\n", tot_requests);
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
        sem_wait(client_me);

        g = x;

        printf("Client %lu - before server process: %d\n", pthread_self(), g);

        sem_post(request);
        sem_wait(reply);

        printf("Client %lu - after server process %d\n", pthread_self(), g);

        sem_post(client_me);
    }

    //wait for exit
    sem_wait(client_me);

    printf("Client %lu - exited\n", pthread_self());

    //client ends its work
    box->active = false;

    //synchronization to exit with server
    sem_post(request);
    sem_wait(reply);

    //wait end of server
    sem_post(client_me);

    //close file
    fclose(fp);

    //sleep(3);

    //exit at the end of thread
    pthread_exit(NULL);
}

/* OTHER FUNCTIONS */
void* Malloc(int size){
    void* ptr = malloc((size_t) size);

    if(ptr == NULL){
        printf("error allocating array\n");
        exit(-1);
    }

    return ptr;
}

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
