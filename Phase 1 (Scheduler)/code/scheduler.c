#include "headers.h"

struct msgbuff
{
    long mtype;
    int algotype;
    struct Process* process;
};

struct msgbuff2
{
    long mtype;
    struct Process* process;
};


int main(int argc, char * argv[])
{
    initClk();
    int algo;
    printf("Entered scheduler\n");

     //Create message Queue
    key_t key_id,key_id2;
    int msgq_id,msgq_id2, rec_val;

    key_id = ftok("keyfile", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);
    // IPC_PRIVATE: create a new queue every time
    // IPC_EXCL: return -1 if the queue already exists else creates it
    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue ID = %d\n", msgq_id);

    key_id2 = ftok("keyfile", 66);
    msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);
    // IPC_PRIVATE: create a new queue every time
    // IPC_EXCL: return -1 if the queue already exists else creates it
    if (msgq_id2 == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    printf("Message Queue 2 ID = %d\n", msgq_id2);

    struct msgbuff message;
    struct msgbuff2 message2;
    //TODO implement the scheduler :)
    //upon termination release the clock resources.
    while (1)
    {
        //receive algorithm type
        rec_val = msgrcv(msgq_id, &message, sizeof(message.algotype), 1, !IPC_NOWAIT);
        if (rec_val == -1)
            perror("Error in receive");
        else
            {
                printf("\nAlgo Chosen: %d\n", message.algotype);
                algo=message.algotype;
            }

        //receive process
        rec_val = msgrcv(msgq_id2, &message2, sizeof(struct Process*), 2, !IPC_NOWAIT);
        if (rec_val == -1)
            perror("Error in receive");
        else
            {
                struct Process* p=message2.process;
                printf("Received process with id %d\n",p->id);
            }
    }
    
    destroyClk(true);
}
