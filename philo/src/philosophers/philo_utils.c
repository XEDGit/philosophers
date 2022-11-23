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

int	ph_first_routine(t_philo *data)
{
	int	ret;
	int	starve;

	ret = philosopher_dispatch(data);
	starve = check_starve(data, false);
	pthread_mutex_lock(data->end_lock);
	if (ret == ERROR || starve == ERROR)
		data->ret = ERROR;
	pthread_mutex_unlock(data->end_lock);
	return (starve);
}

int	terminate_philo(t_philo *data)
{
	if (pthread_mutex_lock(data->end_lock))
		return (true);
	data->ret = data->id;
	*data->end = 1;
	if (pthread_mutex_unlock(data->end_lock))
		return (true);
	return (false);
}

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

static void	update_state(t_philo *data)
{
	pthread_mutex_lock(&data->state_lock);
	if (data->state < 2)
		data->state += 1;
	else
		data->state = 0;
	pthread_mutex_unlock(&data->state_lock);
}

int	print_state(t_philo *data, bool fork)
{
	long		now;
	int			state;
	static char	*msg[4] = {"%ldms: philosopher %d is eating\n",
		"%ldms: philosopher %d is sleeping\n",
		"%ldms: philosopher %d is thinking\n",
		"%ldms: philosopher %d has taken a fork\n"
	};

	if (pthread_mutex_lock(data->print))
		return (ERROR);
	pthread_mutex_lock(&data->state_lock);
	state = data->state;
	pthread_mutex_unlock(&data->state_lock);
	if (fork && state != DIE)
		state = 3;
	now = gettime();
	if (state != DIE)
		printf(msg[state], now, data->id);
	if (!fork && state != DIE)
		update_state(data);
	if (pthread_mutex_unlock(data->print))
		return (ERROR);
	if (now == ERROR)
		return (ERROR);
	return (SUCCESS);
}
