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
	if (ret[0] == ERROR || ret[1] == ERROR)
		return (ERROR);
	if (data->state != DIE && print_state(data, false))
		return (ERROR);
	if (check_starve(data, true) == ERROR)
		return (ERROR);
	if (msleep_starve(data->time->eat, data) == ERROR)
		return (ERROR);
	return (SUCCESS);
}

int	ph_sleep(t_philo *data)
{
	if (print_state(data, false))
		return (ERROR);
	release_forks(data);
	data->num_meals++;
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

	if (data->state != DIE)
		return (actions[data->state](data));
	return (SUCCESS);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*data;
	int		ret;
	int		starve;

	data = (t_philo *)arg;
	ret = 0;
	while (data->state != DIE)
	{
		starve = check_starve(data, false);
		if (ret == ERROR || starve == ERROR)
			data->ret = ERROR;
		if (starve == true && data->state != DIE)
		{
			data->ret = data->id;
			*data->end = 1;
			break ;
		}
		ret = philosopher_dispatch(data);
	}
	return (0);
}
