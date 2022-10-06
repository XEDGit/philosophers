#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct s_times {
	int	eat;
	int	sleep;
	int	die;
}	t_times;

typedef struct s_philo {
	int				id;
	pthread_t       thread;
	pthread_mutex_t fork;
	int             state;
	t_times			*time;
	pthread_mutex_t	*prev;
	pthread_mutex_t	*next;
}	t_philo;

#define USAGE_MSG "Usage:\t./philosopher time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]\n"

int	error(char *msg, int usage, int ret)
{
	printf("Error: %s\n", msg);
	if (usage)
		printf(USAGE_MSG);
	return (ret);
}

void	free_all(t_philo *philosophers, int num)
{
	int	i;

	i = 0;
	while (i != num)
	{
		pthread_mutex_destroy(&philosophers[i].fork);
		i++;
	}
	free(philosophers);
}

bool	ph_isdigit(char *str)
{
	while (*str)
	{
		if (*str < '0' || *str > '9')
			return (false);
		str++;
	}
	return (true);
}

int	ft_atoi(const char *str)
{
	long		res;
	int			min;

	res = 0;
	min = 0;
	if (!*str)
		return (res);
	while (*str == ' ')
		str++;
	if (*str == '-')
		min = 1;
	if (*str == '-' || *str == '+')
		str++;
	while (*str >= '0' && *str <= '9')
	{
		res *= 10;
		res += *str++ - '0';
	}
	if (min)
		res *= -min;
	return (res);
}

void	msleep(unsigned int ms)
{
	while (ms--)
		usleep(1000);
}

bool	parse_argv(char **argv, int *num, t_times *time)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		if (!ph_isdigit(argv[i++]))
			return (false);
	}
	*num = ft_atoi(argv[0]);
	time->die = ft_atoi(argv[1]);
	time->eat = ft_atoi(argv[2]);
	time->sleep = ft_atoi(argv[3]);
	return (true);
}

void	*philosopher_routine(void *arg)
{
	t_philo *data = (t_philo *)arg;
	return (0);
}

bool	initialize_philosophers(t_philo	**philosophers_pointer, int num, t_times *time)
{
	int		i;
	t_philo	*philosophers;

	i = 0;
	*philosophers_pointer = malloc(sizeof(t_philo) * num);
	philosophers = *philosophers_pointer;
	while (i != num)
	{
		philosophers[i].id = i;
		philosophers[i].state = 0;
		if (i)
			philosophers[i].prev = &philosophers[i - 1].fork;
		if (i != num - 1)
		philosophers[i].next = &philosophers[i + 1].fork;
		philosophers[i].time = time;
		if (pthread_mutex_init(&philosophers[i].fork, 0) || \
			pthread_create(&philosophers[i++].thread, 0, \
			&philosopher_routine, (void *)&philosophers[i]) || \
			pthread_join(philosophers[i].thread, 0))
			return (false);
	}
	philosophers[num - 1].next = &philosophers[0].fork;
	philosophers[0].prev = &philosophers[num - 1].fork;
	return (true);
}

bool	wait_for_starve(t_philo *philosophers, int num, t_times *time)
{
	int	i;

	i = 0;
	while (1)
	{
		msleep()
		while (i != num)
		{
			if ()
				return (false);
			i++;
		}
	}

	return (true);
}

int main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_times	time;
	int		ph_num;

	if (argc != 5 && argc != 6)
		return (error("Wrong number of arguments", 1, 1));
	if (!parse_argv(&argv[1], &ph_num, &time))
		return (error("Error parsing arguments", 0, 2));
	if (!initialize_philosophers(&philosophers, ph_num, &time))
		return (error("Error initializing philosophers", 0, 3));
	if (!wait_for_starve(philosophers, ph_num))
		return (error("Error waiting for philosophers", 0, 4));
	free_all(philosophers, ph_num);
	return (0);
}