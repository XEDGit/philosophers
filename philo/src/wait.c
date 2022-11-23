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

	c = 0;
	if (pthread_mutex_lock(philosophers[c].print))
		return (true);
	while (c != data->num)
	{
		pthread_mutex_lock(&philosophers[c].state_lock);
		philosophers[c].state = DIE;
		pthread_mutex_unlock(&philosophers[c++].state_lock);
	}
	pthread_mutex_lock(&data->end_lock);
	now = gettime();
	if (now == ERROR || philosophers[i].ret == ERROR)
		return (true);
	printf("%ldms: philosopher %d died\n", now, philosophers[i].ret);
	if (pthread_mutex_unlock(&data->end_lock) || \
	pthread_mutex_unlock(philosophers[i].print))
		return (true);
	return (false);
}

bool	wait_for_starve(t_philo *philosophers, t_data *data)
{
	int	i;

	i = 0;
	while (1)
	{
		if (pthread_mutex_lock(philosophers[i].end_lock))
			return (true);
		if (data->end)
			break ;
		if (pthread_mutex_unlock(philosophers[i].end_lock))
			return (true);
		usleep(500);
	}
	while (philosophers[i].ret == -2)
		i++;
	if (pthread_mutex_unlock(philosophers[i].end_lock))
		return (true);
	return (finalize(philosophers, data, i));
}

bool	finalize_meals(t_philo *philosophers, t_data *data)
{
	int		i;

	i = 0;
	if (pthread_mutex_lock(philosophers[0].print))
		return (true);
	while (i != data->num)
	{
		pthread_mutex_lock(&philosophers[i].state_lock);
		philosophers[i].state = DIE;
		pthread_mutex_unlock(&philosophers[i++].state_lock);
	}
	if (pthread_mutex_unlock(philosophers[0].print))
		return (true);
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
		pthread_mutex_lock(&data->end_lock);
		while (i != data->num)
		{
			if (philosophers[i].num_meals < data->max_meals)
				end = false;
			if (philosophers[i].ret != -2)
				break ;
			i++;
		}
		pthread_mutex_unlock(&data->end_lock);
		if (i != data->num)
			return (finalize(philosophers, data, i));
		if (end)
			return (finalize_meals(philosophers, data));
	}
}
