/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   time_utils.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: lmuzio <lmuzio@student.codam.nl>             +#+                     */
/*                                                   +#+                      */
/*   Created: 2022/10/10 07:10:35 by lmuzio        #+#    #+#                 */
/*   Updated: 2022/10/10 07:10:35 by lmuzio        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <philosophers.h>

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
