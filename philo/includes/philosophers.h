/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   philosophers.h                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/12 05:48:26 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/12 05:48:26 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef PHILOSOPHERS_H
# define PHILOSOPHERS_H

# include <stdio.h>
# include <stdlib.h>
# include <pthread.h>
# include <unistd.h>
# include <stdbool.h>
# include <sys/time.h>

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
	bool			meal_mode;
	long			eat_time;
	t_times			*time;
	pthread_t		thread;
	pthread_mutex_t	state_lock;
	pthread_mutex_t	fork;
	int				fork_value;
	pthread_mutex_t	*next;
	int				*next_value;
	pthread_mutex_t	*print;
	pthread_mutex_t	*end_lock;
	int				*end;
}	t_philo;

typedef struct s_data
{
	t_times			time;
	pthread_mutex_t	print;
	pthread_mutex_t	end_lock;
	int				end;
	int				num;
	int				max_meals;
}	t_data;

# define STARVE 4
# define DIE 5
# define ERROR -1
# define SUCCESS 0
# define USAGE_MSG "Usage:\t./philosophers philosophers_amount time_to_die time_to_eat time_to_sleep \
[number_of_times_each_philosopher_must_eat]\n"

bool	parse_argv(char **argv, t_data *data);
bool	initialize_data(t_data *data);
bool	init_philosophers(t_philo	**philosophers, int num, \
t_data *data);
bool	start_dinner(t_philo *philosophers, int num);
bool	wait_for_starve(t_philo *philosophers, t_data *data);
bool	wait_for_meals(t_philo *philosophers, t_data *data);
int		free_all(t_philo *philosophers, int num);

long	gettime(void);
void	*philosopher_routine(void *arg);
int		take_fork(int i, t_philo *data);
bool	release_forks(t_philo *data);
int		print_state(t_philo *data, bool fork);
int		check_starve(t_philo *data, int update);
int		ph_first_routine(t_philo *data);
int		philosopher_dispatch(t_philo *data);
int		msleep_starve(unsigned int ms, t_philo *data);
int		terminate_philo(t_philo *data, int starve);

#endif
