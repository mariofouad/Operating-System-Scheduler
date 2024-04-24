#include <stdio.h>
#include <stdlib.h>

// Define a structure for a node in the linked list
struct Priority_node {
    struct Process* data;
    int priority;
    struct Priority_node *next;
};

// Define a structure for the queue
struct Priority_Queue {
    struct Priority_node *front, *rear;
};

// Function to create a new node
struct Priority_node* newPriorityNode(struct Process* data, int priority) {
    struct Priority_node* node = (struct Priority_node*)malloc(sizeof(struct Priority_node));
    node->data = data;
    node->priority = priority;
    node->next = NULL;
    return node;
}

// Function to create a new empty queue
struct Priority_Queue* createPriorityQueue() {
    struct Priority_Queue* queue = (struct Priority_Queue*)malloc(sizeof(struct Priority_Queue));
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to enqueue an element into the queue based on priority
void PriorityEnqueue(struct Priority_Queue* queue, struct Process* data, int priority) {
    struct Priority_node* newnode = newPriorityNode(data, priority);
    if (queue->rear == NULL || priority > queue->rear->priority) {
        newnode->next = queue->rear;
        queue->rear = newnode;
    } else {
        struct Priority_node* current = queue->rear;
        while (current->next != NULL && priority <= current->next->priority) {
            current = current->next;
        }
        newnode->next = current->next;
        current->next = newnode;
    }
    if (queue->front == NULL) {
        queue->front = newnode;
    }
}

struct Process* PriorityPeek(struct Priority_Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }
    struct Process* data = queue->front->data;
    return data;
}

// Function to dequeue an element from the queue
struct Process* PriorityDequeue(struct Priority_Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }
    struct Process* data = queue->front->data;
    struct Priority_node* temp = queue->front;
    queue->front = queue->front->next;
    free(temp);
    return data;
}

// Function to check if the queue is empty
int PQisEmpty(struct Priority_Queue* queue) {
    return (queue->front == NULL);
}
