/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   monitor.c                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:24:14 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:33:41 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	wake_all_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		pthread_mutex_lock(&sim->dongles[i].mutex);
		pthread_cond_broadcast(&sim->coders[i].cond);
		pthread_mutex_unlock(&sim->dongles[i].mutex);
		i++;
	}
}

// lock order: log -> stop, then table alone (only to wake the waiters)
// never hold stop_mutex and table_mutex at the same time
void	exit_simulation(t_sim *sim, bool burned_out, int i)
{
	pthread_mutex_lock(&sim->log_mutex);
	pthread_mutex_lock(&sim->stop_mutex);
	if (!sim->stopped)
	{
		sim->stopped = true;
		if (burned_out)
			printf("%ld %d burned out\n",
				get_time_ms() - sim->start_ms, sim->coders[i].id);
	}
	pthread_mutex_unlock(&sim->stop_mutex);
	pthread_mutex_unlock(&sim->log_mutex);
	pthread_mutex_lock(&sim->table_mutex);
	pthread_cond_broadcast(&sim->table_cond);
	pthread_mutex_unlock(&sim->table_mutex);
	wake_all_coders(sim);
}

// reads compile_count and last_compile_ms under table_mutex and
// releases it before returning, so exit_simulation can be called after
// returns true when every coder compiled enough, else *dead is the index
// of a burned out coder (or -1)
static bool	check_coders(t_sim *sim, int *dead)
{
	long	now;
	int		i;
	int		total;

	pthread_mutex_lock(&sim->table_mutex);
	now = get_time_ms();
	total = 0;
	*dead = -1;
	i = 0;
	while (i < sim->params.num_coders)
	{
		total += sim->coders[i].compile_count;
		if (*dead == -1
			&& sim->coders[i].compile_count < sim->params.num_compiles_required
			&& now >= sim->coders[i].last_compile_ms
			+ sim->params.time_to_burnout)
			*dead = i;
		i++;
	}
	pthread_mutex_unlock(&sim->table_mutex);
	return (total >= sim->params.num_compiles_required
		* sim->params.num_coders);
}

void	monitor_coders(t_sim *sim)
{
	int	dead;

	if (check_coders(sim, &dead))
		exit_simulation(sim, false, 0);
	else if (dead != -1)
		exit_simulation(sim, true, dead);
}

void	*monitor_routine(void *arg)
{
	t_sim	*sim;

	sim = (t_sim *)arg;
	while (!is_stopped(sim))
	{
		monitor_coders(sim);
		usleep(1000);
	}
	return (NULL);
}