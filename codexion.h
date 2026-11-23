/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   codexion.h                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:22:55 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:31:21 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

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
	int				num_coders;
	long			time_to_burnout;
	long			time_to_compile;
	long			time_to_debug;
	long			time_to_refactor;
	int				num_compiles_required;
	long			dongle_cooldown;
	int				scheduler;
}	t_params;

typedef struct s_waiter
{
	pthread_cond_t	*cond;
	long			key;
	int				coder_id;
}	t_waiter;

typedef struct s_dongle
{
	int				id;
	pthread_mutex_t	mutex;
	bool			available;
	long			available_at;
	t_waiter		*waiters;
	int				waiter_count;
}	t_dongle;

typedef struct s_sim	t_sim;

typedef struct s_coder
{
	int				id;
	int				compile_count;
	long			last_compile_ms;
	pthread_t		thread;
	pthread_cond_t	cond;
	t_dongle		*dongle_left;
	t_dongle		*dongle_right;
	t_sim			*sim;
}	t_coder;

/*
** Locking rules (lock order: table -> log -> stop, never the reverse):
** - table_mutex protects dongle state, the waiting heaps, next_ticket,
**   and every coder's last_compile_ms and compile_count.
** - log_mutex serializes output.
** - stop_mutex protects `stopped`.
** monitor_started and started let join_threads know which threads were
** really created, since joining an unstarted pthread_t is undefined.
*/
struct s_sim
{
	t_params		params;
	t_coder			*coders;
	t_dongle		*dongles;
	t_waiter		*waiter_pool;
	long			start_ms;
	long			next_ticket;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	pthread_mutex_t	table_mutex;
	pthread_cond_t	table_cond;
	bool			stopped;
	pthread_t		monitor;
	bool			monitor_started;
	int				started;
};

/* utils */
long	get_time_ms(void);
void	log_state(t_sim *sim, int coder_id, const char *msg);
void	ms_to_ts(long ms, struct timespec *ts);

/* parsing */
int		ft_isnumeric(char *s);
bool	input_error(void);
bool	input_error_int(void);
bool	input_error_schedul(void);
bool	test_args(t_params *params);
bool	params_args(int argc, char **argv, t_params *params);

/* init */
bool	setup(t_sim *sim);
bool	setup_sim(t_sim *sim);
void	setup_dongles(t_sim *sim);
void	setup_coders(t_sim *sim);
void	cleanup(t_sim *sim);

/* heap (one small min-heap of waiters per dongle) */
bool	waiter_less(t_waiter *a, t_waiter *b);
void	waiter_swap(t_waiter *a, t_waiter *b);
void	sift_up(t_dongle *d, int i);
void	sift_down(t_dongle *d, int i);
void	heap_push(t_dongle *d, t_waiter w);
void	heap_remove(t_dongle *d, int i);

/* dongles */
bool	dongles_ready(t_dongle *first, t_dongle *second, t_coder *coder);
int		take_dongle(t_coder *coder);
int		release_dongle(t_coder *coder);
int		wait_for_dongles(t_coder *coder, t_dongle *first, t_dongle *second);
void	table_wait(t_sim *sim, t_dongle *first, t_dongle *second, long now);
bool	dongle_ready(t_dongle *d, long now);
bool	is_at_head(t_dongle *d, t_coder *coder);
void	register_interest(t_coder *coder, t_dongle *first, t_dongle *second);
void	unregister_interest(t_dongle *dongle, int coder_id);

/* threads */
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
void	join_threads(t_sim *sim);

/* monitor */
void	wake_all_coders(t_sim *sim);
void	exit_simulation(t_sim *sim, bool burned_out, int i);
void	monitor_coders(t_sim *sim);
bool	is_stopped(t_sim *sim);

#endif