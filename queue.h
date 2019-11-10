#include <pthread.h>
#include <stdlib.h>
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

template <class T>
class Queue{
    private:
        queue_node *head, *tail;
        ssize_t size, max_size;
        pthread_cond_t cv;
        pthread_mutex_t m;
    public:
        Queue(ssize_t max_size);
        ~Queue();
        void push(T *data);
        T* pop();
}