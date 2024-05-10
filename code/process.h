
#include <assert.h>

struct PCB
{
    int state; // 0:started, 1:resumed, 2:stopped, 3:finished
    int remainingTime;
    int waitingTime;
    int TurnaroundTime;
    double WeightedTurnaroundTime;
    int WaitingtimeSoFar;
};

struct Process
{
    int id;
    int processId;
    int arrivalTime;
    int runTime;
    int priority;
    struct PCB pcb;
    int memsize;

};

//Process constructor
struct Process *Create_Process(int id,int at, int rt, int pr, int ms)
{
    struct Process *p = malloc(sizeof(struct Process));
    assert(p != NULL);

    p->processId = -1;
    p->id = id;
    p->arrivalTime = at;
    p->runTime = rt;
    p->priority = pr;
    p->pcb.state=0;
    p->pcb.remainingTime= rt;
    p->pcb.waitingTime=0;
    p->memsize = ms;

    return p;
}

//Process destructor
void Destroy_Process(struct Process* p)
{
    assert(p != NULL);

    free(p);
}