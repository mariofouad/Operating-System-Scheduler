#include "headers.h"

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

    // TODO it needs to get the remaining time from somewhere

    key_t key_pid2 = ftok("keyfile", 15);
    int msgq_pid2 = msgget(key_pid2, 0666 | IPC_CREAT);
    if (msgq_pid2 == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    struct msgbuff2 messageProcess;
    int rec_val2 = msgrcv(msgq_pid2, &messageProcess, sizeof(messageProcess) - sizeof(long), 3, !IPC_NOWAIT);

    if (rec_val2 == -1)
    {
        perror("msgrcv error");
    }
    else
    {
        printf("Message received in process successfully\n");
        printf("%d\n", rec_val2);
        printf("%ld\n", messageProcess.mtype);
        printf("%d\n", messageProcess.process.id);

        // if(message2.process == NULL) printf("null\n");
        printf("Process running with id = %d while receiving with arr.time = %d and remtime = %d\n", messageProcess.process.id, messageProcess.process.runTime, messageProcess.process.pcb.remainingTime);
    }

    // remainingtime = ??;
    while (messageProcess.process.pcb.remainingTime > 0)
    {
        printf("insid process while loop remaining time = %d\n", messageProcess.process.pcb.remainingTime);
        // remainingtime = ??;
        int rec_val2 = msgrcv(msgq_pid2, &messageProcess, sizeof(messageProcess) - sizeof(long), 3, !IPC_NOWAIT);
    }

    // terminate process
    printf("Process terminated with id: %d\n", getpid());
    kill(getpid(), SIGKILL);

    //exit(1);
    destroyClk(false);

    return 0;
}
