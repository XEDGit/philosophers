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
		if (data->state == STARVE || starve == true)
		{
			data->ret = data->id;
			pthread_mutex_unlock(data->end);
			return (0);
		}
		if (data->state == 0)
			ret = ph_eat(data);
		else if (data->state == 1)
			ret = ph_sleep(data);
		else if (data->state == 2)
			ret = ph_think(data);
	}
	return (0);
}
