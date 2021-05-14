#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *thread(void *vargp);

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        printf("./hello <number of threads>\n");
        exit(EXIT_FAILURE);
    }

    int ret = -1;

    int number_threads = atoi(argv[1]);
    pthread_t tid;
    for (int i = 0; i < number_threads; i++)
    {
        if ((ret = pthread_create(&tid, NULL, thread, NULL)) == -1)
        {
            perror("pthread created failed");
            exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);

    return 0;
}

void *thread(void *vargp)
{
    printf("Hello, world!\n");
    return NULL;
}