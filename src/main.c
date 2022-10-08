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
	int				fork_value;
	pthread_mutex_t	*next;
	int				*next_value;
	pthread_mutex_t	*print;
	pthread_mutex_t	*end;
}	t_philo;

typedef struct s_data
{
	t_times			time;
	pthread_mutex_t	print;
	pthread_mutex_t	end;
	int				num;
}	t_data;


#define STARVE 4
#define DIE 5
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

int	check_starve(t_philo *data, int update)
{
	long	now;

	now = gettime();
	if (now == ERROR)
		return (ERROR);
	// printf("DB: %d time from eating %ld, last eat was at %ld and now is %ld\n", data->id, now - data->eat_time, data->eat_time, now);
	if (now - data->eat_time >= data->time->die)
		return (true);
	if (update)
		data->eat_time = now;
	return (false);
}

void	msleep(unsigned int ms)
{
	usleep(ms * 1000);
}

int	msleep_starve(unsigned int ms, t_philo *data)
{
	int	ret;

	while (ms--)
	{
		ret = check_starve(data, false);
		if (ret)
			return (ret);
		usleep(900);
	}
	return (false);
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
	if (now == ERROR || pthread_mutex_lock(print))
		return (ERROR);
	printf(msg[state], now, id);
	if (pthread_mutex_unlock(print))
		return (ERROR);
	return (SUCCESS);
}

int	take_fork(int i, t_philo *data)
{
	pthread_mutex_t	*fork;
	int				*fork_value;

	fork = &data->fork;
	fork_value = &data->fork_value;
	if (i)
	{
		fork = data->next;
		fork_value = data->next_value;
	}
	while (*fork_value)
	{
		if (check_starve(data, false))
			return (STARVE);
		usleep(100);
	}
	if (pthread_mutex_lock(fork) || \
	print_state(data->id, 3, data->print))
		return (ERROR);
	*fork_value = 1;
	if (pthread_mutex_unlock(fork))
		return (ERROR);
	return (SUCCESS);
}

int	ph_eat(t_philo *data)
{
	int	ret[2];

	ret[0] = take_fork(0, data);
	ret[1] = take_fork(1, data);
	if (ret[0] == ERROR || ret[1] == ERROR)
		return (ERROR);
	if (ret[0] == STARVE || ret[1] == STARVE)
		return (SUCCESS);
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	// printf("%ldms: %d ret: %d %d\n", gettime(), data->id, ret[0], ret[1]);
	// printf("Buondi da %d\n", data->id);
	ret[0] = msleep_starve(data->time->eat, data);
	if (ret[0] == ERROR)
		return (ERROR);
	if (ret[0] == SUCCESS)
		return (SUCCESS);
	if (check_starve(data, true) == ERROR || \
	pthread_mutex_lock(&data->fork))
		return (ERROR);
	data->fork_value = 0;
	if (pthread_mutex_unlock(&data->fork) || \
	pthread_mutex_lock(data->next))
		return (ERROR);
	*data->next_value = 0;
	if (pthread_mutex_unlock(data->next))
		return (ERROR);
	if (data->state != DIE)
		data->state = 1;
	return (SUCCESS);
}

int	ph_sleep(t_philo *data)
{
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	msleep_starve(data->time->eat, data);
	if (data->state != DIE)
		data->state = 2;
	return (false);
}

int	ph_think(t_philo *data)
{
	if (print_state(data->id, data->state, data->print))
		return (ERROR);
	if (data->state != DIE)
		data->state = 0;
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

void	init_t_philo(t_philo *philosophers, int i, t_data *data)
{
	philosophers[i].id = i + 1;
	philosophers[i].ret = -2;
	philosophers[i].eat_time = 0;
	philosophers[i].state = 0;
	philosophers[i].fork_value = 0;
	// if (data->num == 1)
	// {
	// 	philosophers[i].next = 0;
	// 	philosophers[i].next_value = 0;
	// }
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
}

bool	initialize_philosophers(t_philo	**philosophers, int num, \
t_data *data)
{
	int		i;

	i = 0;
	*philosophers = malloc(sizeof(t_philo) * num);
	if (!philosophers)
		return (true);
	while (i != num)
	{
		init_t_philo(*philosophers, i, data);
		if ((!i && pthread_mutex_init(&(*philosophers)[i].fork, 0)) || \
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
	while (i != num)
	{
		if (i % 2)
			usleep(50);
		if (pthread_create(&philosophers[i].thread, 0, \
		&philosopher_routine, (void *)&philosophers[i]))
			return (true);
		i++;
	}
	return (false);
}

bool	finalize(t_philo *philosophers, t_data *data, int i)
{
	int		c;
	long	now;

	c = 0;
	while (c != data->num)
		philosophers[c++].state = DIE;
	if (pthread_mutex_lock(philosophers[i].print))
		return (true);
	now = gettime();
	if (now == ERROR || philosophers[i].ret == ERROR)
		return (true);
	printf("%ldms: philosopher %d died\n", now, philosophers[i].ret);
	if (pthread_mutex_unlock(philosophers[i].print))
		return (true);
	c = 0;
	while (c != data->num)
		pthread_join(philosophers[c++].thread, 0);
	return (false);
}

bool	wait_for_starve(t_philo *philosophers, t_data *data)
{
	long	i;

	i = 0;
	pthread_mutex_lock(&data->end);
	while (philosophers[i].ret == -2)
		i++;
	return (finalize(philosophers, data, i));
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
			ret = ERROR;
		i++;
	}
	if (pthread_mutex_destroy(philosophers[0].print))
		ret = ERROR;
	free(philosophers);
	return (ret);
}

bool initialize_data(t_data *data)
{
	pthread_mutex_init(&data->end, 0);
	pthread_mutex_lock(&data->end);
	pthread_mutex_init(&data->print, 0);
	return (false);
}

int main(int argc, char **argv)
{
	t_philo	*philosophers;
	t_data	data;

	if (argc != 5 && argc != 6)
	{
		printf(USAGE_MSG);
		printf("Wrong number of arguments");
		return (1);
	}
	if (parse_argv(&argv[1], &data.num, &data.time))
		return (error("Error parsing arguments", 2));
	initialize_data(&data);
	if (initialize_philosophers(&philosophers, \
	data.num, &data))
		return (error("Error initializing philosophers", 3));
	if (start_dinner(philosophers, data.num))
		return (error("Error starting the philosophers", 4));
	if (wait_for_starve(philosophers, &data))
		return (error("Error during the dinner", 5));
	if (free_all(philosophers, data.num))
		return (error("Error destroying mutexes", 6));
	return (0);
}
