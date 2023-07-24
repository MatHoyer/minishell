/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   last_pipe.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mhoyer <mhoyer@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 09:56:41 by mhoyer            #+#    #+#             */
/*   Updated: 2023/07/24 18:20:52 by mhoyer           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "exec.h"

void	parent_last(int pipefd[2][2])
{
	close_if(pipefd[0][0]);
	pipefd[0][0] = -1;
}

void	child_last(t_command *cmd, int pipefd[2][2])
{
	int	fdout;

	close_if(pipefd[0][1]);
	dup2(pipefd[0][0], STDIN_FILENO);
	close_if(pipefd[0][0]);
	if (cmd->has_append == false)
		fdout = open(cmd->file_out, O_WRONLY | O_TRUNC);
	else
		fdout = open(cmd->file_out, O_WRONLY | O_APPEND);
	dup2(fdout, STDOUT_FILENO);
	close(fdout);
}

int	execute_last(t_command *cmd, t_minishell *minishell, int pipefd[2][2])
{
	pid_t	pid;
	char	**env;

	pid = fork();
	if (pid == -1)
		return (1);
	ft_lstadd_back(&minishell->pids, ft_lstnew(&pid));
	if (pid == 0)
		child_last(cmd, pipefd);
	else
		parent_last(pipefd);
	if (pid == 0)
	{
		env = convert_env(minishell->env);
		if (!env)
			exec_failed(cmd, env, minishell);
		if (execve(cmd->command[0], cmd->command, env) == -1)
			exec_failed(cmd, env, minishell);
	}
	return (0);
}