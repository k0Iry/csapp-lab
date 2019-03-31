#include "csapp.h"

void sigalarm_handler(int signum) {}

char *tfgets(char *str, int size, FILE *stream)
{
    Signal(SIGALRM, sigalarm_handler);
    alarm(5);
    if (fgets(str, size, stream) == NULL)
    {
        if (errno == EINTR)
        {
            return NULL;
        }
    }

    return str;
}

int main()
{
    char str[256] = {0};
    tfgets(str, 256, stdin);

    printf("got string: %s\n", str);
}