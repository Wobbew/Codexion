
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
 
int	compile_count(t_sim *sim)
{
	int	total;
	int	i;
 
	total = 0;
	i = 0;
	while (i < sim->params.num_coders)
	{
		total += sim->coders[i].compile_count;
		i++;
	}
	return (total);
}
 
void	exit_simulation(t_sim *sim, bool burned_out, int i)
{
	pthread_mutex_lock(&sim->stop_mutex);
	if (!sim->stopped)
	{
		sim->stopped = true;
		if (burned_out)
			log_state(sim, sim->coders[i].id, "burned out");
	}
	pthread_mutex_unlock(&sim->stop_mutex);
	wake_all_coders(sim);
}
 
/*
** A coder burns out only if it has NOT yet reached its required
** compile count and has missed its deadline. Success (every coder
** reached the target) is checked separately so the simulation always
** has a way to stop even when nobody burns out.
*/
void	monitor_coders(t_sim *sim)
{
	long	now;
	int		i;
	int		target;
 
	target = sim->params.num_compiles_required * sim->params.num_coders;
	if (compile_count(sim) == target)
		return (exit_simulation(sim, false, 0));
	now = get_time_ms();
	i = 0;
	while (i < sim->params.num_coders)
	{
		if (sim->coders[i].compile_count < sim->params.num_compiles_required
			&& now >= sim->coders[i].last_compile_ms
			+ sim->params.time_to_burnout)
			return (exit_simulation(sim, true, i));
		i++;
	}
}
 
void	*monitor_routine(void *arg)
{
	t_sim	*sim;
 
	sim = (t_sim *)arg;
	while (true)
	{
		pthread_mutex_lock(&sim->stop_mutex);
		if (sim->stopped)
		{
			pthread_mutex_unlock(&sim->stop_mutex);
			return (NULL);
		}
		pthread_mutex_unlock(&sim->stop_mutex);
		monitor_coders(sim);
		usleep(1000);
	}
	return (NULL);
}
