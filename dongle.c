/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle.c                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:21 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/29 13:44:40 by wwiedijk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
//find the key for a waiter(how inportant it is)
static long	waiter_key(t_coder *coder)
{
	if (coder->sim->params.scheduler == 0)
		return (coder->last_compile_ms);
	return (coder->last_compile_ms + coder->sim->params.time_to_burnout);
}

//looks if a dongle is ready to be used
static bool	dongle_ready(t_dongle *d, long now)
{
	return (d->available && now >= d->available_at);
}

//checks if a coder is at the head of a dongle's waiting queue
static bool	is_at_head(t_dongle *d, t_coder *coder)
{
	return (d->waiter_count > 0 && d->waiters[0].coder_id == coder->id);
}

//tells the simulator about a coder's interest in the dongles
static void	register_interest(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_waiter	w;

	w.cond = &coder->cond;
	w.key = waiter_key(coder);
	w.coder_id = coder->id;
	sort_queue(first, w);
	if (second != first)
		sort_queue(second, w);
}

// removes a coder's interest in a dongle
// (removes the coder from the dongle's waiting queue)
static void	unregister_interest(t_dongle *dongle, int coder_id)
{
	int	i;

	i = 0;
	while (i < dongle->waiter_count && dongle->waiters[i].coder_id != coder_id)
		i++;
	while (i < dongle->waiter_count - 1)
	{
		dongle->waiters[i] = dongle->waiters[i + 1];
		i++;
	}
	if (i < dongle->waiter_count)
		dongle->waiter_count--;
}
//turns milliseconds to timespec
//i need timespec for pthread_cond_timedwait
static void	ms_to_ts(long ms, struct timespec *ts)
{
	ts->tv_sec = ms / 1000;
	ts->tv_nsec = (ms % 1000) * 1000000L;
}
//look if we know how long to wait
// if we do, wait for that long whit pthread_cond_timedwait
static void	table_wait(t_sim *sim, t_dongle *first, t_dongle *second, long now)
{
	struct timespec	ts;
	long			until;

	until = -1;
	if (first->available && first->available_at > now)
		until = first->available_at;
	if (second->available && second->available_at > now
		&& (until == -1 || second->available_at < until))
		until = second->available_at;
	if (until == -1)
		pthread_cond_wait(&sim->table_cond, &sim->table_mutex);
	else
	{
		ms_to_ts(until, &ts);
		pthread_cond_timedwait(&sim->table_cond, &sim->table_mutex, &ts);
	}
}
//to avoid circular wait
//make that the last coder requests the dongles in opisid order
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
//checks if both dongles are ready for use and the coders at the head of their respective queues
static bool	dongles_ready(t_dongle *first, t_dongle *second, t_coder *coder)
{
	return (
		dongle_ready(first, get_time_ms())
		&& dongle_ready(second, get_time_ms())
		&& is_at_head(first, coder) && is_at_head(second, coder));
}

static int	wait_for_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_sim	*sim;
	long	now;

	sim = coder->sim;
	register_interest(coder, first, second);
	now = get_time_ms();
	while (!is_stopped(sim) && !dongles_ready(first, second, coder))
	{
		table_wait(sim, first, second, now);
		now = get_time_ms();
	}
	unregister_interest(first, coder->id);
	unregister_interest(second, coder->id);
	if (is_stopped(sim))
		return (-1);
	return (0);
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
