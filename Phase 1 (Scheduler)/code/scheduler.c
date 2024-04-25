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

struct Process *runningProcess = NULL;
//=================================================================================================================
//============================================== HIGHEST-PRIORITY-FIRST ===========================================
//=================================================================================================================
void HPF()
{
    struct Priority_Queue *HPFReadyQueue = createPriorityQueue();
    int prev_clk = -1;
    int pid;
    int wPid, status;
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
                PriorityEnqueue(HPFReadyQueue, newProcess, newProcess->priority);
                printPriorityQueue(HPFReadyQueue);
                printf("Message received successfully in HPF\n");
                printf("THIS PROCESS IS %d\n", message.process.id);
                printPriorityQueue(HPFReadyQueue);
                printf("\n");

                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, IPC_NOWAIT);
            }

            // running process?
            if (PQisEmpty(HPFReadyQueue) != 1 && runningProcess == NULL)
            {
                printf("//=============== new process ======================//\n");
                printPriorityQueue(HPFReadyQueue);

                runningProcess = PriorityDequeue(HPFReadyQueue);
                printPriorityQueue(HPFReadyQueue);

                if (PQisEmpty(HPFReadyQueue) != 1)
                    printf("after dequeue : %d,  %d\n", PriorityPeek(HPFReadyQueue)->id, PriorityPeek(HPFReadyQueue)->pcb.remainingTime);
                else
                    printf("after dequeue, queue is empty\n");

                printf("new running process remaining time: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

                if (runningProcess->processId == -1)
                {
                    pid = fork();

                    if (pid == 0)
                    {
                        // send el process to process.c
                        execl("./process.out", "process.out", NULL);
                    }
                    else
                    {
                        // set el id gowa el process
                        runningProcess->processId = pid;
                    }
                }

                // send message queue lel process bel data
                key_id2 = ftok("keyfile", 15);
                msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);

                struct msgbuff2 HPFProcMsg;
                HPFProcMsg.process = *runningProcess;
                HPFProcMsg.mtype = 3;

                if (msgq_id2 == -1)
                {
                    perror("Error in create");
                    exit(-1);
                }

                sen_val = msgsnd(msgq_id2, &HPFProcMsg, sizeof(HPFProcMsg) - sizeof(long), !IPC_NOWAIT);
                if (sen_val == -1)
                {
                    perror("msgrcv error");
                }
            }

            else if (runningProcess != NULL)
            {
                runningProcess->pcb.remainingTime--;

                printf("running process remaining time after decrement: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

                // send message queue lel process bel data
                key_id2 = ftok("keyfile", 15);
                msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);

                struct msgbuff2 HPFProcMsg;
                HPFProcMsg.process = *runningProcess;
                HPFProcMsg.mtype = 3;

                if (msgq_id2 == -1)
                {
                    perror("Error in create");
                    exit(-1);
                }

                sen_val = msgsnd(msgq_id2, &HPFProcMsg, sizeof(HPFProcMsg) - sizeof(long), !IPC_NOWAIT);
                if (sen_val == -1)
                {
                    perror("msgrcv error");
                }
                if (runningProcess->pcb.remainingTime == 0)
                {
                    wPid = wait(&status);
                    printf("ANA ALLY 3MLT EXIT : %d\n", wPid);
                    printf("REMAINING IS ZERO\n");
                    runningProcess = NULL;
                }
            }
            prev_clk = c;
        }
    }
}

//=================================================================================================================
//============================================== ROUND-ROBIN ======================================================
//=================================================================================================================

void RR()
{
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
                        execl("./process.out", "process.out", NULL);
                    }
                }
                else
                {
                    kill(runningProcess->processId, SIGCONT);
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
                    {
                        printf("quanta reached and ready is empty\n");
                    }
                    kill(runningProcess->processId, SIGSTOP);
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
void SRTN()
{
    struct Priority_Queue *SRTNReadyQueue = createPriorityQueue();
    int prev_clk = -1;
    int pid;
    int wPid, status;
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

                // if (runningProcess != NULL && newProcess->pcb.remainingTime < runningProcess->pcb.remainingTime)
                // {
                //     struct Process *temp = runningProcess;
                //     PriorityEnqueue(SRTNReadyQueue, temp, temp->pcb.remainingTime);
                //     runningProcess = newProcess;
                // }
                // else
                // {
                PriorityEnqueue(SRTNReadyQueue, newProcess, newProcess->pcb.remainingTime);
                //}

                printf("Message received successfully in SRTN\n");
                printPriorityQueue(SRTNReadyQueue);

                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, IPC_NOWAIT);
            }

            // running process?
            if (PQisEmpty(SRTNReadyQueue) != 1 && runningProcess == NULL)
            {
                printf("//=============== new process ======================//\n");
                printPriorityQueue(SRTNReadyQueue);

                runningProcess = PriorityDequeue(SRTNReadyQueue);

                if (PQisEmpty(SRTNReadyQueue) != 1)
                    printf("after dequeue : %d,  %d\n", PriorityPeek(SRTNReadyQueue)->id, PriorityPeek(SRTNReadyQueue)->pcb.remainingTime);
                else
                    printf("after dequeue, queue is empty\n");

                printf("new running process remaining time: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

                if (runningProcess->processId == -1)
                {
                    pid = fork();

                    if (pid == 0)
                    {
                        // send el process to process.c
                        execl("./process.out", "process.out", NULL);
                    }
                    else
                    {
                        // set el id gowa el process
                        runningProcess->processId = pid;
                    }
                }
                else
                {
                    printf("here continuing\n");
                    printf("continuing with id = %d and process id = %d", runningProcess->id, runningProcess->processId);
                    kill(runningProcess->processId, SIGCONT);
                }

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
                if (PQisEmpty(SRTNReadyQueue) != 1)
                {
                    if (runningProcess->pcb.remainingTime > PriorityPeek(SRTNReadyQueue)->pcb.remainingTime)
                    {
                        printf("HAL ANA DA5LT HENA ? ANA : %d \n", runningProcess->processId);
                        kill(runningProcess->processId, SIGSTOP);
                        PriorityEnqueue(SRTNReadyQueue, runningProcess, runningProcess->pcb.remainingTime);
                        runningProcess = NULL;
                    }
                    else
                    {
                        runningProcess->pcb.remainingTime--;
                        printf("running process remaining time after decrement: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

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
                            wPid = wait(&status);
                            printf("Exited with id = %d\n", runningProcess->processId);
                            printf("REMAINING IS ZERO\n");
                            runningProcess = NULL;
                        }
                    }
                }
                else
                {
                    runningProcess->pcb.remainingTime--;
                    printf("running process remaining time after decrement: %d,  %d\n", runningProcess->id, runningProcess->pcb.remainingTime);

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
                        wPid = wait(&status);
                        printf("Exited with id = %d\n", runningProcess->processId);
                        printf("REMAINING IS ZERO\n");
                        runningProcess = NULL;
                    }
                }
            }

            prev_clk = c;
        }
    }
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
    else if (message.algotype == 1)
    {
        HPF();
    }
    else if (message.algotype == 2)
    {
        SRTN();
    }

    // struct Priority_Queue *ready_list = createPriorityQueue();

    // while (1)
    // {

    //     // receive process
    //     int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), 2, !IPC_NOWAIT);
    //     if (rec_val2 == -1)
    //     {
    //         perror("msgrcv error");
    //     }
    //     else
    //     {
    //         printf("Message received successfully\n");
    //         printf("%d\n", rec_val2);
    //         printf("%ld\n", message.mtype);

    //         printf("id = %d while receiving with arr.time = %d and remaingtime = %d\n", message.process.id, message.process.arrivalTime, message.process.pcb.remainingTime);

    //         if (algo == 2)
    //         {

    //             SRTN(message.process, ready_list);
    //         }
    //     }

    //     // if(algo == 2) {
    //     //     SRTN(message2.process);
    //     // }
    // }

    destroyClk(true);
}
