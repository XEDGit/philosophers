/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.c                                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/07 04:06:52 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/07 04:06:52 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

int	destroy_data(t_data *data)
{
	if (pthread_mutex_destroy(&data->end_lock) || \
	pthread_mutex_destroy(&data->print))
		return (true);
	return (false);
}

int	error(char *msg, int ret, t_philo *tofree, t_data *data)
{
	int	i;

	printf("Error: %s\n", msg);
	if (tofree)
	{
		if (data)
		{
			i = 0;
			while (i != data->num)
			{
				if (pthread_mutex_lock(&tofree[i].state_lock))
					continue ;
				tofree[i].state = DIE;
				pthread_mutex_unlock(&tofree[i++].state_lock);
			}
			if (free_all(tofree, data->num))
				error("Error destroying mutexes", 0, 0, 0);
		}
		else
			free(tofree);
	}
	if (data && !tofree && destroy_data(data))
		error("Error destroying mutexes", 0, 0, 0);
	return (ret);
}

int	main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_data	data;

	if (argc != 5 && argc != 6)
	{
		printf(USAGE_MSG);
		return (error("Wrong number of arguments", 1, 0, 0));
	}
	if (parse_argv(&argv[1], &data))
		return (error("Error parsing arguments", 2, 0, 0));
	if (initialize_data(&data))
		return (error("Error initializing data", 3, 0, &data));
	if (init_philosophers(&philosophers, data.num, &data))
		return (error("Error initializing philosophers", 4, \
		philosophers, &data));
	if (start_dinner(philosophers, data.num))
		return (error("Error starting the philosophers", 5, \
		philosophers, &data));
	if (data.max_meals == -1 && wait_for_starve(philosophers, &data))
		return (error("Error during the dinner", 6, philosophers, &data));
	else if (data.max_meals != -1 && wait_for_meals(philosophers, &data))
		return (error("Error during the dinner", 6, philosophers, &data));
	if (free_all(philosophers, data.num))
		return (error("Error destroying mutexes", 7, philosophers, &data));
	return (0);
}
