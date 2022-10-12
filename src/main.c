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

int	error(char *msg, int ret)
{
	printf("Error: %s\n", msg);
	return (ret);
}

int	main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_data	data;

	if (argc != 5 && argc != 6)
	{
		printf(USAGE_MSG);
		printf("Wrong number of arguments");
		return (1);
	}
	if (parse_argv(&argv[1], &data))
		return (error("Error parsing arguments", 2));
	if (initialize_data(&data))
		return (error("Error initializing data", 3));
	if (initialize_philosophers(&philosophers, \
	data.num, &data))
		return (error("Error initializing philosophers", 4));
	if (start_dinner(philosophers, data.num))
		return (error("Error starting the philosophers", 5));
	if (data.max_meals == -1 && wait_for_starve(philosophers, &data))
		return (error("Error during the dinner", 6));
	else if (data.max_meals != -1 && wait_for_meals(philosophers, &data))
		return (error("Error during the dinner", 6));
	if (free_all(philosophers, data.num))
		return (error("Error destroying mutexes", 7));
	return (0);
}
