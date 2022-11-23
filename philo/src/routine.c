/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   routine.c                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/12 05:56:19 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/12 05:56:19 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

bool	initialize_data(t_data *data)
{
	data->end = 0;
	if (pthread_mutex_init(&data->print, 0) || \
	pthread_mutex_init(&data->end_lock, 0))
		return (true);
	return (false);
}

void	init_t_philo(t_philo *philosophers, int i, t_data *data)
{
	philosophers[i].id = i + 1;
	philosophers[i].ret = -2;
	philosophers[i].eat_time = 0;
	philosophers[i].num_meals = 0;
	philosophers[i].state = 0;
	philosophers[i].fork_value = 0;
	philosophers[i].meal_mode = true;
	if (data->max_meals == -1)
		philosophers[i].meal_mode = false;
	if (i != data->num - 1)
	{
		philosophers[i].next_value = &philosophers[i + 1].fork_value;
		philosophers[i].next = &philosophers[i + 1].fork;
	}
	else
	{
		philosophers[i].next_value = &philosophers[0].fork_value;
		philosophers[i].next = &philosophers[0].fork;
	}
	philosophers[i].time = &data->time;
	philosophers[i].print = &data->print;
	philosophers[i].end = &data->end;
	philosophers[i].end_lock = &data->end_lock;
}

bool	init_philosophers(t_philo	**philosophers, int num, \
t_data *data)
{
	int		i;

	i = 0;
	*philosophers = malloc(sizeof(t_philo) * num);
	if (!*philosophers)
		return (true);
	while (i != num)
	{
		init_t_philo(*philosophers, i, data);
		if (pthread_mutex_init(&(*philosophers)[i].state_lock, 0) || \
		(!i && pthread_mutex_init(&(*philosophers)[i].fork, 0)) || \
		(num != 1 && pthread_mutex_init((*philosophers)[i].next, 0)))
			return (true);
		i++;
	}
	return (false);
}

bool	start_dinner(t_philo *philosophers, int num)
{
	int	i;

	i = 0;
	if (gettime() == ERROR)
		return (true);
	while (i < num)
	{
		if (pthread_create(&philosophers[i].thread, 0, \
		&philosopher_routine, (void *)&philosophers[i]))
			return (true);
		i += 1;
	}
	return (false);
}

int	free_all(t_philo *philosophers, int num)
{
	int	i;
	int	ret;

	i = 0;
	ret = SUCCESS;
	while (i != num)
	{
		pthread_join(philosophers[i].thread, 0);
		if (pthread_mutex_lock(&philosophers[i].fork) || \
		pthread_mutex_unlock(&philosophers[i].fork) || \
		pthread_mutex_destroy(&philosophers[i].fork) || \
		pthread_mutex_lock(&philosophers[i].state_lock) || \
		pthread_mutex_unlock(&philosophers[i].state_lock) || \
		pthread_mutex_destroy(&philosophers[i].state_lock))
			ret = ERROR;
		i++;
	}
	if (pthread_mutex_destroy(philosophers[0].print) || \
	pthread_mutex_destroy(philosophers[0].end_lock))
		ret = ERROR;
	free(philosophers);
	return (ret);
}
