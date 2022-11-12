/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   philo_utils.c                                      :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/10 07:11:26 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/10 07:11:26 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

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

static void	update_state(int *state)
{
	if (*state < 2)
		*state += 1;
	else
		*state = 0;
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
	if (pthread_mutex_lock(data->print))
		return (ERROR);
	now = gettime();
	if (now == ERROR)
		return (ERROR);
	if (data->state != DIE)
		printf(msg[state], now, data->id);
	if (!fork && data->state != DIE)
		update_state(&data->state);
	if (pthread_mutex_unlock(data->print))
		return (ERROR);
	return (SUCCESS);
}
