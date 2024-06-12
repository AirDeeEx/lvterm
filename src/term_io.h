#ifndef LV_TERM_IO_H
#define LV_TERM_IO_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>

#define PTYLEN			(1 << 16) // 65536
#define	BUFFSIZE	1024


typedef struct {
    int master_fd;
    int slave_fd;
    char slave_path[20];
    pid_t slave_pid;

    struct winsize ws;

    char ptybuf[BUFFSIZE];
    ssize_t last_nread;
} term_pty_t;

int term_openpty(term_pty_t * pty);
int term_readptym(term_pty_t * pty);
void term_set_size(term_pty_t * pty, uint16_t x_max, uint16_t y_max);
pid_t term_process(term_pty_t * pty, char **argv, char **env);
void term_exec(term_pty_t * pty, char **args, char **env);
void term_notify_resize(term_pty_t * pty);

#endif