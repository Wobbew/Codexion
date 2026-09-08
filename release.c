/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   release.c                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:27:38 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/08 22:56:54 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	release_dongle(t_coder *coder)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	pthread_mutex_lock(&sim->table_mutex);
	now = get_time_ms();
	coder->dongle_left->available = true;
	coder->dongle_left->available_at = now + sim->params.dongle_cooldown;
	coder->dongle_right->available = true;
	coder->dongle_right->available_at = now + sim->params.dongle_cooldown;
	pthread_cond_broadcast(&sim->table_cond);
	pthread_mutex_unlock(&sim->table_mutex);
	return (0);
}

bool	is_stopped(t_sim *sim)
{
	bool	s;

	pthread_mutex_lock(&sim->stop_mutex);
	s = sim->stopped;
	pthread_mutex_unlock(&sim->stop_mutex);
	return (s);
}