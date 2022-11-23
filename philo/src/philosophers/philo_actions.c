/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   philo_actions.c                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/12 05:49:03 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/12 05:49:03 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

int	ph_eat(t_philo *data)
{
	int	ret[2];

	ret[0] = take_fork(0, data);
	if (ret[0] == STARVE)
		return (SUCCESS);
	ret[1] = take_fork(1, data);
	if (ret[1] == STARVE)
		return (SUCCESS);
	if (ret[0] == ERROR || ret[1] == ERROR || \
	print_state(data, false) || \
	check_starve(data, true) == ERROR)
		return (ERROR);
	ret[0] = msleep_starve(data->time->eat, data);
	if (ret[0] == ERROR)
		return (ERROR);
	if (pthread_mutex_lock(data->end_lock))
		return (ERROR);
	data->num_meals++;
	if (pthread_mutex_unlock(data->end_lock))
		return (ERROR);
	return (SUCCESS);
}

int	ph_sleep(t_philo *data)
{
	if (print_state(data, false))
		return (ERROR);
	release_forks(data);
	msleep_starve(data->time->sleep, data);
	return (false);
}

int	ph_think(t_philo *data)
{
	if (print_state(data, false))
		return (ERROR);
	return (false);
}

int	philosopher_dispatch(t_philo *data)
{
	static int	(*actions[3])(t_philo *) = {
		&ph_eat,
		&ph_sleep,
		&ph_think
	};
	int			state;

	if (pthread_mutex_lock(&data->state_lock))
		return (ERROR);
	state = data->state;
	if (pthread_mutex_unlock(&data->state_lock))
		return (ERROR);
	if (state != DIE)
		return (actions[state](data));
	return (SUCCESS);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*data;
	int		starve;

	data = (t_philo *)arg;
	if (!(data->id % 2))
		msleep_starve(data->time->eat * 0.9, data);
	while (1)
	{
		starve = ph_first_routine(data);
		if (pthread_mutex_lock(&data->state_lock) || \
		data->state == DIE)
			break ;
		if (starve)
		{
			terminate_philo(data, starve);
			break ;
		}
		if (pthread_mutex_unlock(&data->state_lock))
			break ;
	}
	if (pthread_mutex_unlock(&data->state_lock))
		data->ret = ERROR;
	return (0);
}
