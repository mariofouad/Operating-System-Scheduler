
#include <assert.h>
#include "headers.h"

void clearResources(int);

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

    struct Queue* ProcessQueue = createQueue();

    int algo;
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    //char* filepath;
    //printf("Enter Input file name\n");
    //scanf("%s",filepath);

    FILE *inputfile = fopen("processes.txt", "r");
    if (inputfile == NULL) {
        perror("Error opening file");
        return 1;
    }
    
    char buffer[1024]; // Buffer to read lines
    int Processno=0;
    int id,at,rt,pr;

    //Read file and create processes and enqueue in ProcessQueue
    while (fgets(buffer, sizeof(buffer), inputfile) != NULL) {
        // Check if the first character of the line is #
        if (buffer[0] != '#') {
            Processno++;
            sscanf(buffer,"%d    %d  %d  %d",&id,&at,&rt,&pr);
            struct Process* newp = Create_Process(id,at,rt,pr);
            enqueue(ProcessQueue,newp);
        }
    }

    printf("There are %d process\n",Processno);

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Choose Scheluding Algorithm: 1 HPF, 2 SRTN, 3 RR\n");
    scanf("%d",&algo);
    while (algo<1 || algo>3) //validation of input
    {
        printf("Error, Choose a valid Scheluding Algorithm: 1 HPF, 2 SRTN, 3 RR\n");
        scanf("%d",&algo);
    }
    if (algo == 3) //input quantum for RR
    {
        int quantum;
        printf("Enter RR Quantum:\n");
        scanf("%d",&quantum);
        while (quantum<1)
        {
            printf("Error, Enter a valid RR Quantum:\n");
            scanf("%d",&quantum);
        }
    }
    // 3. Initiate and create the scheduler and clock processes.
    int pid=fork();
    if (pid==0)
    {
        system("./clk.out");
    }
    else
    {
        int pid2=fork();
        if (pid2==0)
        {
            system("./scheduler.out");
        }
        initClk();

    
    }

    //Create message Queue
    key_t key_id,key_id2;
    int msgq_id,msgq_id2, send_val;

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

    message.mtype = 1; /* arbitrary value */
    message.algotype=algo;

    send_val = msgsnd(msgq_id, &message, sizeof(message.algotype), !IPC_NOWAIT);
    if (send_val == -1)
        perror("Errror in send");


    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    while (1)
    {
        int c=getClk();
        printf("Clock = %d\n",c);
        
        //check is process arrival time
        struct Process*p = peek(ProcessQueue);
        if (c >= p->arrivalTime)
        {
            struct msgbuff2 message2;
            message2.process=p;
            message2.mtype=2;
            send_val = msgsnd(msgq_id2, &message2, sizeof(struct Process*), !IPC_NOWAIT);
            if (send_val == -1)
                perror("Errror in send");
            p = dequeue(ProcessQueue);
            p = peek(ProcessQueue);
        }

        sleep(1);

    }
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
