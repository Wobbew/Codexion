/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle_helper.c                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/10/07 18:42:34 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/10/07 18:43:54 by wwiedijk      ########   odam.nl         */
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

// //looks if a dongle is ready to be used
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
