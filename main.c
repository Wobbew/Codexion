/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:56 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/04 17:46:32 by wwiedijk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

/*
** Only joins the monitor thread if one was actually started; joining
** an uninitialized pthread_t is undefined behavior.
*/
void	join_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->params.num_coders)
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
		free(sim->dongles[i].waiters);
		i++;
	}
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_mutex_destroy(&sim->table_mutex);
	pthread_cond_destroy(&sim->table_cond);
	free(sim->coders);
	free(sim->dongles);
}

int	main(int argc, char **argv)
{
	t_params	params;
	t_sim		sim;

	if (params_args(argc, argv, &params) != true)
		return (1);
	sim.params = params;
	if (setup(&sim) != true)
		return (1);
	if (pthread_create(&sim.monitor, NULL, monitor_routine, &sim) != 0)
		return (1);
	sim.monitor_started = true;
	join_threads(&sim);
	cleanup(&sim);
	return (0);
}
