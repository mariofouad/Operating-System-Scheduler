
#include <assert.h>
#include "headers.h"

void clearResources(int);
// messge struct for sending the data to the scheduler
struct msgbuff
{
    long mtype;
    int algotype;
    int quantum;
    struct Process process;
};

int main(int argc, char *argv[])
{
    // initialize the processes queue
    struct Queue *ProcessQueue = createQueue();

    int algo;
    int quantum = -1;
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    // char* filepath;
    // printf("Enter Input file name\n");
    // scanf("%s",filepath);

    //============= READ FILE ===================//
    FILE *inputfile = fopen("processes.txt", "r");
    if (inputfile == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    char buffer[1024]; // Buffer to read lines
    int Processno = 0;
    int id, at, rt, pr;
    // Read file and create processes and enqueue in ProcessQueue
    while (fgets(buffer, sizeof(buffer), inputfile) != NULL)
    {
        // Check if the first character of the line is #
        if (buffer[0] != '#')
        {
            Processno++;
            sscanf(buffer, "%d    %d  %d  %d", &id, &at, &rt, &pr);
            struct Process *newp = Create_Process(id, at, rt, pr);
            enqueue(ProcessQueue, newp);
        }
    }
    //===========================================================//

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Choose Scheluding Algorithm: 1 HPF, 2 SRTN, 3 RR\n");
    scanf("%d", &algo);
    while (algo < 1 || algo > 3) // validation of input
    {
        printf("Error, Choose a valid Scheluding Algorithm: 1 HPF, 2 SRTN, 3 RR\n");
        scanf("%d", &algo);
    }
    if (algo == 3) // input quantum for RR
    {
        printf("Enter RR Quantum:\n");
        scanf("%d", &quantum);
        while (quantum < 1)
        {
            printf("Error, Enter a valid RR Quantum:\n");
            scanf("%d", &quantum);
        }
    }
    // 3. Initiate and create the scheduler and clock processes.
    int pid = fork();
    if (pid == 0)
    {
        system("./clk.out");
    }
    else
    {
        int pid2 = fork();
        if (pid2 == 0)
        {
            system("./scheduler.out");
        }
        initClk();
    }

    //========================== SENDING DATA =======================//
    // Create message Queue
    key_t key_id, key_id2;
    int msgq_id, msgq_id2, send_val;

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

    // sending the algorithm chosen type
    struct msgbuff message;

    message.mtype = 1;
    message.algotype = algo;
    message.quantum = quantum;

    send_val = msgsnd(msgq_id, &message, sizeof(message.algotype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send");

    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    int prev_c = -1;
    while (1)
    {
        int c = getClk();
        if (c != prev_c)
        {
            printf("Clock = %d\n", c);
            prev_c = c;

            // check is process arrival time

            while (isEmpty(ProcessQueue) != 1)
            {
                struct Process *p = peek(ProcessQueue);
                if (c >= p->arrivalTime)
                {
                    // sending processes
                    message.process = *p;
                    message.mtype = 2;
                    // if(message2.process == NULL) printf("null while sending\n");
                    printf("%d while sendingg\n", message.process.id);
                    // printf("%ld while sending\n", message2.mtype);
                    printf("RUNNING TIME!!!!!%d\n", message.process.runTime);
                    send_val = msgsnd(msgq_id, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    if (send_val == -1)
                        perror("Error in send");
                    p = dequeue(ProcessQueue);
                    // p = peek(ProcessQueue);
                }
                else
                {
                    break;
                }
            }
        }
        // sleep(1);
    }
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}
