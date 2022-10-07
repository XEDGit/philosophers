#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>

typedef struct s_times {
	int	eat;
	int	sleep;
	int	die;
}	t_times;

typedef struct s_philo {
	int				id;
	int				ret;
	int             state;
	long			eat_time;
	t_times			*time;
	pthread_t       thread;
	pthread_mutex_t fork;
	pthread_mutex_t	*next;
}	t_philo;

#define USAGE_MSG "Usage:\t./philosopher time_to_die time_to_eat time_to_sleep [number_of_times_each_philosopher_must_eat]\n"

int	error(char *msg, int ret)
{
	printf("Error: %s\n", msg);
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

#define STARVE 4
#define ERROR -1
#define SUCCESS 0

long	gettime()
{
	static struct timeval	start = {0, 0};
	struct timeval			now;

	if (start.tv_sec == 0 && start.tv_usec == 0 && \
	gettimeofday(&start, 0))
		return (ERROR);
	if (gettimeofday(&now, 0))
		return (ERROR);
	return ((now.tv_sec * 1000000 + now.tv_usec) - \
	(start.tv_sec * 1000000 + start.tv_usec));
}

void	msleep(unsigned int ms)
{
	ms++;
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

int	check_starve(int update_eat_time, t_philo *data)
{
	long	now;

	now = gettime();
	if (now == ERROR)
		return (ERROR);
	if (data->eat_time && (now / 1000) - data->eat_time > data->time->die)
		return (true);
	if (update_eat_time)
		data->eat_time = now;
	return (false);
}

int	print_state(int id, int state)
{
	long		now;
	static char	*msg[5] = {
		"%ldms: philosopher %d is eating\n",
		"%ldms: philosopher %d is thinking\n",
		"%ldms: philosopher %d is sleeping\n",
		"%ldms: philosopher %d has taken a fork\n"
		"%ldms: philosopher %d died\n"
	};

	now = gettime();
	if (now == ERROR)
		return (ERROR);
	printf(msg[state], now / 1000, id);
	if (state == STARVE)
		return (STARVE);
	return (SUCCESS);
}

int	ph_eat(t_philo *data)
{
	int		starve;

	starve = check_starve(true, data);
	if (starve == true)
		return (print_state(data->id, STARVE));
	if (pthread_mutex_lock(&data->fork) || print_state(data->id, 3) || \
	pthread_mutex_lock(data->next) || print_state(data->id, 3))
		return (ERROR);
	if (starve == ERROR || \
	print_state(data->id, data->state))
		return (ERROR);
	msleep(data->time->eat);
	if (pthread_mutex_unlock(&data->fork) || \
	pthread_mutex_unlock(data->next))
		return (ERROR);
	data->state = 1;
	return (SUCCESS);
}


int	ph_sleep(t_philo *data)
{
	int		starve;

	starve = check_starve(false, data);
	if (starve == true)
		return (print_state(data->id, STARVE));
	if (starve == ERROR || print_state(data->id, data->state))
		return (ERROR);
	msleep(data->time->sleep);
	data->state = 2;
	return (false);
}

int	ph_think(t_philo *data)
{
	int		starve;

	starve = check_starve(false, data);
	if (starve == true)
		return (print_state(data->id, STARVE));
	if (starve == ERROR || print_state(data->id, data->state))
		return (ERROR);
	data->state = 0;
	return (false);
}

void	*philosopher_routine(void *arg)
{
	t_philo *data;
	int		*ret;

	ret = malloc(sizeof(int));
	data = (t_philo *)arg;
	while (1)
	{
		if (data->state == 0)
			*ret = ph_eat(data);
		else if (data->state == 1)
			*ret = ph_sleep(data);
		else if (data->state == 2)
			*ret = ph_think(data);
		if (*ret == STARVE)
			return ((void *) ret);
	}
	free(ret);
	return (0);
}

void	init_t_philo(t_philo *philosophers, int i, int num, t_times *time)
{
	philosophers[i].id = i;
	philosophers[i].ret = -1;
	philosophers[i].eat_time = 0;
	if (i % 2)
		philosophers[i].state = 2;
	else
		philosophers[i].state = 0;
	if (i != num - 1)
		philosophers[i].next = &philosophers[i + 1].fork;
	else
		philosophers[i].next = &philosophers[0].fork;
	philosophers[i].time = time;
}

bool	initialize_philosophers(t_philo	**philosophers_pointer, int num, t_times *time)
{
	int		i;
	t_philo	*philosophers;

	i = 0;
	*philosophers_pointer = malloc(sizeof(t_philo) * num);
	philosophers = *philosophers_pointer;
	if (gettime() == ERROR)
		return (false);
	while (i != num)
	{
		init_t_philo(philosophers, i, num, time);
		if (!i && pthread_mutex_init(&philosophers[i].fork, 0))
			return (false);
		if (pthread_mutex_init(philosophers[i].next, 0) || \
		pthread_create(&philosophers[i].thread, 0, \
		&philosopher_routine, (void *)&philosophers[i]))
			return (false);
		i++;
	}
	return (true);
}

bool	wait_for_starve(t_philo *philosophers, int num)
{
	int	i;
	int	ret;

	ret = -1;
	while (1)
	{
		i = 0;
		while (i != num)
		{
			if (philosophers[i].ret != -1)
			{
				ret = philosophers[i].ret;
				break ;
			}
			i++;
		}
		if (ret != -1)
			break ;
	}
	printf("finishedddd with %d\n", ret);
	return (true);
}

int main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_times	time;
	int		ph_num;

	if (argc != 5 && argc != 6)
	{
		printf(USAGE_MSG);
		printf("Wrong number of arguments");
		return (1);
	}
	if (!parse_argv(&argv[1], &ph_num, &time))
		return (error("Error parsing arguments", 2));
	if (!initialize_philosophers(&philosophers, ph_num, &time))
		return (error("Error initializing philosophers", 3));
	if (!wait_for_starve(philosophers, ph_num))
		return (error("Error waiting for philosophers", 4));
	free_all(philosophers, ph_num);
	return (0);
}