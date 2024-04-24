#include "headers.h"

struct msgbuff
{
    long mtype;
    int algotype;
    int quantum;
    struct Process process;
};

// Create message Queue
key_t key_id, key_id2;
int msgq_id, msgq_id2, rec_val, sen_val;
struct msgbuff message;

struct msgbuff2
{
    long mtype;
    struct Process process;
};

struct Process RunningProcess;

//=================================================================================================================
//============================================== ROUND-ROBIN ======================================================
//=================================================================================================================

void RR()
{
    struct Process *runningProcess = NULL;
    int quanta = 0;
    struct Queue *rrReadyQueue = createQueue();
    int prev_clk = -1;
    while (1)
    {
        int c = getClk();
        if (c != prev_clk)
        {
            // receive process
            int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, !IPC_NOWAIT);

            if (rec_val2 == -1)
            {
                perror("msgrcv error");
            }
            else
            {
                enqueue(rrReadyQueue, &message.process);

                printf("Message received successfully\n");
                printf("%d\n", rec_val2);
                printf("%ld\n", message.mtype);

                // running process?
                if (runningProcess == NULL)
                {
                    runningProcess = dequeue(rrReadyQueue);
                    quanta = 0;
                    if (runningProcess->processId == -1)
                    {
                        int pid = fork();
                        if (pid == 0)
                        {
                            // send el process to process.c
                            system("./process.out");
                        }
                        else
                        {
                            // parent
                            // set el id gowa el process
                            runningProcess->processId = pid;
                            // send message queue lel process bel data
                            key_id2 = ftok("keyfile", 15);
                            msgq_id2 = msgget(key_id, 0666 | IPC_CREAT);
                            struct msgbuff2 rrProcMsg;
                            rrProcMsg.process = *runningProcess;
                            rrProcMsg.mtype = 3;

                            if (msgq_id2 == -1)
                            {
                                perror("Error in create");
                                exit(-1);
                            }

                            sen_val = msgsnd(msgq_id2, &rrProcMsg, sizeof(rrProcMsg) - sizeof(long), !IPC_NOWAIT);
                            if (sen_val == -1)
                            {
                                perror("msgrcv error");
                            }
                        }
                    }
                    else
                    {
                        kill(runningProcess->processId, SIGCONT);
                    }
                }
                else
                {
                    quanta++;
                    runningProcess->pcb.remainingTime--;
                    // if (runningProcess->pcb.remainingTime == 0)
                    // {
                    //     runningProcess = dequeue(rrReadyQueue);
                    //     quanta = 0;
                    // }
                    /*else */ if (message.quantum == quanta)
                    {
                        kill(runningProcess->processId, SIGSTOP);
                        enqueue(rrReadyQueue, &runningProcess);
                        quanta = 0;
                        runningProcess = dequeue(rrReadyQueue);
                        if (runningProcess->processId == -1)
                        {
                            int pid = fork();
                            if (pid == 0)
                            {
                                // send el process to process.c
                                system("./process.out");
                            }
                            else
                            {
                                // parent
                                // set el id gowa el process
                                runningProcess->processId = pid;
                                // send message queue lel process bel data
                                key_id2 = ftok("keyfile", 15);
                                msgq_id2 = msgget(key_id, 0666 | IPC_CREAT);
                                struct msgbuff2 rrProcMsg;
                                rrProcMsg.process = *runningProcess;
                                rrProcMsg.mtype = 3;

                                if (msgq_id2 == -1)
                                {
                                    perror("Error in create");
                                    exit(-1);
                                }

                                sen_val = msgsnd(msgq_id2, &rrProcMsg, sizeof(rrProcMsg) - sizeof(long), !IPC_NOWAIT);
                                if (sen_val == -1)
                                {
                                    perror("msgrcv error");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

//=================================================================================================================
//========================================== SHORTEST-REAMAINING-TIME =============================================
//=================================================================================================================
void SRTN(struct Process p, struct Priority_Queue *ready_list)
{

    if (&RunningProcess == NULL)
    {
        struct Process *process = PriorityDequeue(ready_list);
        RunningProcess = *process;
        // fork process.c and send process struct
    }
    else
    {
        if (RunningProcess.pcb.remainingTime > p.pcb.remainingTime)
        {
            // preempt running process by sending signal interrupt and fork new process
            struct Process temp = RunningProcess;
            RunningProcess = p;
            PriorityEnqueue(ready_list, &temp, temp.pcb.remainingTime);
        }
        else
        {
            PriorityEnqueue(ready_list, &p, p.pcb.remainingTime);
        }
    }

    int pid = fork();
    if (pid == 0)
    {
        system("./process.out");
    }

    // running process and sending it to process.c
    key_t key_pid;
    int msgq_pid, send_srtn_val;
    key_pid = ftok("keyfile", 15);
    msgq_pid = msgget(key_pid, 0666 | IPC_CREAT);
    if (msgq_pid == -1)
    {
        perror("Error in create");
        exit(-1);
    }

    printf("Message Queue SRTN ID = %d\n", msgq_pid);
    struct msgbuff2 messageSRTN;
    messageSRTN.mtype = 3;
    messageSRTN.process = p;

    if (p.pcb.remainingTime == 0)
        printf("fuck\n");
    else
        printf("here we go. rem time is %d", p.pcb.remainingTime);

    send_srtn_val = msgsnd(msgq_pid, &messageSRTN, sizeof(messageSRTN.process) - sizeof(long), !IPC_NOWAIT);
    if (send_srtn_val == -1)
        perror("Error in send");
}
//======================================================================================================
//=============================================== MAIN =================================================
//======================================================================================================
int main(int argc, char *argv[])
{
    initClk();
    int algo;
    printf("Entered scheduler\n");

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

    // struct msgbuff2 message2;
    // TODO implement the scheduler :)
    // upon termination release the clock resources.

    // receive algorithm type
    rec_val = msgrcv(msgq_id, &message, sizeof(message.algotype), 1, !IPC_NOWAIT);
    if (rec_val == -1)
        perror("Error in receive");
    else
    {
        printf("\nAlgo Chosen: %d\n", message.algotype);
        algo = message.algotype;
    }

    if (message.algotype == 3)
    {
        RR();
    }

    struct Priority_Queue *ready_list = createPriorityQueue();

    while (1)
    {

        // receive process
        int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, !IPC_NOWAIT);
        if (rec_val2 == -1)
        {
            perror("msgrcv error");
        }
        else
        {
            printf("Message received successfully\n");
            printf("%d\n", rec_val2);
            printf("%ld\n", message.mtype);

            printf("id = %d while receiving with arr.time = %d and remaingtime = %d\n", message.process.id, message.process.arrivalTime, message.process.pcb.remainingTime);

            if (algo == 2)
            {

                SRTN(message.process, ready_list);
            }
        }

        // if(algo == 2) {
        //     SRTN(message2.process);
        // }
    }

    destroyClk(true);
}
