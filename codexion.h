// codexion.h
#ifndef CODEXION_H
# define CODEXION_H

# include <stdbool.h>
# include <stdio.h>
# include <pthread.h>
# include <stdlib.h>
# include <string.h>
# include <sys/time.h>
# include <unistd.h>

typedef struct s_params
{
    int     num_coders;
    long    time_to_burnout;
    long    time_to_compile;
    long    time_to_debug;
    long    time_to_refactor;
    int     num_compiles_required;
    long    dongle_cooldown;
    char    *scheduler;
    pthread_t monitor;
}   t_params;

// Forward declaration so t_coder can reference t_dongle *
typedef struct s_dongle
{
    int             id;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            available;
    long            released_at_ms;
}   t_dongle;

typedef struct s_sim t_sim;

typedef struct s_coder
{
    int             id;
    int             compile_count;
    long            last_compile_ms;
    pthread_t       thread;
    t_dongle        *dongle_left;   // pointer, not copy
    t_dongle        *dongle_right;  // pointer, not copy
    t_sim           *sim;           // back-reference for params/stop flag
}   t_coder;

struct s_sim
{
    t_params        params;
    t_coder         *coders;
    t_dongle        *dongles;
    long            start_ms;       // gettimeofday-based, milliseconds
    pthread_mutex_t log_mutex;      // serialise all log output
    pthread_mutex_t stop_mutex;
    bool            stopped;
    pthread_t       monitor;
};

// utils
long    get_time_ms(void);
void    log_state(t_sim *sim, int coder_id, const char *msg);

// init
bool    params_args(int argc, char **argv, t_params *params);
bool    setup(t_sim *sim);
bool    setup_sim(t_sim *sim);
bool    setup_dongles(t_sim *sim);
bool    setup_coders(t_sim *sim);
void    cleanup(t_sim *sim);

// threads
void    *coder_routine(void *arg);
void    *monitor_routine(void *arg);

#endif