/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   release.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: wwiedijk <wwiedijk@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/31 13:27:38 by wwiedijk          #+#    #+#             */
/*   Updated: 2026/08/31 13:27:42 by wwiedijk         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	release_one(t_dongle *dongle, int cooldown)
{
	pthread_mutex_lock(&dongle->mutex);
	dongle->available = true;
	dongle->available_at = get_time_ms() + cooldown;
	if (dongle->waiter_count > 0)
		pthread_cond_broadcast(dongle->waiters[0].cond);
	pthread_mutex_unlock(&dongle->mutex);
}

int	release_dongle(t_coder *coder)
{
	release_one(coder->dongle_left, coder->sim->params.dongle_cooldown);
	release_one(coder->dongle_right, coder->sim->params.dongle_cooldown);
	return (0);
}
