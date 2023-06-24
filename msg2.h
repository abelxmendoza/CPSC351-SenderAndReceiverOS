#ifndef MSG_H
#define MSG_H

#define MAX_MSG_PAYLOAD 100

/* The information type -*/

#define SENDER_DATA_TYPE 1

/* The done message */
#define RECV_DONE_TYPE 2

/* The file name transfer message */
#define FILE_NAME_TRANSFER_TYPE 3

/* The maximum size of the file name */
#define MAX_FILE_NAME_SIZE 100

/**
 * The structure representing the message
 * used by sender to send the name of the file
 * to be transferred to the receiver.
 */
struct fileNameMsg
{
    /* The message type */
    long mtype;

    /* The name of the file */
    char fileName[MAX_FILE_NAME_SIZE];
};

/**
 * The message structure representing the message
 * sent from the sender to the receiver indicating
 * the number of bytes in the shared memory segment
 * that are ready to read.
 */
struct message
{
    /* The message type */
    long mtype;

    /* How many bytes in the message */
    int size;

    /* The payload data */
    char payload[MAX_MSG_PAYLOAD];
};

/* Struct representing the message sent from the receiver
 * to the sender acknowledging the successful reception and
 * saving of data.
 */
struct ackMessage
{
    /* The type of message */
    long mtype;
};

#endif
