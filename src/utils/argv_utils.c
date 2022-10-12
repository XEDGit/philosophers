/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   argv_utils.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/12 05:51:57 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/12 05:51:57 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

bool	ph_isdigit(char *str)
{
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (false);
		str++;
	}
	return (true);
}

int	ft_atoi(const char *str)
{
	long		res;
	int			min;

	res = 0;
	min = 0;
	if (!*str)
		return (res);
	while (*str == ' ')
		str++;
	if (*str == '-')
		min = 1;
	if (*str == '-' || *str == '+')
		str++;
	while (*str >= '0' && *str <= '9')
	{
		res *= 10;
		res += *str++ - '0';
	}
	if (min)
		res *= -min;
	return (res);
}

bool	parse_argv(char **argv, t_data *data)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		if (!ph_isdigit(argv[i++]))
			return (true);
	}
	data->num = ft_atoi(argv[0]);
	data->time.die = ft_atoi(argv[1]);
	data->time.eat = ft_atoi(argv[2]);
	data->time.sleep = ft_atoi(argv[3]);
	if (argv[4])
		data->max_meals = ft_atoi(argv[4]);
	else
		data->max_meals = -1;
	if (!data->max_meals || !data->num || !data->time.sleep || \
	!data->time.eat || !data->time.die)
	{
		printf("Values should not be 0\n");
		return (true);
	}
	return (false);
}
