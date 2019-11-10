#include <queue.h>

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

template <class T>
void Queue::push(T *data) {
    /* Your code here */
    pthread_mutex_lock(&m);
    if(max_size > 0 && size == max_size){
        while(size == max_size){
            pthread_cond_wait(&cv, &m);
        }
    }
    queue_node *node = malloc(sizeof(queue_node));
    node->data = data;
    if(size > 0){
        tail->next = node;
        tail = node;
    } else {
        head = node;
        tail = node;
    }
    size++;      
    pthread_cond_signal(cv);
    pthread_mutex_unlock(m);
}

template <class T>
T* Queue::pull() {
    pthread_mutex_lock(&m);
    T *ret = NULL;
    while(size == 0){
        pthread_cond_wait(&cv, &m);
    }
    ret = head->data;
    queue_node * lastHead = head;
    if(size > 1) head = head ->next;
    free(lastHead);
    size--;  
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&m);
    return ret;
}
