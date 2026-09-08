/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle_queue.c                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:48 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/09 00:26:19 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	timed_wait(t_coder *coder, t_dongle *dongle)
{
	struct timespec	ts;
	long			remaining_ms;

	remaining_ms = dongle->available_at - get_time_ms();
	ts.tv_sec = remaining_ms / 1000;
	ts.tv_nsec = (remaining_ms % 1000) * 1000000L;
	pthread_cond_timedwait(&coder->cond, &dongle->mutex, &ts);
}

void	wait_for_dongle(t_coder *coder, t_dongle *dongle)
{
	while (dongle->waiter_count == 0
		|| dongle->waiters[0].cond != &coder->cond
		|| !dongle->available
		|| get_time_ms() < dongle->available_at)
	{
		if (coder->sim->stopped)
			return ;
		if (dongle->available && get_time_ms() < dongle->available_at)
			timed_wait(coder, dongle);
		else
			pthread_cond_wait(&coder->cond, &dongle->mutex);
	}
}

void	pops_queue(t_dongle *dongle)
{
	int	i;

	i = 0;
	while (i < dongle->waiter_count - 1)
	{
		dongle->waiters[i] = dongle->waiters[i + 1];
		i++;
	}
	dongle->waiter_count--;
}
// sorts the waiting queue based on the waiter's key
int	sort_queue(t_dongle *dongle, t_waiter waiter)
{
	t_waiter	tmp;
	
	tmp = waiter;
	dongle->waiters[dongle->waiter_count] = waiter;

	if ((dongle->waiter_count > 0 && dongle->waiters[0].key > dongle->waiters[1].key) || (dongle->waiter_count > 0 && dongle->waiters[0].key == dongle->waiters[1].key && (dongle->waiters[0].coder_id + 1) % 2 != 0))
	{
		tmp = dongle->waiters[0];
		dongle->waiters[0] = dongle->waiters[1];
		dongle->waiters[1] = tmp;
	}
	
	dongle->waiter_count++;
	return (0);
}
