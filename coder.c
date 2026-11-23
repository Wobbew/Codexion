/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   coder.c                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:21:00 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:31:39 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

// never prints after the simulation stopped: the stop check and the
// printf happen under the same log_mutex (lock order: log -> stop)
void	log_state(t_sim *sim, int id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);
	if (!is_stopped(sim))
		printf("%ld %d %s\n", get_time_ms() - sim->start_ms, id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

// last_compile_ms is set in take_dongle and compile_count in
// release_dongle, both under table_mutex (the monitor reads them there)
static bool	do_compile(t_coder *coder)
{
	if (take_dongle(coder) == -1)
		return (false);
	log_state(coder->sim, coder->id, "is compiling");
	usleep(coder->sim->params.time_to_compile * 1000);
	release_dongle(coder);
	return (!is_stopped(coder->sim));
}

static bool	do_debug_refactor(t_coder *coder)
{
	log_state(coder->sim, coder->id, "is debugging");
	usleep(coder->sim->params.time_to_debug * 1000);
	if (is_stopped(coder->sim))
		return (false);
	log_state(coder->sim, coder->id, "is refactoring");
	usleep(coder->sim->params.time_to_refactor * 1000);
	return (true);
}

void	*coder_routine(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	if (coder->id % 2 == 0)
		usleep(coder->sim->params.time_to_compile * 1000 / 2);
	while (!is_stopped(coder->sim))
	{
		if (do_compile(coder) == false)
			break ;
		if (do_debug_refactor(coder) == false)
			break ;
		if (coder->compile_count >= coder->sim->params.num_compiles_required)
			break ;
	}
	return (NULL);
}