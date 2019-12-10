#pragma once
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

struct Task {
    int sock;
};

typedef struct queue_node {
    Task *data;
    struct queue_node *next;
} queue_node;

class Queue {
    private:
        queue_node *head, *tail;
        ssize_t size, max_size;
        pthread_cond_t cv;
        pthread_mutex_t m;
    public:
        Queue(ssize_t max_size);
        ~Queue();
        void push(Task *data);
        Task* pop();
};



Queue::Queue(ssize_t max_size) {
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&cv, NULL);
    head = NULL;
    tail = NULL;
    size = 0;
    max_size = max_size;
}

Queue::~Queue(){
    queue_node *item;
    while(size > 1){
        item = head;
        head = head->next;
        free(item);
        size--;
    }
    if(size == 1) free(head);
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&cv);
}

void Queue::push(Task *data) {
    /* Your code here */
    pthread_mutex_lock(&m);
    if(max_size > 0 && size == max_size){
        while(size == max_size){
            pthread_cond_wait(&cv, &m);
        }
    }
    queue_node *node = new queue_node{data};
    if(size > 0){
        tail->next = node;
        tail = node;
    } else {
        head = node;
        tail = node;
    }
    size++;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
}

Task* Queue::pop() {
    pthread_mutex_lock(&m);
    Task *ret = NULL;
    while(size == 0){
        pthread_cond_wait(&cv, &m);
    }
    ret = head->data;
    queue_node *lastHead = head;
    if(size > 1) head = head->next;
    free(lastHead);
    size--;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    return ret;
}
