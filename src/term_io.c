#include <sys/types.h>

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <pty.h>
#include "term_io.h"
#include <lvgl.h>
#include <utmp.h>
#include <signal.h>


int term_openpty(term_pty_t * pty)
{
    int res = 0;


    // -------------- V2 ----------------

    res = openpty(&pty->master_fd, &pty->slave_fd, pty->slave_path, NULL, NULL);

    fcntl(pty->master_fd, F_SETFD, fcntl(pty->master_fd, F_GETFD) | FD_CLOEXEC);
    fcntl(pty->master_fd, F_SETFL, fcntl(pty->master_fd, F_GETFL) | O_NONBLOCK);

    pty->slave_pid = -1;

	return res;
}

int term_readptym(term_pty_t * pty)
{
    memset(pty->ptybuf,'\0', BUFFSIZE);
    pty->last_nread = read(pty->master_fd, pty->ptybuf, BUFFSIZE);
    //LV_LOG_USER("read bytes: %d", pty->last_nread);
	return pty->last_nread;
}

void term_notify_resize(term_pty_t * pty)
{
    if (pty->slave_pid != -1)
    {
        LV_LOG_WARN("Send SIGWINCH to %d", pty->slave_pid);
        kill(pty->slave_pid, SIGWINCH);
    }
}

void term_set_size(term_pty_t * pty, uint16_t x_max, uint16_t y_max)
{
    pty->ws.ws_row = y_max;
    pty->ws.ws_col = x_max;

    //ioctl(pty->master_fd, TIOCSWINSZ, &pty->ws.ws_row);
    ioctl(pty->slave_fd, TIOCSWINSZ, &pty->ws.ws_row);
    term_notify_resize(pty);
        
}

int term_process(term_pty_t * pty, char **argv, char **env)
{
    int res;
    pid_t pid;

    pid = fork();

    if (pid == -1) {
            /* Error.  */
            LV_LOG_WARN("forkpty");
            return -1;
    }

    if (pid == 0) {
            /* Child. (shell, etc) */
            close(pty->master_fd);
            res = login_tty(pty->slave_fd);
            if (res)
            {
                LV_LOG_WARN("login_tty err: %d", res);
                //return res;
                exit(res);
            }

            /* replace child process */
            res = execve(argv[0], argv, env);
            /* only reaches here in case of error */
            if (res)
            {
                LV_LOG_WARN("execve err: %d", res);
                // return res;
                exit(res);
            }
            
    }

    /* Parent. (terminal) */
    pty->slave_pid = pid;
}
