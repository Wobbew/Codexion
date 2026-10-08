/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   heap_move.c                                        :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/10/08 17:16:01 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/10/08 17:16:50 by wwiedijk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	sift_up(t_dongle *d, int i)
{
	int	parent;

	while (i > 0)
	{
		parent = (i - 1) / 2;
		if (!waiter_less(&d->waiters[i], &d->waiters[parent]))
			return ;
		waiter_swap(&d->waiters[i], &d->waiters[parent]);
		i = parent;
	}
}

void	sift_down(t_dongle *d, int i)
{
	int	smallest;
	int	l;
	int	r;

	while (1)
	{
		smallest = i;
		l = 2 * i + 1;
		r = 2 * i + 2;
		if (l < d->waiter_count
			&& waiter_less(&d->waiters[l], &d->waiters[smallest]))
			smallest = l;
		if (r < d->waiter_count
			&& waiter_less(&d->waiters[r], &d->waiters[smallest]))
			smallest = r;
		if (smallest == i)
			return ;
		waiter_swap(&d->waiters[i], &d->waiters[smallest]);
		i = smallest;
	}
}
