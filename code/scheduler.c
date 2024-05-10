#include "headers.h"
#include <math.h>
#define SCHEDULER_KEY 65
#define SCHEDULER_LOCK 2
#define SCHEDULER_INFO_LOCK 1
#define PROCESS_KEY 15
#define PROCESS_LOCK 3
#define STARTED_STATE 0
#define RESUMED_STATE 1
#define STOPPED_STATE 2
#define FINISHED_STATE 3
#define NEW_PROCESS -1
#define EMPTY_MESSAGE_QUEUE -1
#define CHILD_PROCESS 0
#define ERROR -1
#define TRUE_CONDITION 1
#define max_memory_size 1024
#define max_process_size 256
#define sleep_seconds 200000

struct msgbuff
{
    long mtype;
    int algotype;
    int quantum;
    struct Process process;
    int NumOfProcesses;
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
int algo;
int Processno;
int NumOfTerminatedProcesses = 0;
int TimeWhileRunning = 0;
double TotalWeightedTRT = 0;
int TotalWaitingTimes = 0;
double *TotalWeightedTRTs;
struct Process *runningProcess = NULL;
TreeNode *memory_tree;
struct Queue *BlockedQueue;
// clear all resources after termenation
void ClearResources(int signum);

//=================================================================================================================
//=================================================== OUTPUT-FILE =================================================
//=================================================================================================================

// Calculate the processing needed calculations for output files
// and make the scheduler.perf file
void PrintAllocatedMemoryState(TreeNode *treeNode, struct Process *process)
{
    // waiting time of the process taking preemption in cosideration
    FILE *file = fopen("memory.log", "a");
    int clk = getClk();
    fprintf(file, "At time %d allocated %d bytes for process %d from %d to %d\n", clk, process->memsize, process->id, treeNode->stIndex, treeNode->endIndex);

    fclose(file);
}

void PrintFreedMemoryState(TreeNode *root, int NodeId)
{
    if (root == NULL)
    {
        return;
    }

    if (root->left && root->left->pid == NodeId)
    {
        FILE *file = fopen("memory.log", "a");
        int clk = getClk();
        fprintf(file, "At time %d freed %d bytes for process %d from %d to %d\n", clk, runningProcess->memsize, runningProcess->id, root->left->stIndex, root->left->endIndex);
        fclose(file);
        return;
    }

    if (root->right && root->right->pid == NodeId)
    {
        FILE *file = fopen("memory.log", "a");
        int clk = getClk();
        fprintf(file, "At time %d freed %d bytes for process %d from %d to %d\n", clk, runningProcess->memsize, runningProcess->id, root->right->stIndex, root->right->endIndex);
        fclose(file);
        return;
    }

    PrintFreedMemoryState(root->left, NodeId);
    PrintFreedMemoryState(root->right, NodeId);
}

bool AssignToBuddySystem(struct Process *process)
{
    int size = max_process_size;
    while (process->memsize <= size && process->memsize <= size / 2)
    {
        size /= 2;
    }
    TreeNode *allocatedNode = allocate_memory(memory_tree, size, process->id);

    if (allocatedNode)
    {
        PrintAllocatedMemoryState(allocatedNode, process);
        return true;
    }
    else
        return false;
}

void FreeFromBuddySystem(int id)
{
    PrintFreedMemoryState(memory_tree, id);
    TreeNode *deallocatedNode = deallocate_memory(memory_tree, id);
}

void Calculate()
{
    int FinalFinishTime = getClk();
    double CPUUtilization = ((double)TimeWhileRunning / (double)FinalFinishTime) * 100;
    double AvgWeightedTRT = (double)TotalWeightedTRT / (double)Processno;
    double AvgWaitingTime = (double)TotalWaitingTimes / (double)Processno;
    double Variance = 0;
    for (int i = 0; i < Processno; i++)
        Variance += pow((TotalWeightedTRTs[i] - AvgWeightedTRT), 2);
    Variance /= (double)Processno;
    double StandardDeviationWTA = sqrt(Variance);

    free(TotalWeightedTRTs);
    FILE *file = fopen("scheduler.perf", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }
    fprintf(file, "CPU utilization = %.2f%%\n", CPUUtilization);
    fprintf(file, "Avg WTA = %.2f\n", AvgWeightedTRT);
    fprintf(file, "Avg Waiting = %.2f\n", AvgWaitingTime);
    fprintf(file, "Std WTA = %.2f", StandardDeviationWTA);
    fclose(file);
}

// make the titile for the scheduler.log file
void InitializePrintFile()
{
    FILE *file = fopen("scheduler.log", "w");
    fprintf(file, "#At time x Process y state arr w total z remain y wait k\n");
    fclose(file);
}

// This function is used to write a new line in the .log file at each state in the process life cycle
void PrintCurrentState(struct Process *p)
{
    // waiting time of the process taking preemption in cosideration
    p->pcb.WaitingtimeSoFar = getClk() - p->arrivalTime - p->runTime + p->pcb.remainingTime;
    FILE *file = fopen("scheduler.log", "a");
    int remTime = p->pcb.remainingTime, waitTime = p->pcb.WaitingtimeSoFar;

    if (p->pcb.state != FINISHED_STATE)
    {
        if (p->pcb.state == STARTED_STATE)
        {
            printf("Process started with id: %d\n", p->processId);
            fprintf(file, "At time %d Process %d started arr %d total %d remain %d wait %d\n", getClk(), p->id, p->arrivalTime, p->runTime, remTime, waitTime);
        }
        if (p->pcb.state == RESUMED_STATE)
        {
            printf("Process resumed with id: %d\n", p->processId);
            fprintf(file, "At time %d Process %d resumed arr %d total %d remain %d wait %d\n", getClk(), p->id, p->arrivalTime, p->runTime, remTime, waitTime);
        }
        if (p->pcb.state == STOPPED_STATE)
        {
            printf("Process stopped with id: %d\n", p->processId);
            fprintf(file, "At time %d Process %d stopped arr %d total %d remain %d wait %d\n", getClk(), p->id, p->arrivalTime, p->runTime, remTime, waitTime);
        }
    }
    else
    {
        printf("Process finished with id: %d\n", p->processId);
        fprintf(file, "At time %d Process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), p->id, p->arrivalTime, p->runTime, remTime, waitTime, p->pcb.TurnaroundTime, p->pcb.WeightedTurnaroundTime);
    }
    fclose(file);
}

void InitializeMemoryFile()
{
    FILE *file = fopen("memory.log", "w");
    fprintf(file, "#At time x allocated y bytes for process z from i to j\n");
    fclose(file);
}

//=================================================================================================================
//============================================== HIGHEST-PRIORITY-FIRST ===========================================
//=================================================================================================================

/// Highest priority first algorithm, this algorithms is considering the priority number at the first place
/// Initially the Algorithm initializes a priority queue, variable for storing the clk
/// then the algorithms starts to receive the processes at the arrival times of these processes
/// making sure to receive processes received at same arrival time
/// and stores them in the priority ready queue
/// starting the processing by checking that the ready queue is not empty and there is no running processes
/// then take a new process from the ready queue and check if the process is processed before or its first time
/// if it is its first time a new process is forked and its id is stored in the process
/// update its pcb variables and send the process to the process file
/// and run this process until finish its run time and make same process for all processes
/// at the end check if the scheduling is finishing by checking if ready is empty
/// and number of scheduled processes equal to total number of processes
void HPFSwitching(struct Priority_Queue *HPFReadyQueue, struct msgbuff2 HPFProcMsg, int c)
{
    int pid;
    runningProcess = PriorityDequeue(HPFReadyQueue);

    // check if it is a new process
    if (runningProcess->processId == NEW_PROCESS)
    {
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrivalTime - runningProcess->runTime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = STARTED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrivalTime;

        // fork new process
        pid = fork();
        if (pid == CHILD_PROCESS)
        {
            // send el process to process.c
            execl("./process.out", "process.out", NULL);
        }
        else
        {
            // save the id in the process
            runningProcess->processId = pid;
        }
        // add the new state to the output file
        PrintCurrentState(runningProcess);
    }

    // send message queue to process file with the running process
    HPFProcMsg.process = *runningProcess;

    sen_val = msgsnd(msgq_id2, &HPFProcMsg, sizeof(HPFProcMsg) - sizeof(long), !IPC_NOWAIT);
    if (sen_val == ERROR)
    {
        perror("msgrcv error");
    }
}

void HPF()
{
    // create the ready queue
    struct Priority_Queue *HPFReadyQueue = createPriorityQueue();
    int prev_clk = -1;
    int pid;
    int wPid, status;
    key_id2 = ftok("keyfile", PROCESS_KEY);
    msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);
    struct msgbuff2 HPFProcMsg;
    HPFProcMsg.mtype = PROCESS_LOCK;
    if (msgq_id2 == ERROR)
    {
        perror("Error in create");
        exit(ERROR);
    }
    printf("//=====================================================================================================================//\n");
    printf("//=============================================== WELCOME TO HPF ALGORITHM ============================================//\n");
    printf("//=====================================================================================================================//\n\n");

    while (TRUE_CONDITION)
    {
        int c = getClk();
        if (c != prev_clk)
        {
            printf("CLK IN SCH %d\n", c);
            usleep(sleep_seconds);
            // receive process
            int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);
            while (rec_val2 != EMPTY_MESSAGE_QUEUE)
            {
                // receive the process and initialize the new processes
                struct Process *newProcess = Create_Process(message.process.id, message.process.arrivalTime, message.process.runTime, message.process.priority, message.process.memsize);
                printf("received process id %d at time %d\n", newProcess->id, getClk());

                // try to allocate memory for arrived process
                // if allocated then add to the queue, else enqueue in BLK queue
                if (AssignToBuddySystem(newProcess))
                {
                    printf("I AM HERE ASIGNED TO PRIORITY QUEUE\n");
                    // store the processes in ready queue
                    PriorityEnqueue(HPFReadyQueue, newProcess, newProcess->priority);
                }
                else
                {
                    printf("MEMORY IS FULL ASIGNED TO BLOCKED QUEUE\n");
                    enqueue(BlockedQueue, newProcess);
                }

                // receive another processes if there are many at same arrival time
                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);
            }

            // check if ready is not empty and no process is running
            if (PQisEmpty(HPFReadyQueue) != TRUE_CONDITION && runningProcess == NULL)
            {
                HPFSwitching(HPFReadyQueue, HPFProcMsg, c);
            }

            // if there is a process running
            else if (runningProcess != NULL)
            {
                // decrement running time
                runningProcess->pcb.remainingTime--;
                // increment the running time
                TimeWhileRunning++;

                // send message queue to process file with the running process
                HPFProcMsg.process = *runningProcess;

                sen_val = msgsnd(msgq_id2, &HPFProcMsg, sizeof(HPFProcMsg) - sizeof(long), !IPC_NOWAIT);
                if (sen_val == ERROR)
                {
                    perror("msgrcv error");
                }
                // check if the process finished its running time
                if (runningProcess->pcb.remainingTime == 0)
                {
                    wPid = wait(&status);

                    // update the process turnaround time, weighted turnaround time, and total weighted turnaround time
                    runningProcess->pcb.TurnaroundTime = c - runningProcess->arrivalTime;
                    runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / (double)runningProcess->runTime;
                    TotalWeightedTRT += runningProcess->pcb.WeightedTurnaroundTime;
                    TotalWeightedTRTs[NumOfTerminatedProcesses] = runningProcess->pcb.WeightedTurnaroundTime;

                    // increment number of terminated processes
                    NumOfTerminatedProcesses++;

                    // update process state
                    runningProcess->pcb.state = FINISHED_STATE;

                    // add the state to output file
                    PrintCurrentState(runningProcess);
                    TotalWaitingTimes += runningProcess->pcb.WaitingtimeSoFar;
                    FreeFromBuddySystem(runningProcess->id); // free the finished process from the buddy system
                    runningProcess = NULL;

                    if (isEmpty(BlockedQueue) != TRUE_CONDITION)
                    {
                        struct Process *toBeAddedProcess = peek(BlockedQueue);
                        if (AssignToBuddySystem(toBeAddedProcess))
                        {
                            PriorityEnqueue(HPFReadyQueue, toBeAddedProcess, toBeAddedProcess->priority);
                            dequeue(BlockedQueue);
                        }
                    }

                    // run another process
                    if (PQisEmpty(HPFReadyQueue) != TRUE_CONDITION)
                    {
                        HPFSwitching(HPFReadyQueue, HPFProcMsg, c);
                    }
                }
            }
            // check if the scheduling is finished
            if (PQisEmpty(HPFReadyQueue) == TRUE_CONDITION && runningProcess == NULL && NumOfTerminatedProcesses == Processno)
            {
                // print the .perf file and clear the process-scheduler message queues
                Calculate();
                msgctl(msgq_id, IPC_RMID, NULL);
                msgctl(msgq_id2, IPC_RMID, NULL);
                shmdt(NULL);
                exit(1);
            }
            prev_clk = c;
        }
    }
}

//=================================================================================================================
//============================================== ROUND-ROBIN ======================================================
//=================================================================================================================

/// Round Robin algorithm, this algorithms is considering the quantum number qiven to run processes
/// Initially the Algorithm initializes a ready queue, variable for storing the clk
/// then the algorithms starts to receive the processes at the arrival times of these processes
/// making sure to receive processes received at same arrival time
/// and stores them in the ready queue
/// starting the processing by checking that the ready queue is not empty and there is no running processes
/// then take a new process from the ready queue and check if the process is processed before or its first time
/// if it is its first time a new process is forked and its id is stored in the process
/// update its pcb variables and send the process to the process file
/// and run this process until finish its run time or the quanta is reached
/// and in case preemtion the data is saved before re inserting in the ready queue and retreived when resuming
/// and make same process for all processes
/// at the end check if the scheduling is finishing by checking if ready is empty
/// and number of scheduled processes equal to total number of processes
void RRPreemption(struct Queue *rrReadyQueue, struct msgbuff2 rrProcMsg, int c)
{
    int pid;
    runningProcess = dequeue(rrReadyQueue);

    if (runningProcess->processId == NEW_PROCESS)
    {
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrivalTime - runningProcess->runTime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = STARTED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrivalTime;

        pid = fork();

        if (pid == CHILD_PROCESS)
        {
            // send el process to process.c
            execl("./process.out", "process.out", NULL);
        }
        // set the id of the new process
        runningProcess->processId = pid;
        PrintCurrentState(runningProcess);
    }
    // if not then it is a resuming process
    else
    {
        runningProcess->pcb.state = RESUMED_STATE;
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrivalTime - runningProcess->runTime + runningProcess->pcb.remainingTime;
        PrintCurrentState(runningProcess);
        // send a signal to start the process again
        kill(runningProcess->processId, SIGCONT);
    }

    rrProcMsg.process = *runningProcess;

    sen_val = msgsnd(msgq_id2, &rrProcMsg, sizeof(rrProcMsg) - sizeof(long), !IPC_NOWAIT);
    if (sen_val == ERROR)
    {
        perror("msgrcv error");
    }
}

void RR()
{
    int quanta = 0;
    // create ready queue
    struct Queue *rrReadyQueue = createQueue();
    int prev_clk = -1;
    int pid;
    struct msgbuff2 rrProcMsg;
    key_id2 = ftok("keyfile", PROCESS_KEY);
    msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);
    rrProcMsg.mtype = PROCESS_LOCK;

    if (msgq_id2 == ERROR)
    {
        perror("Error in create");
        exit(ERROR);
    }

    printf("//=====================================================================================================================//\n");
    printf("//=============================================== WELCOME TO RR ALGORITHM =============================================//\n");
    printf("//=====================================================================================================================//\n\n");

    while (TRUE_CONDITION)
    {
        int c = getClk();
        if (c != prev_clk)
        {
            printf("CLK IN SCH %d\n", c);
            usleep(sleep_seconds);
            // receive process
            int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);

            while (rec_val2 != EMPTY_MESSAGE_QUEUE)
            {
                // receive the process and initialize the new processes
                struct Process *newProcess = Create_Process(message.process.id, message.process.arrivalTime, message.process.runTime, message.process.priority, message.process.memsize);

                // try to allocate memory for arrived process
                // if allocated then add to the queue, else enqueue in BLK queue
                if (AssignToBuddySystem(newProcess))
                {
                    printf("I AM HERE ASIGNED TO PRIORITY QUEUE\n");
                    // store the processes in ready queue
                    enqueue(rrReadyQueue, newProcess);
                }
                else
                {
                    printf("MEMORY IS FULL ASIGNED TO BLOCKED QUEUE\n");
                    enqueue(BlockedQueue, newProcess);
                }

                // receive the other processes arrive at same arrival time
                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);
            }

            // check if the ready is not empty and no process is running
            if (isEmpty(rrReadyQueue) != TRUE_CONDITION && runningProcess == NULL)
            {

                quanta = 0;
                RRPreemption(rrReadyQueue, rrProcMsg, c);
            }
            // if there is a process running
            else if (runningProcess != NULL)
            {
                // increment quanta, time while running time, and decrement the remaining time
                quanta++;
                runningProcess->pcb.remainingTime--;
                TimeWhileRunning++;

                // send the process to the process file
                rrProcMsg.process = *runningProcess;

                sen_val = msgsnd(msgq_id2, &rrProcMsg, sizeof(rrProcMsg) - sizeof(long), !IPC_NOWAIT);
                if (sen_val == ERROR)
                {
                    perror("msgrcv error");
                }

                // if the running time is finished
                if (runningProcess->pcb.remainingTime == 0)
                {
                    // calculate the turnaround time, weighted turnaround time, and the total weighted turnaround time
                    runningProcess->pcb.TurnaroundTime = c - runningProcess->arrivalTime;
                    runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / (double)runningProcess->runTime;
                    TotalWeightedTRT += runningProcess->pcb.WeightedTurnaroundTime;
                    TotalWeightedTRTs[NumOfTerminatedProcesses] = runningProcess->pcb.WeightedTurnaroundTime;
                    // increment number of terminated
                    NumOfTerminatedProcesses++;
                    runningProcess->pcb.state = FINISHED_STATE;
                    PrintCurrentState(runningProcess);
                    TotalWaitingTimes += runningProcess->pcb.WaitingtimeSoFar;
                    FreeFromBuddySystem(runningProcess->id); // free the finished process from the buddy system

                    runningProcess = NULL;
                    // reset quanta
                    quanta = 0;

                    if (isEmpty(BlockedQueue) != TRUE_CONDITION)
                    {
                        struct Process *toBeAddedProcess = peek(BlockedQueue);
                        if (AssignToBuddySystem(toBeAddedProcess))
                        {
                            enqueue(rrReadyQueue, toBeAddedProcess);
                            dequeue(BlockedQueue);
                        }
                    }

                    // run another process
                    if (isEmpty(rrReadyQueue) != TRUE_CONDITION)
                    {
                        RRPreemption(rrReadyQueue, rrProcMsg, c);
                    }
                }
                // if the quanta is reached
                else if (message.quantum == quanta)
                {
                    runningProcess->pcb.state = STOPPED_STATE;
                    PrintCurrentState(runningProcess);
                    // send signal to stop the process from running
                    kill(runningProcess->processId, SIGSTOP);
                    enqueue(rrReadyQueue, runningProcess);

                    runningProcess = NULL;
                    // reset quanta
                    quanta = 0;

                    if (isEmpty(rrReadyQueue) != TRUE_CONDITION)
                    {
                        RRPreemption(rrReadyQueue, rrProcMsg, c);
                    }
                }
            }
            // check if scheduling is finished
            if (isEmpty(rrReadyQueue) == TRUE_CONDITION && runningProcess == NULL && NumOfTerminatedProcesses == Processno)
            {
                // generate the .perf file and clear the process-scheduler message queues
                Calculate();
                msgctl(msgq_id, IPC_RMID, NULL);
                msgctl(msgq_id2, IPC_RMID, NULL);
                shmdt(NULL);
                exit(1);
            }
            prev_clk = c;
        }
    }
}

//=================================================================================================================
//========================================== SHORTEST-REAMAINING-TIME =============================================
//=================================================================================================================

/// Shortest remaining algorithm, this algorithms is considering the remaining time at the first place
/// Initially the Algorithm initializes a priority ready queue, variable for storing the clk
/// then the algorithms starts to receive the processes at the arrival times of these processes
/// making sure to receive processes received at same arrival time
/// and stores them in the prioriy ready queue
/// starting the processing by checking that the ready queue is not empty and there is no running processes
/// then take a new process from the ready queue and check if the process is processed before or its first time
/// if it is its first time a new process is forked and its id is stored in the process
/// update its pcb variables and send the process to the process file
/// and run this process until finish its run time or another process with less remaining time is reached
/// and in case preemtion the data is saved before re inserting in the ready queue and retreived when resuming
/// and make same process for all processes
/// at the end check if the scheduling is finishing by checking if ready is empty
/// and number of scheduled processes equal to total number of processes
void STRNPreemption(struct Priority_Queue *SRTNReadyQueue, struct msgbuff2 SRTNProcMsg, int c)
{
    int pid;
    runningProcess = PriorityDequeue(SRTNReadyQueue);

    if (runningProcess->processId == NEW_PROCESS)
    {
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrivalTime - runningProcess->runTime + runningProcess->pcb.remainingTime;
        runningProcess->pcb.state = STARTED_STATE;
        runningProcess->pcb.waitingTime = c - runningProcess->arrivalTime;

        pid = fork();

        if (pid == CHILD_PROCESS)
        {
            // send el process to process.c
            execl("./process.out", "process.out", NULL);
        }

        // set el id gowa el process
        runningProcess->processId = pid;
        PrintCurrentState(runningProcess);
    }
    else
    {
        runningProcess->pcb.state = RESUMED_STATE;
        runningProcess->pcb.WaitingtimeSoFar = c - runningProcess->arrivalTime - runningProcess->runTime + runningProcess->pcb.remainingTime;
        PrintCurrentState(runningProcess);
        kill(runningProcess->processId, SIGCONT);
    }

    // send message queue lel process bel data

    SRTNProcMsg.process = *runningProcess;

    sen_val = msgsnd(msgq_id2, &SRTNProcMsg, sizeof(SRTNProcMsg) - sizeof(long), !IPC_NOWAIT);
    if (sen_val == ERROR)
    {
        perror("msgrcv error");
    }
}

void STRNFinish(struct Priority_Queue *SRTNReadyQueue, struct msgbuff2 SRTNProcMsg, int c)
{
    int wPid, status;

    // send message queue lel process bel data
    SRTNProcMsg.process = *runningProcess;

    sen_val = msgsnd(msgq_id2, &SRTNProcMsg, sizeof(SRTNProcMsg) - sizeof(long), !IPC_NOWAIT);
    if (sen_val == ERROR)
    {
        perror("msgrcv error");
    }

    if (runningProcess->pcb.remainingTime == 0)
    {
        wPid = wait(&status);
        runningProcess->pcb.TurnaroundTime = c - runningProcess->arrivalTime;
        runningProcess->pcb.WeightedTurnaroundTime = (double)runningProcess->pcb.TurnaroundTime / (double)runningProcess->runTime;
        TotalWeightedTRT += runningProcess->pcb.WeightedTurnaroundTime;
        TotalWeightedTRTs[NumOfTerminatedProcesses] = runningProcess->pcb.WeightedTurnaroundTime;
        NumOfTerminatedProcesses++;
        runningProcess->pcb.state = FINISHED_STATE;
        PrintCurrentState(runningProcess);
        TotalWaitingTimes += runningProcess->pcb.WaitingtimeSoFar;
        FreeFromBuddySystem(runningProcess->id);
        runningProcess = NULL;

        if (isEmpty(BlockedQueue) != TRUE_CONDITION)
        {
            struct Process *toBeAddedProcess = peek(BlockedQueue);
            if (AssignToBuddySystem(toBeAddedProcess))
            {
                PriorityEnqueue(SRTNReadyQueue, toBeAddedProcess, toBeAddedProcess->pcb.remainingTime);
                dequeue(BlockedQueue);
            }
        }

        //////// run top process in queue //////////
        if (PQisEmpty(SRTNReadyQueue) != TRUE_CONDITION)
        {
            STRNPreemption(SRTNReadyQueue, SRTNProcMsg, c);
        }
    }
}

void SRTN()
{
    struct Priority_Queue *SRTNReadyQueue = createPriorityQueue();
    int prev_clk = -1;
    key_id2 = ftok("keyfile", PROCESS_KEY);
    msgq_id2 = msgget(key_id2, 0666 | IPC_CREAT);
    struct msgbuff2 SRTNProcMsg;
    SRTNProcMsg.mtype = PROCESS_LOCK;
    if (msgq_id2 == ERROR)
    {
        perror("Error in create");
        exit(ERROR);
    }

    printf("//=====================================================================================================================//\n");
    printf("//=============================================== WELCOME TO STRN ALGORITHM ===========================================//\n");
    printf("//=====================================================================================================================//\n\n");

    while (TRUE_CONDITION)
    {
        int c = getClk();
        if (c != prev_clk)
        {
            printf("CLK IN SCH %d\n", c);
            // receive process
            usleep(sleep_seconds);
            int rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);
            while (rec_val2 != EMPTY_MESSAGE_QUEUE)
            {
                struct Process *newProcess = Create_Process(message.process.id, message.process.arrivalTime, message.process.runTime, message.process.priority, message.process.memsize);

                // try to allocate memory for arrived process
                // if allocated then add to the queue, else enqueue in BLK queue
                if (AssignToBuddySystem(newProcess))
                {
                    printf("I AM HERE ASIGNED TO PRIORITY QUEUE\n");
                    // store the processes in ready queue
                    PriorityEnqueue(SRTNReadyQueue, newProcess, newProcess->pcb.remainingTime);
                }
                else
                {
                    printf("MEMORY IS FULL ASIGNED TO BLOCKED QUEUE\n");
                    enqueue(BlockedQueue, newProcess);
                }

                rec_val2 = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_LOCK, IPC_NOWAIT);
            }

            // running process?
            if (PQisEmpty(SRTNReadyQueue) != TRUE_CONDITION && runningProcess == NULL)
            {
                STRNPreemption(SRTNReadyQueue, SRTNProcMsg, c);
            }
            else if (runningProcess != NULL)
            {
                TimeWhileRunning++;
                if (PQisEmpty(SRTNReadyQueue) != TRUE_CONDITION)
                {
                    runningProcess->pcb.remainingTime--;
                    if (runningProcess->pcb.remainingTime > PriorityPeek(SRTNReadyQueue)->pcb.remainingTime)
                    {
                        runningProcess->pcb.state = STOPPED_STATE;
                        PrintCurrentState(runningProcess);
                        kill(runningProcess->processId, SIGSTOP);
                        PriorityEnqueue(SRTNReadyQueue, runningProcess, runningProcess->pcb.remainingTime);
                        runningProcess = NULL;

                        ///////////  run the top process in the queue //////////////////////
                        STRNPreemption(SRTNReadyQueue, SRTNProcMsg, c);
                    }
                    else
                    {
                        // send message queue lel process bel data
                        STRNFinish(SRTNReadyQueue, SRTNProcMsg, c);
                    }
                }
                else
                {
                    runningProcess->pcb.remainingTime--;
                    STRNFinish(SRTNReadyQueue, SRTNProcMsg, c);
                }
            }
            if (PQisEmpty(SRTNReadyQueue) == 1 && runningProcess == NULL && NumOfTerminatedProcesses == Processno)
            {
                Calculate();
                msgctl(msgq_id, IPC_RMID, NULL);
                msgctl(msgq_id2, IPC_RMID, NULL);
                shmdt(NULL);
                exit(1);
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
    InitializePrintFile();
    InitializeMemoryFile();
    BlockedQueue = createQueue();
    key_id = ftok("keyfile", SCHEDULER_KEY);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    if (msgq_id == ERROR)
    {
        perror("Error in create");
        exit(ERROR);
    }

    // receive algorithm type
    rec_val = msgrcv(msgq_id, &message, sizeof(message) - sizeof(long), SCHEDULER_INFO_LOCK, !IPC_NOWAIT);
    if (rec_val == ERROR)
        perror("Error in receive");
    else
    {
        Processno = message.NumOfProcesses;
        TotalWeightedTRTs = (double *)malloc(Processno * sizeof(double));
        algo = message.algotype;
    }

    memory_tree = initialize_memory(max_memory_size, 0, max_memory_size - 1); // initialize tree for buddy system

    if (message.algotype == PROCESS_LOCK)
    {
        RR();
    }
    else if (message.algotype == SCHEDULER_INFO_LOCK)
    {
        HPF();
    }
    else if (message.algotype == SCHEDULER_LOCK)
    {
        SRTN();
    }

    shmdt(NULL);
    destroyClk(true);
}