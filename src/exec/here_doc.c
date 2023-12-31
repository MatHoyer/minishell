/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   here_doc.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouvier <ebouvier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/23 09:51:32 by mhoyer            #+#    #+#             */
/*   Updated: 2023/09/04 14:50:42 by ebouvier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "exec.h"
#include "minishell.h"
#include "signals.h"

int	str_cmpend(char *line, char *limiter)
{
	int	i;

	i = 0;
	while (line[i] == limiter[i] && line[i] && limiter[i])
		i++;
	if (line[i] == '\0' && limiter[i] == '\0')
		return (0);
	return (1);
}

t_bool	check_limiter(char *line, char *limiter)
{
	if (!line)
		return (true);
	if (str_cmpend(line, limiter) == 0)
	{
		free(line);
		return (true);
	}
	return (false);
}

void	here_doc(char *limiter)
{
	int		fd;
	char	*line;

	fd = open(FILE_HEREDOC, O_WRONLY | O_TRUNC | O_CREAT, 0644);
	if (fd == -1)
		return ;
	line = NULL;
	signal(SIGINT, sig_handler_here_doc);
	while (1)
	{
		if (g_sigint == 1)
		{
			close_if(fd);
			unlink(FILE_HEREDOC);
			return ;
		}
		line = readline("> ");
		if (check_limiter(line, limiter))
			break ;
		ft_putendl_fd(line, fd);
		free(line);
	}
	close_if(fd);
}
