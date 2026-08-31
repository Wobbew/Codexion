
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

/*
** Stops waiting once this coder is first in line, the dongle is
** available, and its cooldown has elapsed. Also bails out if the
** simulation has been signalled to stop, so a queued coder never
** hangs forever past shutdown.
*/
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

/*
** Shifts the queue left, stopping one short of the end so we never
** read waiters[waiter_count], which would be out of bounds once
** waiter_count reaches its max of 2.
*/
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

int	sort_queue(t_dongle *dongle, t_waiter waiter)
{
	int			i;
	t_waiter	tmp;

	i = 0;
	dongle->waiters[dongle->waiter_count] = waiter;
	while (i < dongle->waiter_count)
	{
		if (dongle->waiters[i].key > dongle->waiters[i + 1].key)
		{
			tmp = dongle->waiters[i];
			dongle->waiters[i] = dongle->waiters[i + 1];
			dongle->waiters[i + 1] = tmp;
		}
		i++;
	}
	dongle->waiter_count++;
	return (0);
}