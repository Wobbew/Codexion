/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle.c                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:21 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/10/08 17:12:51 by wwiedijk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// look if we know how long to wait
// if we do, wait for that long whit pthread_cond_timedwait
void	table_wait(t_sim *sim, t_dongle *first, t_dongle *second, long now)
{
	struct timespec	ts;
	long			until;

	until = -1;
	if (first->available && first->available_at > now)
		until = first->available_at;
	if (second->available && second->available_at > now && (until == -1
			|| second->available_at < until))
		until = second->available_at;
	if (until == -1)
		pthread_cond_wait(&sim->table_cond, &sim->table_mutex);
	else
	{
		ms_to_ts(until, &ts);
		pthread_cond_timedwait(&sim->table_cond, &sim->table_mutex, &ts);
	}
}

// to avoid circular wait
// make that the last coder requests the dongles in opisid order
static void	select_dongles(t_coder *coder, t_dongle **first, t_dongle **second)
{
	if (coder->id == coder->sim->params.num_coders)
	{
		*first = coder->dongle_right;
		*second = coder->dongle_left;
	}
	else
	{
		*first = coder->dongle_left;
		*second = coder->dongle_right;
	}
}

// for one coder
static int	handle_same_dongle(t_coder *coder, t_dongle *dongle)
{
	t_sim	*sim;

	sim = coder->sim;
	pthread_mutex_lock(&sim->table_mutex);
	dongle->available = false;
	log_state(sim, coder->id, "has taken a dongle");
	while (!sim->stopped)
		pthread_cond_wait(&sim->table_cond, &sim->table_mutex);
	pthread_mutex_unlock(&sim->table_mutex);
	return (-1);
}

// checks if both dongles are ready for use
// and the coders at the head of their respective queues
bool	dongles_ready(t_dongle *first, t_dongle *second, t_coder *coder)
{
	return (dongle_ready(first, get_time_ms()) && dongle_ready(second,
			get_time_ms()) && is_at_head(first, coder) && is_at_head(second,
			coder));
}

int	take_dongle(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	t_sim		*sim;

	sim = coder->sim;
	select_dongles(coder, &first, &second);
	if (first == second)
		return (handle_same_dongle(coder, first));
	pthread_mutex_lock(&sim->table_mutex);
	if (wait_for_dongles(coder, first, second) == -1)
	{
		pthread_mutex_unlock(&sim->table_mutex);
		return (-1);
	}
	first->available = false;
	second->available = false;
	pthread_mutex_unlock(&sim->table_mutex);
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (0);
}
