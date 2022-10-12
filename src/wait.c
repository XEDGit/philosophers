/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   wait.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/12 05:55:16 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/12 05:55:16 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

bool	finalize(t_philo *philosophers, t_data *data, int i)
{
	int		c;
	long	now;

	if (pthread_mutex_lock(philosophers[i].print))
		return (true);
	c = 0;
	while (c != data->num)
		philosophers[c++].state = DIE;
	now = gettime();
	if (now == ERROR || philosophers[i].ret == ERROR)
		return (true);
	printf("%ldms: philosopher %d died\n", now, philosophers[i].ret);
	if (pthread_mutex_unlock(philosophers[i].print))
		return (true);
	c = 0;
	while (c != data->num)
		pthread_join(philosophers[c++].thread, 0);
	return (false);
}

bool	wait_for_starve(t_philo *philosophers, t_data *data)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&data->end);
	while (philosophers[i].ret == -2)
		i++;
	return (finalize(philosophers, data, i));
}

bool	finalize_meals(t_philo *philosophers, t_data *data)
{
	int		i;

	i = 0;
	if (pthread_mutex_lock(philosophers[0].print))
		return (true);
	while (i != data->num)
		philosophers[i++].state = DIE;
	if (pthread_mutex_unlock(philosophers[0].print))
		return (true);
	i = 0;
	while (i != data->num)
		pthread_join(philosophers[i++].thread, 0);
	return (false);
}

bool	wait_for_meals(t_philo *philosophers, t_data *data)
{
	int		i;
	bool	end;

	while (true)
	{
		i = 0;
		end = true;
		while (i != data->num)
		{
			if (philosophers[i].num_meals < data->max_meals)
				end = false;
			if (philosophers[i].ret != -2)
				break ;
			i++;
		}
		if (end)
			return (finalize_meals(philosophers, data));
		if (i != data->num)
			return (finalize(philosophers, data, i));
	}
}
