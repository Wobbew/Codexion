/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:56 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:33:15 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Only joins the threads that were really created (sim->started coders
** and, if it exists, the monitor); joining an uninitialized pthread_t is
** undefined behavior.
*/
void	join_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->started)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	if (sim->monitor_started)
		pthread_join(sim->monitor, NULL);
}

void	cleanup(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		pthread_cond_destroy(&sim->coders[i].cond);
		pthread_mutex_destroy(&sim->dongles[i].mutex);
		i++;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->table_mutex);
	pthread_cond_destroy(&sim->table_cond);
	free(sim->waiter_pool);
	free(sim->coders);
	free(sim->dongles);
}

// creates every coder thread, then the monitor
// sim->started counts the coder threads that exist
static bool	start_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
			return (false);
		sim->started++;
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (false);
	sim->monitor_started = true;
	return (true);
}

int	main(int argc, char **argv)
{
	t_params	params;
	t_sim		sim;
	int			ret;

	if (params_args(argc, argv, &params) != true)
		return (1);
	sim.params = params;
	if (setup(&sim) != true)
		return (1);
	ret = 0;
	if (start_threads(&sim) != true)
	{
		exit_simulation(&sim, false, 0);
		ret = 1;
	}
	join_threads(&sim);
	cleanup(&sim);
	return (ret);
}