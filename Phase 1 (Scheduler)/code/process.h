
#include <assert.h>


struct PCB
{
    int state; // 0:running 1:ready 2:blocked
    //int executionTime;
    int remainingTime;
    int waitingTime;
};

struct Process
{
    int id;
    int processId;
    int arrivalTime;
    int runTime;
    int priority;
    struct PCB pcb;

};



//Process constructor
struct Process *Create_Process(int id,int at, int rt, int pr)
{
    struct Process *p = malloc(sizeof(struct Process));
    assert(p != NULL);

    p->processId = -1;
    p->id = id;
    p->arrivalTime = at;
    p->runTime = rt;
    p->priority = pr;
    // struct PCB controlblock;
    // p->pcb=controlblock;
    p->pcb.state=1;
    p->pcb.remainingTime= rt;
    printf("REMAINING TIME :%d\n",p->pcb.remainingTime);
    p->pcb.waitingTime=0;

    //if(p->pcb.remainingTime != 0) printf("process %d created, I am happy\n", p->id); check that rem time is not 0, it worked!
    return p;
}

//Process destructor
void Destroy_Process(struct Process* p)
{
    assert(p != NULL);

    free(p);
}