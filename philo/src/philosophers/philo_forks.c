/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_forks.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmuzio <lmuzio@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/11/12 17:56:44 by lmuzio            #+#    #+#             */
/*   Updated: 2022/11/23 19:04:28 by lmuzio           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

void	choose_fork(pthread_mutex_t **fork, int **fork_value, \
int i, t_philo *data)
{
	if (i)
	{
		*fork = data->next;
		*fork_value = data->next_value;
	}
	else
	{
		*fork = &data->fork;
		*fork_value = &data->fork_value;
	}
}

int	take_fork_starve(t_philo *data)
{
	if (pthread_mutex_unlock(&data->state_lock))
		return (ERROR);
	return (STARVE);
}

int	take_fork(int i, t_philo *data)
{
	pthread_mutex_t	*fork;
	int				*fork_value;

	choose_fork(&fork, &fork_value, i, data);
	while (1)
	{
		if (pthread_mutex_lock(fork))
			return (ERROR);
		if (!*fork_value)
			break ;
		if (pthread_mutex_unlock(fork))
			return (ERROR);
		if (pthread_mutex_lock(&data->state_lock))
			return (ERROR);
		if (check_starve(data, false) == true || data->state == DIE)
			return (take_fork_starve(data));
		if (pthread_mutex_unlock(&data->state_lock))
			return (ERROR);
		usleep(800);
	}
	*fork_value = 1;
	if (pthread_mutex_unlock(fork) || \
	print_state(data, true))
		return (ERROR);
	return (SUCCESS);
}

bool	release_forks(t_philo *data)
{
	if (pthread_mutex_lock(&data->fork))
		return (true);
	data->fork_value = 0;
	if (pthread_mutex_unlock(&data->fork))
		return (true);
	if (pthread_mutex_lock(data->next))
		return (true);
	*data->next_value = 0;
	if (pthread_mutex_unlock(data->next))
		return (true);
	return (false);
}
