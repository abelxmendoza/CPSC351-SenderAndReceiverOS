/*

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "msg.h"    /* For the message struct */


/*
#define SHARED_MEMORY_CHUNK_SIZE 1000

int shmid, msqid;
void *sharedMemPtr = NULL;

char* recvFileName()
{
    char* fileName = NULL;
    fileNameMsg msg;
    msgrcv(msqid, &msg, sizeof(fileNameMsg) - sizeof(long), FILE_NAME_TRANSFER_TYPE, 0);
    fileName = malloc(strlen(msg.fileName) + 1);
    strcpy(fileName, msg.fileName);
    return fileName;
}

void init(int* shmid, int* msqid, void** sharedMemPtr)
{
    FILE* keyFile = fopen("keyfile.txt", "w");
    fputs("Hello world", keyFile);
    fclose(keyFile);

    key_t key = ftok("keyfile.txt", 'a');

    *shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT);
    *sharedMemPtr = shmat(*shmid, (void*)0, 0);
    *msqid = msgget(key, 0666 | IPC_CREAT);
}

unsigned long mainLoop(const char* fileName)
{
    int msgSize = -1;
    int numBytesRecv = 0;
    char* recvFileNameStr = malloc(strlen(fileName) + 7);
    strcpy(recvFileNameStr, fileName);
    strcat(recvFileNameStr, "__recv");

    FILE* fp = fopen(recvFileNameStr, "w");
    if (!fp)
    {
        perror("fopen");
        exit(-1);
    }

    while (msgSize != 0)
    {
        message msg;
        msgrcv(msqid, &msg, sizeof(message) - sizeof(long), SENDER_DATA_TYPE, 0);
        msgSize = msg.size;

        if (msgSize != 0)
        {
            numBytesRecv += msgSize;

            if (fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
            {
                perror("fwrite");
            }

            ackMessage ack;
            ack.mtype = RECV_DONE_TYPE;
            msgsnd(msqid, &ack, sizeof(ackMessage) - sizeof(long), 0);
        }
        else
        {
            fclose(fp);
        }
    }

    free(recvFileNameStr);
    return numBytesRecv;
}

void cleanUp(const int* shmid, const int* msqid, void* sharedMemPtr)
{
    shmdt(sharedMemPtr);
    shmctl(*shmid, IPC_RMID, NULL);
    msgctl(*msqid, IPC_RMID, NULL);
}

void ctrlCSignal(int signal)
{
    cleanUp(&shmid, &msqid, sharedMemPtr);
}

int main(int argc, char** argv)
{
    signal(SIGINT, ctrlCSignal);

    init(&shmid, &msqid, &sharedMemPtr);

    char* fileName = recvFileName();

    fprintf(stderr, "The number of bytes received is: %lu\n", mainLoop(fileName));

    cleanUp(&shmid, &msqid, sharedMemPtr);

    return 0;
}
*/

#include "msg2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>


#define SHARED_MEMORY_CHUNK_SIZE 1000
#define MESSAGE_QUEUE_SIZE 10

void init(int* shmid, int* msqid, void** sharedMemPtr) {
    key_t key = ftok("keyfile.txt", 'a');
    if (key == -1) {
        perror("ftok");
        exit(1);
    }

    *shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT);
    if (*shmid == -1) {
        perror("shmget");
        exit(1);
    }

    *sharedMemPtr = shmat(*shmid, (void*)0, 0);
    if (*sharedMemPtr == (void*)-1) {
        perror("shmat");
        exit(1);
    }

    *msqid = msgget(key, 0666 | IPC_CREAT);
    if (*msqid == -1) {
        perror("msgget");
        exit(1);
    }
}

void cleanUp(const int* shmid, const int* msqid, void* sharedMemPtr) {
    if (shmctl(*shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    if (msgctl(*msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (shmdt(sharedMemPtr) == -1) {
        perror("shmdt");
        exit(1);
    }
}

void recvFileName(const int msqid, char* fileName) {
    struct fileNameMsg msg;
    msgrcv(msqid, &msg, sizeof(struct fileNameMsg) - sizeof(long), FILE_NAME_TRANSFER_TYPE, 0);
    strcpy(fileName, msg.fileName);
}

void mainLoop(const int msqid, const char* fileName) {
    FILE* file = fopen(fileName, "w");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }

    struct message msg;
    int msgSize;
    while (1) {
        msgrcv(msqid, &msg, sizeof(struct message) - sizeof(long), SENDER_DATA_TYPE, 0);
        msgSize = msg.size;

        if (msgSize == 0) {
            break;
        }

        fwrite(msg.payload, sizeof(char), msgSize, file);

        struct ackMessage ack;
        ack.mtype = RECV_DONE_TYPE;
        msgsnd(msqid, &ack, sizeof(struct ackMessage) - sizeof(long), 0);
    }

    fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    int shmid, msqid;
    void* sharedMemPtr;

    init(&shmid, &msqid, &sharedMemPtr);

    char fileName[MAX_FILE_NAME_SIZE];
    recvFileName(msqid, fileName);
    printf("Received file name: %s\n", fileName);

    mainLoop(msqid, argv[1]);

    cleanUp(&shmid, &msqid, sharedMemPtr);

    return 0;
}
