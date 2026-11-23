/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   parsing.c                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: wobbe <wobbe@student.42.fr>                  +#+                     */
/*                                                   +#+                      */
/*   Created: 2026/08/31 13:26:42 by wwiedijk      #+#    #+#                 */
/*   Updated: 2026/09/11 15:46:12 by wobbe         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
//checks if a string is a valid integer
int	ft_isnumeric(char *s)
{
	int	i;

	i = 0;
	if (s[i] == '+' || s[i] == '-')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

bool	test_args(t_params *params)
{
	if (params->num_coders < 1)
		return (input_error_int());
	if (params->time_to_burnout < 0)
		return (input_error_int());
	if (params->time_to_compile < 0)
		return (input_error_int());
	if (params->time_to_debug < 0)
		return (input_error_int());
	if (params->time_to_refactor < 0)
		return (input_error_int());
	if (params->num_compiles_required < 1)
		return (input_error_int());
	if (params->dongle_cooldown < 0)
		return (input_error_int());
	return (true);
}
//fills the params struct with the provided arguments
static bool	fill_params(char **argv, t_params *params)
{
	params->num_coders = atoi(argv[1]);
	params->time_to_burnout = atoi(argv[2]);
	params->time_to_compile = atoi(argv[3]);
	params->time_to_debug = atoi(argv[4]);
	params->time_to_refactor = atoi(argv[5]);
	params->num_compiles_required = atoi(argv[6]);
	params->dongle_cooldown = atoi(argv[7]);
	if (strcmp(argv[8], "fifo") == 0)
		params->scheduler = 0;
	else if (strcmp(argv[8], "edf") == 0)
		params->scheduler = 1;
	else
		return (input_error_schedul());
	return (true);
}
//takes all arguments and fills the params struct
bool	params_args(int argc, char **argv, t_params *params)
{
	int	i;
	if (argc != 9)
		return (input_error());
	i = 1;
	while (i < argc - 1)
	{
		if (ft_isnumeric(argv[i]) == 0)
			return (input_error_int());
		i++;
	}
	if (fill_params(argv, params) != true)
		return (false);
	if (test_args(params) != true)
		return (false);
	return (true);
}
