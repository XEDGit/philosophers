/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   philo_utils.c                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/10 07:11:26 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/10 07:11:26 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

int	check_starve(t_philo *data, int update)
{
	long	now;

	now = gettime();
	if (now == ERROR)
		return (ERROR);
	if (now - data->eat_time >= data->time->die)
		return (true);
	if (update)
		data->eat_time = now;
	return (false);
}

static void	update_state(int *state)
{
	if (*state < 2)
		*state += 1;
	else
		*state = 0;
}

int	print_state(t_philo *data, bool fork)
{
	long		now;
	static char	*msg[4] = {
		"%ldms: philosopher %d is eating\n",
		"%ldms: philosopher %d is sleeping\n",
		"%ldms: philosopher %d is thinking\n",
		"%ldms: philosopher %d has taken a fork\n"
	};

	if (pthread_mutex_lock(data->print))
		return (ERROR);
	now = gettime();
	if (now == ERROR)
		return (ERROR);
	if (!fork && data->state != DIE)
		printf(msg[data->state], now, data->id);
	else if (data->state != DIE)
		printf(msg[3], now, data->id);
	if (!fork && data->state != DIE)
		update_state(&data->state);
	if (pthread_mutex_unlock(data->print))
		return (ERROR);
	return (SUCCESS);
}

int	take_fork(int i, t_philo *data)
{
	pthread_mutex_t	*fork;
	int				*fork_value;

	fork = &data->fork;
	fork_value = &data->fork_value;
	if (i)
	{
		fork = data->next;
		fork_value = data->next_value;
	}
	while (*fork_value)
	{
		if (check_starve(data, false) == true || data->state == DIE)
			return (STARVE);
		usleep(50);
	}
	if (pthread_mutex_lock(fork))
		return (ERROR);
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
