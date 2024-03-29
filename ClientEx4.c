/******************************************
*
Student name: Haim Rubinstein
*
Student ID: 203405386
*
Course Exercise Group:01
*
Exercise name:Ex41
******************************************/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <asm/errno.h>
#include <errno.h>

#define SHM_SIZE 4096

///declare the variables
union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};


void EndProgram();

void SetSems(int check);


union semun semarg;
char *memory;
int writeId, thirdMutexId, readId;
key_t memKey, thirdKey;
key_t key;
key_t writerKey;
key_t readerKey;
int shmid;
char *data;
char move;
union semun arg;
struct semid_ds buf;
struct sembuf sb;
int semid;

/**
 * the operation - the main function
 */
int main() {

    char dummy;
    char temp[2];
    int numRead =0;
    SetSems(0);

    /*create memory*/

    /* the memoty key */
    if ((memKey = ftok("203405386.txt", 'K')) == -1) {
        perror("failed ftok");
        exit(1);
    }

    /* connect to segment */
    if ((shmid = shmget(memKey, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to segment */
    data = shmat(shmid, NULL, 0);
    memory = data;
    if (data == (char *) (-1)) {
        perror("shmat");
        exit(1);
    }
    //run in loop that gets the action and perform it
    do {
        sb.sem_num = 0;
        sb.sem_op = -1;
        sb.sem_flg = SEM_UNDO;
        //try close(=wait) writer
        if (semop(writeId, &sb, 1) < 0) {
            if ((errno == EIDRM) || (errno == EINVAL)) {
                EndProgram();
            } else {
                perror("failed semop write");
                EndProgram();
            }
        }

        //close third(inner) sem
        if (semop(thirdMutexId, &sb, 1) < 0) {
            if ((errno == EIDRM) || (errno == EINVAL)) {
                EndProgram();
            } else {
                perror("failed semop write");
                EndProgram();
            }
        }
        /*ask user for input*/
        if (write(STDOUT_FILENO, "Please enter request code\n", strlen("Please enter request code\n")) < 0) {
            perror("failed to write to screen");
            exit(0);
        }
        //get input
        do{
            if(read(STDIN_FILENO,&temp[numRead %2],1)<0){
                perror("failed to read input");
            }
            numRead ++;
        }while (temp[(numRead-1) %2]!= '\n');
        if ((move != 'i') && (move != 'I') && (numRead ==2)) {
            //add item
            *data = temp[0];
        }
        //free all semaphors
        sb.sem_num = 0;
        sb.sem_op = 1;
        sb.sem_flg = SEM_UNDO;

        //release third sem
        if (semop(thirdMutexId, &sb, 1) < 0) {
            perror("failed semop in");
            EndProgram();
        }

        // release reader
        if (semop(readId, &sb, 1) < 0) {
            perror("failed semop read");
            EndProgram();
        }

        //check if need to finish
        if ((move == 'g') || (move == 'h') || (move == 'H') || (move == 'G')||(move == 'i')&&(move == 'I')) {
            EndProgram();
        }
        numRead =0;
    } while (1);
    return 0;
}

/**
 * the input - a flag if we need to wait
 * the operation - sets all the semaphors
 */
void SetSems(int check) {

    if (check == 1) {
        sleep(3);
    }
    /*write semaphore*/

    //create new key for the first semaphore.
    if ((writerKey = ftok("writeSemaphore.c", 'A')) == -1) {
        perror("failed ftok");
        exit(1);
    }

    //try getting the semaphore
    writeId = semget(writerKey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (writeId < 0) { /* someone else created it first */
        writeId = semget(writerKey, 1, 0); /* get the id */
        if (writeId < 0) {
            perror("failed creating semaphore");
            exit(0);
        }
    }

    /*read semaphore*/

    if ((readerKey = ftok("readSemaphore.c", 'B')) == -1) {
        write(STDERR_FILENO, "failed ftok", strlen("failed ftok"));
        exit(1);
    }
    //try getting the semaphore
    readId = semget(readerKey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (readId < 0) { /* someone else created it first */
        readId = semget(readerKey, 1, 0); /* get the id */
        if (readId < 0) {
            perror("failed creating semaphore");
            exit(0);
        }
    }

    /*third(inner) semaphore*/
    //create new key for the third semaphore.
    if ((thirdKey = ftok("semaphore.c", 'C')) == -1) {
        perror("failed creating semaphore");
        exit(0);
    }
    //try getting the semaphore
    thirdMutexId = semget(thirdKey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (thirdMutexId < 0) { /* someone else created it first */
        thirdMutexId = semget(thirdKey, 1, 0); /* get the id */
        if (thirdMutexId < 0) {
            perror("failed creating semaphore");
            exit(0);
        }
    }
}

/**
 * the operation - release all resources and exit the program
 */
void EndProgram() {

    if (shmdt(memory) < 0) {
    }
    exit(0);
}