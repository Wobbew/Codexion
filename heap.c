/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   heap.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: wwiedijk <wwiedijk@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/10/08 17:15:50 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/10/08 17:16:59 by wwiedijk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

bool	waiter_less(t_waiter *a, t_waiter *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->coder_id < b->coder_id);
}

void	waiter_swap(t_waiter *a, t_waiter *b)
{
	t_waiter	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heap_push(t_dongle *d, t_waiter w)
{
	d->waiters[d->waiter_count] = w;
	d->waiter_count++;
	sift_up(d, d->waiter_count - 1);
}

// removes the waiter at index i (i == 0 is a normal pop)
void	heap_remove(t_dongle *d, int i)
{
	d->waiter_count--;
	if (i == d->waiter_count)
		return ;
	d->waiters[i] = d->waiters[d->waiter_count];
	sift_down(d, i);
	sift_up(d, i);
}
