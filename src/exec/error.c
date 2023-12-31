/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouvier <ebouvier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/22 11:04:11 by mhoyer            #+#    #+#             */
/*   Updated: 2023/09/02 16:49:13 by ebouvier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "exec.h"

void	free_command(t_command *cmd)
{
	free_mat(cmd->command);
	free(cmd);
}

void	free_minishell(t_minishell *minishell)
{
	rl_clear_history();
	ast_delete(minishell->root);
	ft_lstclear(&minishell->pids, nothing);
	ft_lstclear_env(&minishell->env, free);
	close_if(minishell->m_fd[0]);
	close_if(minishell->m_fd[1]);
}

int	msg_error(char *str, char *error, char *type)
{
	if (type)
		ft_dprintf(2, "minishell: %s: %s: %s\n", type, error, str);
	else
		ft_dprintf(2, "minishell: %s: %s\n", error, str);
	return (1);
}

void	print_msg_err(t_command *cmd)
{
	struct stat	file_info;

	file_info.st_mode = 0;
	stat(cmd->command[0], &file_info);
	cmd->error = ERR_NOT_FOUND;
	if (cmd->has_path == false)
	{
		msg_error("command not found", cmd->command[0], NULL);
		cmd->error = ERR_NOT_FOUND;
	}
	else if (S_ISDIR(file_info.st_mode))
	{
		msg_error("Is a directory", cmd->command[0], NULL);
		cmd->error = ERR_IS_DIR;
	}
	else
	{
		if (access(cmd->command[0], F_OK) == -1)
			msg_error("No such file or directory", cmd->command[0], NULL);
		else
		{
			msg_error("Permission denied", cmd->command[0], NULL);
			cmd->error = ERR_IS_DIR;
		}
	}
}

void	exec_failed(t_command *cmd, char **env, t_minishell *minishell,
		int status)
{
	int	err;

	if ((cmd->builtin && cmd->has_path == true) || cmd->builtin == NONE)
		print_msg_err(cmd);
	err = cmd->error;
	free_mat(env);
	free_minishell(minishell);
	if (cmd->builtin)
	{
		free_command(cmd);
		if (status == 1)
			exit(1);
		exit(0);
	}
	free_command(cmd);
	exit(err);
}
