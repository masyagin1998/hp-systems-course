// gcc -std=c11 -O2 -Wall -Wextra -o thread_pool thread_pool.c -lpthread
// ./thread_pool

// поменять число threads в пуле, перекомпилировать и перезапустить программу

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define MAX_QUEUE_SIZE 64

typedef struct {
    void (*function)(void*);
    void *arg;
} Task;

typedef struct {
    Task queue[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;

    pthread_mutex_t lock;
    pthread_cond_t has_tasks;
    pthread_t *threads;
    int num_threads;

    int stop;
} ThreadPool;

void* worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool*) arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);

        while (pool->count == 0 && !pool->stop) {
            pthread_cond_wait(&pool->has_tasks, &pool->lock);
        }

        if (pool->stop && pool->count == 0) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }

        Task task = pool->queue[pool->front];
        pool->front = (pool->front + 1) % MAX_QUEUE_SIZE;
        pool->count--;

        pthread_mutex_unlock(&pool->lock);

        task.function(task.arg);
    }

    return NULL;
}

void thread_pool_init(ThreadPool *pool, int num_threads) {
    pool->front = 0;
    pool->rear = 0;
    pool->count = 0;
    pool->stop = 0;
    pool->num_threads = num_threads;

    pthread_mutex_init(&pool->lock, NULL);
    pthread_cond_init(&pool->has_tasks, NULL);

    pool->threads = malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    }
}

void thread_pool_add_task(ThreadPool *pool, void (*function)(void*), void *arg) {
    pthread_mutex_lock(&pool->lock);

    if (pool->count == MAX_QUEUE_SIZE) {
        fprintf(stderr, "Очередь задач переполнена!\n");
        pthread_mutex_unlock(&pool->lock);
        return;
    }

    pool->queue[pool->rear].function = function;
    pool->queue[pool->rear].arg = arg;
    pool->rear = (pool->rear + 1) % MAX_QUEUE_SIZE;
    pool->count++;

    pthread_cond_signal(&pool->has_tasks);
    pthread_mutex_unlock(&pool->lock);
}

void thread_pool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->lock);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->has_tasks);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->has_tasks);
}

typedef struct {
    int id;
    int delay_sec;
} TaskArg;

void print_task(void *arg) {
    TaskArg *t = (TaskArg*)arg;
    printf("Task %d started in thread %lu, sleeping for %d sec\n",
           t->id, pthread_self(), t->delay_sec);
    sleep(t->delay_sec);
    printf("Task %d finished\n", t->id);
    free(t);
}

int main() {
    ThreadPool pool;
    int num_threads = 1;
    thread_pool_init(&pool, num_threads);

    for (int i = 0; i < 5; i++) {
        TaskArg *arg = malloc(sizeof(TaskArg));
        arg->id = i;
        arg->delay_sec = 5 - (i + 1);

        thread_pool_add_task(&pool, print_task, arg);
    }

    thread_pool_destroy(&pool);
    return 0;
}
