/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wwiedijk <wwiedijk@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:22:55 by wwiedijk          #+#    #+#             */
/*   Updated: 2026/08/31 13:22:58 by wwiedijk         ###   ########.fr       */
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
	bool			active;
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
** monitor_started lets join_threads know whether `monitor` was ever
** pthread_create'd, since joining an unstarted pthread_t is
** undefined behavior.
*/
struct s_sim
{
	t_params		params;
	t_coder			*coders;
	t_dongle		*dongles;
	long			start_ms;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	stop_mutex;
	bool			stopped;
	pthread_t		monitor;
	bool			monitor_started;
};

/* utils */
long	get_time_ms(void);
void	log_state(t_sim *sim, int coder_id, const char *msg);

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
bool	setup_dongles(t_sim *sim);
bool	setup_coders(t_sim *sim);
void	cleanup(t_sim *sim);

/* dongles */
void	wait_for_dongle(t_coder *coder, t_dongle *dongle);
void	pops_queue(t_dongle *dongle);
int		sort_queue(t_dongle *dongle, t_waiter waiter);
int		get_dongle(t_coder *coder, t_dongle *dongle);
int		take_dongle(t_coder *coder);
int		release_dongle(t_coder *coder);

/* threads */
void	*coder_routine(void *arg);
void	*monitor_routine(void *arg);
void	join_threads(t_sim *sim);

/* monitor */
void	wake_all_coders(t_sim *sim);
int		compile_count(t_sim *sim);
void	exit_simulation(t_sim *sim, bool burned_out, int i);
void	monitor_coders(t_sim *sim);

#endif