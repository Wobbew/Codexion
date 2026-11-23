/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle_helper.c                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/10/08 17:13:35 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:32:54 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// called under table_mutex, so next_ticket needs no extra lock
// fifo: the ticket is the real arrival order of the request
// (for the LIFO recode: return (-(coder->sim->next_ticket++));)
static long	waiter_key(t_coder *coder)
{
	if (coder->sim->params.scheduler == 0)
		return (coder->sim->next_ticket++);
	return (coder->last_compile_ms + coder->sim->params.time_to_burnout);
}

void	register_interest(t_coder *coder, t_dongle *first, t_dongle *second)
{
	t_waiter	w;

	w.cond = &coder->cond;
	w.key = waiter_key(coder);
	w.coder_id = coder->id;
	heap_push(first, w);
	if (second != first)
		heap_push(second, w);
}

void	unregister_interest(t_dongle *dongle, int coder_id)
{
	int	i;

	i = 0;
	while (i < dongle->waiter_count && dongle->waiters[i].coder_id != coder_id)
		i++;
	if (i < dongle->waiter_count)
		heap_remove(dongle, i);
}

bool	is_at_head(t_dongle *d, t_coder *coder)
{
	return (d->waiter_count > 0 && d->waiters[0].coder_id == coder->id);
}

bool	dongle_ready(t_dongle *d, long now)
{
	return (d->available && now >= d->available_at);
}