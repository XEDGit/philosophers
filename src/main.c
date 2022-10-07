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
	int				state;
	long			eat_time;
	t_times			*time;
	pthread_t		thread;
	pthread_mutex_t	fork;
	pthread_mutex_t	*next;
	pthread_mutex_t	*print;
}	t_philo;

#define STARVE 4
#define ERROR -1
#define SUCCESS 0
#define USAGE_MSG "Usage:\t./philosopher time_to_die time_to_eat time_to_sleep \
[number_of_times_each_philosopher_must_eat]\n"

int	error(char *msg, int ret)
{
	printf("Error: %s\n", msg);
	return (ret);
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

long	gettime(void)
{
	static struct timeval	start = {0, 0};
	struct timeval			now;

	if (start.tv_sec == 0 && start.tv_usec == 0 && \
	gettimeofday(&start, 0))
		return (ERROR);
	if (gettimeofday(&now, 0))
		return (ERROR);
	return (((now.tv_sec * 1000000 + now.tv_usec) - \
	(start.tv_sec * 1000000 + start.tv_usec)) / 1000);
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
			return (true);
	}
	*num = ft_atoi(argv[0]);
	time->die = ft_atoi(argv[1]);
	time->eat = ft_atoi(argv[2]);
	time->sleep = ft_atoi(argv[3]);
	return (false);
}

int	check_starve(t_philo *data)
{
	long	now;

	now = gettime();
	if (now == ERROR)
		return (ERROR);
	// printf("DB: %d time from eating %ld\n", data->id, now - data->eat_time );
	if (now - data->eat_time >= data->time->die)
		return (true);
	if (data->state == 0)
		data->eat_time = now;
	return (false);
}

int	print_state(int id, int state, pthread_mutex_t *print)
{
	long		now;
	static char	*msg[4] = {
		"%ldms: philosopher %d is eating\n",
		"%ldms: philosopher %d is sleeping\n",
		"%ldms: philosopher %d is thinking\n",
		"%ldms: philosopher %d has taken a fork\n"
	};

	now = gettime();
	if (state == 3)
		return (SUCCESS);
	if (now == ERROR || pthread_mutex_lock(print))
		return (ERROR);
	printf(msg[state], now, id);
	if (pthread_mutex_unlock(print))
		return (ERROR);
	return (SUCCESS);
}

int	ph_eat(t_philo *data)
{
	if (\
	pthread_mutex_lock(&data->fork) || print_state(data->id, 3, data->print) || \
	pthread_mutex_lock(data->next) || print_state(data->id, 3, data->print))
		return (ERROR);
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	msleep(data->time->eat);
	if (pthread_mutex_unlock(&data->fork) || \
	pthread_mutex_unlock(data->next))
		return (ERROR);
	if (data->state != 3)
		data->state = 1;
	return (SUCCESS);
}

int	ph_sleep(t_philo *data)
{
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	msleep(data->time->sleep);
	if (data->state != 3)
		data->state = 2;
	return (false);
}

int	ph_think(t_philo *data)
{
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	if (data->state != 3)
		data->state = 0;
	return (false);
}

void	*philosopher_routine(void *arg)
{
	t_philo	*data;
	int		ret;
	int		starve;

	data = (t_philo *)arg;
	while (data->ret == -2 && data->state != 3)
	{
		starve = check_starve(data);
		if (ret == ERROR || starve == ERROR)
			data->ret = ERROR;
		if (data->state == 3 || starve == true)
		{
			data->ret = data->id;
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

void	init_t_philo(t_philo *philosophers, int i, int num, t_times *time)
{
	philosophers[i].id = i + 1;
	philosophers[i].ret = -2;
	philosophers[i].eat_time = 0;
	philosophers[i].state = 0;
	if (i != num - 1)
		philosophers[i].next = &philosophers[i + 1].fork;
	else
		philosophers[i].next = &philosophers[0].fork;
	philosophers[i].time = time;
}

bool	initialize_philosophers(t_philo	**philosophers, int num, \
								t_times *time, pthread_mutex_t *print)
{
	int		i;

	i = 0;
	*philosophers = malloc(sizeof(t_philo) * num);
	if (!philosophers || pthread_mutex_init(print, 0))
		return (true);
	while (i != num)
	{
		init_t_philo(*philosophers, i, num, time);
		(*philosophers)[i].print = print;
		if ((!i && pthread_mutex_init(&(*philosophers)[i].fork, 0)) || \
		pthread_mutex_init((*philosophers)[i].next, 0))
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
	while (i != num)
	{
		if (i % 2)
			msleep(1);
		if (pthread_create(&philosophers[i].thread, 0, \
		&philosopher_routine, (void *)&philosophers[i]))
			return (true);
		i++;
	}
	return (false);
}
		
void *alive(void *)
{
	while (1)
	{
		dprintf(2, "%d Alive!\n", getpid());
		msleep(1000);
	}
	return (0);
}

bool	wait_for_starve(t_philo *philosophers, int num)
{
	long	i;
	int		ret;

	while (true)
	{
		i = 0;
		while (i != num)
		{
			if (philosophers[i].ret != -2)
			{
				ret = philosophers[i].ret;
				i = 0;
				while (i != num)
					philosophers[i++].state = 3;
				i = gettime();
				if (i == ERROR || ret == ERROR)
					return (true);
				msleep(5);
				if (pthread_mutex_lock(philosophers[0].print))
					return (true);
				printf("%ldms: philosophers %d died\n", i, ret);
				return (false);
			}
			i++;
		}
	}
}

int	free_all(t_philo *philosophers, int num)
{
	int	i;
	int	ret;

	i = 0;
	ret = SUCCESS;
	// while (i != num)
	// 	pthread_mutex_unlock(&philosophers[i++].fork);
	i = 0;
	pthread_mutex_unlock(philosophers[0].print);
	while (i != num)
	{
		pthread_join(philosophers[i].thread, 0);
		if (pthread_mutex_destroy(&philosophers[i].fork))
			// printf("error destroying %d\n", i);
			ret = ERROR;
		i++;
	}
	if (pthread_mutex_destroy(philosophers[0].print))
		ret = ERROR;
	free(philosophers);
	return (ret);
}

int main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_times	time;
	int		ph_num;
	pthread_mutex_t	print;

	if (argc != 5 && argc != 6)
	{
		printf(USAGE_MSG);
		printf("Wrong number of arguments");
		return (1);
	}
	// pthread_t aliv;
	// pthread_create(&aliv, 0, &alive, 0);
	if (parse_argv(&argv[1], &ph_num, &time))
		return (error("Error parsing arguments", 2));
	if (initialize_philosophers(&philosophers, ph_num, &time, &print))
		return (error("Error initializing philosophers", 3));
	if (start_dinner(philosophers, ph_num))
		return (error("Error starting the philosophers", 4));
	if (wait_for_starve(philosophers, ph_num))
		return (error("Error waiting for philosophers", 5));
	if (free_all(philosophers, ph_num))
		return (error("Error destroying mutexes", 6));
	return (0);
}