/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   setup.c                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:27:44 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:34:04 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// turns milliseconds to timespec
// i need timespec for pthread_cond_timedwait
void	ms_to_ts(long ms, struct timespec *ts)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000L;
}

// the only step that can fail (malloc)
// all waiting heaps share one pool: 2 slots per dongle
bool	setup_sim(t_sim *sim)
{
	int	n;

	n = sim->params.num_coders;
	sim->coders = malloc(sizeof(t_coder) * n);
	sim->dongles = malloc(sizeof(t_dongle) * n);
	sim->waiter_pool = malloc(sizeof(t_waiter) * 2 * n);
	if (!sim->coders || !sim->dongles || !sim->waiter_pool)
	{
		free(sim->coders);
		free(sim->dongles);
		free(sim->waiter_pool);
		return (false);
	}
	sim->start_ms = get_time_ms();
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->table_mutex, NULL);
	pthread_cond_init(&sim->table_cond, NULL);
	sim->stopped = false;
	sim->monitor_started = false;
	sim->next_ticket = 0;
	sim->started = 0;
	return (true);
}

// threads are created later, in main.c, once everything is initialised
void	setup_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_ms = sim->start_ms;
		sim->coders[i].dongle_right = &sim->dongles[i];
		sim->coders[i].dongle_left = &sim->dongles[(i + 1)
			% sim->params.num_coders];
		sim->coders[i].sim = sim;
		pthread_cond_init(&sim->coders[i].cond, NULL);
		i++;
	}
}

void	setup_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available = true;
		sim->dongles[i].available_at = 0;
		sim->dongles[i].waiters = &sim->waiter_pool[2 * i];
		sim->dongles[i].waiter_count = 0;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		i++;
	}
}

// make everything ready for execution
bool	setup(t_sim *sim)
{
	if (setup_sim(sim) != true)
		return (false);
	setup_dongles(sim);
	setup_coders(sim);
	return (true);
}