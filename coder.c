/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   coder.c                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:21:00 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/08 23:00:47 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

long	get_time_ms(void)
{
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	return ((long)tv.tv_sec * 1000L + (long)tv.tv_usec / 1000L);
}

void	log_state(t_sim *sim, int id, const char *msg)
{
	pthread_mutex_lock(&sim->log_mutex);
	printf("%ld %d %s\n", get_time_ms() - sim->start_ms, id, msg);
	pthread_mutex_unlock(&sim->log_mutex);
}

static bool	do_compile(t_coder *coder)
{
	if (take_dongle(coder) == -1)
		return (false);
	coder->last_compile_ms = get_time_ms();
	log_state(coder->sim, coder->id, "is compiling");
	usleep(coder->sim->params.time_to_compile * 1000);
	coder->compile_count++;
	release_dongle(coder);
	return (!coder->sim->stopped);
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
	while (coder->sim->stopped == false)
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
