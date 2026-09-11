/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   setup.c                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:27:44 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/11 15:53:55 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
//make the simulation ready for execution
//mallocs all things
//and initializing mutexes and conditions
bool	setup_sim(t_sim *sim)
{
	sim->coders = malloc(sizeof(t_coder) * sim->params.num_coders);
	if (!sim->coders)
		return (false);
	sim->dongles = malloc(sizeof(t_dongle) * sim->params.num_coders);
	if (!sim->dongles)
	{
		free(sim->coders);
		return (false);
	}
	sim->start_ms = get_time_ms();
	pthread_mutex_init(&sim->log_mutex, NULL);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_mutex_init(&sim->table_mutex, NULL);
	pthread_cond_init(&sim->table_cond, NULL);
	sim->stopped = false;
	sim->monitor_started = false;
	return (true);
}

bool	setup_coders(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->coders[i].id = i + 1;
		sim->coders[i].compile_count = 0;
		sim->coders[i].last_compile_ms = sim->start_ms;
		sim->coders[i].dongle_right = &sim->dongles[i];
		sim->coders[i].dongle_left
			= &sim->dongles[(i + 1) % sim->params.num_coders];
		sim->coders[i].sim = sim;
		pthread_cond_init(&sim->coders[i].cond, NULL);
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (false);
		i++;
	}
	return (true);
}

bool	setup_dongles(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		sim->dongles[i].id = i + 1;
		sim->dongles[i].available = true;
		sim->dongles[i].available_at = 0;
		sim->dongles[i].waiters = malloc(sizeof(t_waiter) * 2);
		if (!sim->dongles[i].waiters)
			return (false);
		sim->dongles[i].waiter_count = 0;
		pthread_mutex_init(&sim->dongles[i].mutex, NULL);
		i++;
	}
	return (true);
}
//make everything ready for execution
bool	setup(t_sim *sim)
{
	
	if (setup_sim(sim) != true)
		return (false);
	if (setup_dongles(sim) != true)
		return (false);
	if (setup_coders(sim) != true)
		return (false);
	return (true);
}
