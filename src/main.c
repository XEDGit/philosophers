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
	int				num_meals;
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
	int				max_meals;
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
	if (now - data->eat_time >= data->time->die)
		return (true);
	if (update)
		data->eat_time = now;
	return (false);
}

int	msleep_starve(unsigned int ms, t_philo *data)
{
	int		ret;
	long	start;
	long	now;

	start = gettime();
	if (start == ERROR)
		return (ERROR);
	now = start;
	while (now - start < ms)
	{
		now = gettime();
		if (now == ERROR)
		return (ERROR);
		ret = check_starve(data, false);
		if (ret)
			return (ret);
		usleep(50);
	}
	return (false);
}

int	print_state(t_philo *data, bool fork)
{
	long		now;
	int			state;
	static char	*msg[4] = {
		"%ldms: philosopher %d is eating\n",
		"%ldms: philosopher %d is sleeping\n",
		"%ldms: philosopher %d is thinking\n",
		"%ldms: philosopher %d has taken a fork\n"
	};

	state = data->state;
	if (fork)
		state = 3;
	now = gettime();
	if (now == ERROR || pthread_mutex_lock(data->print))
		return (ERROR);
	printf(msg[state], now, data->id);
	if (!fork && data->state != DIE)
	{
		if (data->state < 2)
			data->state += 1;
		else
			data->state = 0;
	}
	if (pthread_mutex_unlock(data->print))
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
		if (check_starve(data, false) == true || data->state == DIE)
			return (STARVE);
		usleep(100);
	}
	if (pthread_mutex_lock(fork))
		return (ERROR);
	*fork_value = 1;
	if (pthread_mutex_unlock(fork) || \
	print_state(data, true))
		return (ERROR);
	return (SUCCESS);
}

bool	release_forks(t_philo *data)
{
	if (pthread_mutex_lock(&data->fork))
		return (true);
	data->fork_value = 0;
	if (pthread_mutex_unlock(&data->fork))
		return (true);
	if (pthread_mutex_lock(data->next))
		return (true);
	*data->next_value = 0;
	if (pthread_mutex_unlock(data->next))
		return (true);
	return (false);
}

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

bool	parse_argv(char **argv, t_data *data)
{
	int	i;

	i = 0;
	while (argv[i])
	{
		if (!ph_isdigit(argv[i++]))
			return (true);
	}
	data->num = ft_atoi(argv[0]);
	data->time.die = ft_atoi(argv[1]);
	data->time.eat = ft_atoi(argv[2]);
	data->time.sleep = ft_atoi(argv[3]);
	if (argv[4])
		data->max_meals = ft_atoi(argv[4]);
	else
		data->max_meals = -1;
	if (!data->max_meals || !data->num || !data->time.sleep || !data->time.eat || !data->time.die)
	{
		printf("Values should not be 0\n");
		return (true);
	}
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

	if (pthread_mutex_lock(philosophers[i].print))
		return (true);
	c = 0;
	while (c != data->num)
		philosophers[c++].state = DIE;
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
	int	i;

	i = 0;
	pthread_mutex_lock(&data->end);
	while (philosophers[i].ret == -2)
		i++;
	return (finalize(philosophers, data, i));
}

bool	finalize_meals(t_philo *philosophers, t_data *data)
{
	int		i;

	i = 0;
	if (pthread_mutex_lock(philosophers[0].print))
		return (true);
	while (i != data->num)
		philosophers[i++].state = DIE;
	if (pthread_mutex_unlock(philosophers[0].print))
		return (true);
	i = 0;
	while (i != data->num)
		pthread_join(philosophers[i++].thread, 0);
	return (false);
}

bool	wait_for_meals(t_philo *philosophers, t_data *data)
{
	int 	i;
	bool	end;

	while (true)
	{
		i = 0;
		end = true;
		while (i != data->num)
		{
			if (philosophers[i].num_meals < data->max_meals)
				end = false;
			if (philosophers[i].ret != -2)
				break ;
			i++;
		}
		if (end)
			return (finalize_meals(philosophers, data));
		if (i != data->num)
			return (finalize(philosophers, data, i));
		// usleep(10);
	}
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
	if (pthread_mutex_init(&data->end, 0) || \
	(data->max_meals == -1 && pthread_mutex_lock(&data->end)) || \
	pthread_mutex_init(&data->print, 0))
		return (true);
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
	if (parse_argv(&argv[1], &data))
		return (error("Error parsing arguments", 2));
	if (initialize_data(&data))
		return (error("Error initializing data", 3));;
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
