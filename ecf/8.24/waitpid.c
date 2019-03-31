#include "csapp.h"

#define N 10

volatile sig_atomic_t pid;

void sigchld_handler(int s)
{
    int olderrno = errno;
    int status;
    char str[256];

    while ((pid = waitpid(-1, &status, 0)) > 0)
    {
        if (WIFSIGNALED(status))
        {
            snprintf(str, 256, "child %d terminated by signal %d", pid, WTERMSIG(status));
            psignal(WTERMSIG(status), str);
        }
    }

    errno = olderrno;
}

int main()
{
    int status, i;
    sigset_t mask, prev;
    Sigemptyset(&mask);
    Sigaddset(&mask, SIGCHLD);

    if (Signal(SIGCHLD, sigchld_handler) == SIG_ERR)
    {
        unix_error("signal error");
    }

    /* Parent creates N children */
    for (i = 0; i < N; i++)
    {
        Sigprocmask(SIG_BLOCK, &mask, &prev);       // block SIGCHLD to avoid uncertain schedule
        if (Fork() == 0)
        {
            const char *conststr = NULL;
            printf("%c", conststr[0]);          // children gonna crash here
        }

        pid = 0;    // wait for pid being set by handler
        while (!pid)
        {
            Sigsuspend(&prev);      // unblock SIGCHLD, this is uninterruptible
        }

        Sigprocmask(SIG_SETMASK, &prev, NULL);
    }
    exit(0);
}