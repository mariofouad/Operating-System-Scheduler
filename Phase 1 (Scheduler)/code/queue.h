#include <stdio.h>
#include <stdlib.h>

// Define a structure for a node in the linked list
struct Node {
    struct Process* data;
    struct Node *next;
};

// Define a structure for the queue
struct Queue {
    struct Node *front, *rear;
};

// Function to create a new node
struct Node* newNode(struct Process* data) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    node->data = data;
    node->next = NULL;
    return node;
}

// Function to create a new empty queue
struct Queue* createQueue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->front = queue->rear = NULL;
    return queue;
}

// Function to enqueue an element into the queue
void enqueue(struct Queue* queue,struct Process* data) {
    struct Node* newnode = newNode(data);
    if (queue->rear == NULL) {
        queue->front = queue->rear = newnode;
        return;
    }
    queue->rear->next = newnode;
    queue->rear = newnode;
}

struct Process* peek(struct Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }
    struct Process* data = queue->front->data;
    return data;
}

// Function to dequeue an element from the queue
struct Process* dequeue(struct Queue* queue) {
    if (queue->front == NULL) {
        printf("Queue is empty\n");
        return NULL;
    }
    struct Process* data = queue->front->data;
    struct Node* temp = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    return data;
}

// Function to check if the queue is empty
int isEmpty(struct Queue* queue) {
    return (queue->front == NULL);
}

