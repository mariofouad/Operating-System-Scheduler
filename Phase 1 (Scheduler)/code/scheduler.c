#include "headers.h"

struct msgbuff
{
    long mtype;
    int algotype;
    int quantum;
    struct Process process;
};

struct msgbuff2
{
    long mtype;
    struct Process process;
};

// Create message Queue
key_t key_id, key_id2;
int msgq_id, msgq_id2, rec_val, sen_val;
struct msgbuff message;

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
    int pid;
    while (1)
    {
        int c = getClk();
        if (c != prev_clk)
        {
            // receive process
            int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, IPC_NOWAIT);
            while (rec_val2 != -1)
            {
                struct Process temp = message.process;
                struct Process *newProcess = Create_Process(message.process.id, message.process.arrivalTime, message.process.runTime, message.process.priority);
                enqueue(rrReadyQueue, newProcess);
                printf("Message received successfully in ROUND-ROBIN\n");
                printQueue(rrReadyQueue);
                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, IPC_NOWAIT);
            }

            // running process?
            if (isEmpty(rrReadyQueue) != 1 && runningProcess == NULL)
            {
                printf("//=============== new process ======================//\n");
                printQueue(rrReadyQueue);

                runningProcess = dequeue(rrReadyQueue);
                printQueue(rrReadyQueue);

                if (isEmpty(rrReadyQueue) != 1)
                    printf("after dequeue : %d,  %d\n", peek(rrReadyQueue)->id, peek(rrReadyQueue)->pcb.remainingTime);
                else
                    printf("after dequeue, queue is empty\n");

                printf("new running process remaining time: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);
                quanta = 0;
                if (runningProcess->processId == -1)
                {
                    pid = fork();

                    if (pid == 0)
                    {
                        // send el process to process.c
                        system("./process.out");
                    }
                }
                else
                {
                    //kill(runningProcess->processId, SIGCONT);
                }

                // set el id gowa el process
                runningProcess->processId = pid;
                // send message queue lel process bel data
                key_id2 = ftok("keyfile", 15);
                msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);

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
            else if (runningProcess != NULL)
            {
                quanta++;
                runningProcess->pcb.remainingTime--;

                printf("QUANTA %d\n", quanta);
                printf("running process remaining time after decrement: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

                // set el id gowa el process
                runningProcess->processId = pid;
                // send message queue lel process bel data
                key_id2 = ftok("keyfile", 15);
                msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);

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
                if (runningProcess->pcb.remainingTime == 0)
                {
                    printf("REMAINING IS ZERO\n");
                    runningProcess = NULL;

                    quanta = 0;
                }
                else if (message.quantum == quanta)
                {
                    printf("QUANTA REACHED\n");
                    printf("running process remaining time after quanta reached: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

                    if (isEmpty(rrReadyQueue) != 1)
                        printf("last process in the queue before enqueue %d,  %d\n", peek(rrReadyQueue)->id, peek(rrReadyQueue)->pcb.remainingTime);
                    else
                        printf("quanta reached and ready is empty\n");

                    //kill(runningProcess->processId, SIGSTOP);

                    struct Process *process = runningProcess;
                    printQueue(rrReadyQueue);

                    enqueue(rrReadyQueue, process);
                    printQueue(rrReadyQueue);

                    printf("last process in the queue after enqueue %d,  %d\n", peek(rrReadyQueue)->id, peek(rrReadyQueue)->pcb.remainingTime);
                    printf("\n");

                    quanta = 0;
                    runningProcess = NULL;
                }
            }
            prev_clk = c;
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
