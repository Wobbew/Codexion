/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   dongle_queue.c                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:23:48 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/11/23 23:33:02 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

// must be called with table_mutex held
// registers the coder in both heaps, waits until it is at the head of
// both and both dongles are ready, then removes it from both heaps
int	wait_for_dongles(t_coder *coder, t_dongle *first, t_dongle *second)
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