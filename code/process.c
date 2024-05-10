#include "headers.h"
#define PROCESS_KEY 15
#define PROCESS_LOCK 3

/* Modify this file as needed*/
int remainingtime;
struct Process p;

struct msgbuff2
{
    long mtype;
    struct Process process;
};

int main(int agrc, char *argv[])
{
    initClk();

    key_t key_pid2 = ftok("keyfile", PROCESS_KEY);
    int msgq_pid2 = msgget(key_pid2, 0666 | IPC_CREAT);
    if (msgq_pid2 == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    struct msgbuff2 messageProcess;
    int rec_val2 = msgrcv(msgq_pid2, &messageProcess, sizeof(messageProcess) - sizeof(long), PROCESS_LOCK, !IPC_NOWAIT);
   
    if (rec_val2 == -1)
    {
        perror("msgrcv error");
    }

    // remainingtime = ??;
    while (messageProcess.process.pcb.remainingTime > 0)
    {
        int rec_val2 = msgrcv(msgq_pid2, &messageProcess, sizeof(messageProcess) - sizeof(long), PROCESS_LOCK, !IPC_NOWAIT);
    }

    exit(1);
    destroyClk(false);

    return 0;
}
