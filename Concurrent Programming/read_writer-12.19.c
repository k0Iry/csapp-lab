#include "../tiny/csapp.h"
#include <stdbool.h>

static int readcnt;
static sem_t mutex;
static sem_t w;

void init()
{
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    readcnt = 0;
}

void *reader()
{
    while (true)
    {
        P(&mutex);
        readcnt++;
        if (readcnt == 1)
        {
            // first in
            P(&w);
        }
        // read threads always run below with write lock held, which means writer has to be blocked
        V(&mutex);

        P(&mutex);
        readcnt--;
        if (readcnt == 0)
        {
            V(&w);
        }
        V(&mutex);
    }

    return NULL;
}

void *writer()
{
    while (true)
    {
        if (readcnt == 1) continue;
        // contional wait until reader acquire the lock?
        P(&w);
        V(&w);
        // when writer is about to release the write lock, there is no reader proceeding, but definitely there is
        // one reader that is blocked acquiring write lock, if so, we let reader get it.
        // but if no reader is ready, we can continue write more
    }
    return NULL;
}

int main()
{
    int i = 0;
    pthread_t tid;
    init();
    Pthread_create(&tid, NULL, reader, NULL);
    Pthread_create(&tid, NULL, writer, NULL);

    Pthread_exit(NULL);
    return 0;
}