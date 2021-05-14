#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread(void *vargp);

int main()
{
    pthread_t tid;
    int ret = -1;
    if ((ret = pthread_create(&tid, NULL, thread, NULL)) == -1)
    {
        exit(EXIT_FAILURE);
    }

    // replace exit with pthread_exit
    // so that main thread will wait for peer threads terminated
    pthread_exit(NULL);
}

/* Thread routine */
void *thread(void *vargp)
{
    sleep(1);
    printf("Hello, world!\n");
    return NULL;
}