/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongle.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wwiedijk <wwiedijk@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:23:21 by wwiedijk          #+#    #+#             */
/*   Updated: 2026/08/31 13:23:26 by wwiedijk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static long	waiter_key(t_coder *coder)
{
	if (coder->sim->params.scheduler == 0)
		return (get_time_ms());
	return (coder->last_compile_ms + coder->sim->params.time_to_burnout);
}

int	get_dongle(t_coder *coder, t_dongle *dongle)
{
	t_waiter	waiter;

	pthread_mutex_lock(&dongle->mutex);
	if (coder->sim->stopped)
		return (pthread_mutex_unlock(&dongle->mutex));
	waiter.cond = &coder->cond;
	waiter.key = waiter_key(coder);
	waiter.coder_id = coder->id;
	sort_queue(dongle, waiter);
	wait_for_dongle(coder, dongle);
	if (coder->sim->stopped)
		return (pthread_mutex_unlock(&dongle->mutex));
	pops_queue(dongle);
	dongle->available = 0;
	log_state(coder->sim, coder->id, "has taken a dongle");
	return (pthread_mutex_unlock(&dongle->mutex));
}

static void	release_first_after_abort(t_dongle *first)
{
	pthread_mutex_lock(&first->mutex);
	first->available = true;
	first->available_at = get_time_ms();
	if (first->waiter_count > 0)
		pthread_cond_broadcast(first->waiters[0].cond);
	pthread_mutex_unlock(&first->mutex);
}

int	take_dongle(t_coder *coder)
{
	t_dongle	*first;
	t_dongle	*second;

	if (coder->id == coder->sim->params.num_coders)
	{
		first = coder->dongle_right;
		second = coder->dongle_left;
	}
	else
	{
		first = coder->dongle_left;
		second = coder->dongle_right;
	}
	get_dongle(coder, first);
	if (coder->sim->stopped)
		return (-1);
	get_dongle(coder, second);
	if (coder->sim->stopped)
	{
		release_first_after_abort(first);
		return (-1);
	}
	return (0);
}
