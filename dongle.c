/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle.c                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:21 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/03 00:44:52 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	waiter_key(t_coder *coder)
{
	if (coder->sim->params.scheduler == 0)
		return (coder->last_compile_ms);
	return (coder->last_compile_ms + coder->sim->params.time_to_burnout);
}

static bool	dongle_ready(t_dongle *d, long now)
{
	return (d->available && now >= d->available_at);
}

static bool	is_at_head(t_dongle *d, t_coder *coder)
{
	return (d->waiter_count > 0 && d->waiters[0].coder_id == coder->id);
}

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

static void	epoch_ms_to_ts(long epoch_ms, struct timespec *ts)
{
	ts->tv_sec = epoch_ms / 1000;
	ts->tv_nsec = (epoch_ms % 1000) * 1000000L;
}

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
		epoch_ms_to_ts(until, &ts);
		pthread_cond_timedwait(&sim->table_cond, &sim->table_mutex, &ts);
	}
}

int	take_dongle(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;
	t_sim		*sim;
	long		now;

	sim = coder->sim;
	if (coder->id == sim->params.num_coders)
	{
		first = coder->dongle_right;
		second = coder->dongle_left;
	}
	else
	{
		first = coder->dongle_left;
		second = coder->dongle_right;
	}
	pthread_mutex_lock(&sim->table_mutex);
	register_interest(coder, first, second);
	now = get_time_ms();
	while (!sim->stopped && !(dongle_ready(first, now) && dongle_ready(second, now)
			&& is_at_head(first, coder) && is_at_head(second, coder)))
	{
		table_wait(sim, first, second, now);
		now = get_time_ms();
	}
	unregister_interest(first, coder->id);
	unregister_interest(second, coder->id);
	if (sim->stopped)
	{
		pthread_mutex_unlock(&sim->table_mutex);
		return (-1);
	}
	first->available = false;
	second->available = false;
	pthread_mutex_unlock(&sim->table_mutex);
	log_state(sim, coder->id, "has taken a dongle");
	log_state(sim, coder->id, "has taken a dongle");
	return (0);./$(NAME)
}

