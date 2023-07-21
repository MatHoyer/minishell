/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ebouvier <ebouvier@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/07/14 11:37:55 by mhoyer            #+#    #+#             */
/*   Updated: 2023/07/21 14:52:30 by ebouvier         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "exec.h"
#include "minishell.h"
#include <stdio.h>

void	close_if(int fd)
{
	if (fd >= 0)
		close(fd);
}

int	nb_node(t_node *node)
{
	int	nb;

	nb = 0;
	if (!node)
		return (1);
	return (nb + nb_node(node->left) + nb_node(node->right));
}
t_command	*node_to_command(t_node *node, char **env)
{
	t_command	*command;

	command = malloc(sizeof(t_command));
	if (!command)
		return (NULL); // free all exit
	command->command = get_cmd(node->raw_command, env);
	command->fd_in = -1;
	command->fd_out = -1;
	return (command);
}

void	execute_first(t_command *cmd, char **env, int pipefd[2][2])
{
	pid_t	pid;
	int		fdin;

	pid = fork();
	if (pid == 0)
	{
		fdin = open("/dev/stdin", O_RDONLY);
		dup2(fdin, STDIN_FILENO);
		close(fdin);
		close_if(pipefd[1][0]);
		dup2(pipefd[1][1], STDOUT_FILENO);
		close_if(pipefd[1][1]);
	}
	else
	{
		close_if(pipefd[0][0]);
		pipefd[0][0] = pipefd[1][0];
		pipefd[1][0] = -1;
		close_if(pipefd[1][1]);
	}
	if (pid == 0)
	{
		if (execve(cmd->command[0], cmd->command, env) == -1)
			printf("exitlast");
	}
}

void	execute_middle(t_command *cmd, char **env, int pipefd[2][2])
{
	pid_t	pid;

	pid = fork();
	if (pid == 0)
	{
		close_if(pipefd[0][1]);
		dup2(pipefd[0][0], STDIN_FILENO);
		close_if(pipefd[0][0]);
		close_if(pipefd[1][0]);
		dup2(pipefd[1][1], STDOUT_FILENO);
		close_if(pipefd[1][1]);
	}
	else
	{
		close_if(pipefd[0][0]);
		pipefd[0][0] = pipefd[1][0];
		pipefd[1][0] = -1;
		close_if(pipefd[1][1]);
	}
	if (pid == 0)
	{
		if (execve(cmd->command[0], cmd->command, env) == -1)
			printf("exitmiddle");
	}
}

void	execute_last(t_command *cmd, char **env, int pipefd[2][2])
{
	pid_t	pid;
	int		fdout;

	pid = fork();
	if (pid == 0)
	{
		close_if(pipefd[0][1]);
		dup2(pipefd[0][0], STDIN_FILENO);
		close_if(pipefd[0][0]);
		fdout = open("/dev/stdout", O_WRONLY);
		dup2(fdout, 1);
		close(fdout);
	}
	else
	{
		close_if(pipefd[0][0]);
		pipefd[0][0] = -1;
	}
	if (pid == 0)
	{
		if (execve(cmd->command[0], cmd->command, env) == -1)
			printf("exitlast");
	}
}

void	init_pipes(int pipefd[2][2])
{
	pipefd[0][0] = -1;
	pipefd[0][1] = -1;
	pipefd[1][0] = -1;
	pipefd[1][1] = -1;
}

void	execute_pipeline(t_node *root, t_node *node, char **env,
		int pipefd[2][2])
{
	t_command	*command;

	if (!node)
		return ;
	if (node->type == PIPE)
	{
		execute_pipeline(root, node->left, env, pipefd);
		execute_pipeline(root, node->right, env, pipefd);
	}
	if (node->type == COMMAND)
	{
		command = node_to_command(node, env);
		if (node->parent && node->parent->type == PIPE
			&& node->parent->left == node)
		{
			pipe(pipefd[1]);
			printf("first : (%s)\n", node->raw_command);
			execute_first(command, env, pipefd);
		}
		else if (node->parent && node->parent->type == PIPE
				&& node->parent->right == node && node->parent != root)
		{
			pipe(pipefd[1]);
			printf("middle : (%s)\n", node->raw_command);
			execute_middle(command, env, pipefd);
		}
		else if (node->parent && node->parent == root
				&& node->parent->right == node)
		{
			printf("last : (%s)\n", node->raw_command);
			execute_last(command, env, pipefd);
		}
	}
}

void	wait_all(int nb_cmd)
{
	int	i;

	i = 0;
	while (i < nb_cmd)
	{
		wait(NULL);
		i++;
	}
}

void	exec(t_node *node, t_minishell *minishell)
{
	int	pipefd[2][2];

	init_pipes(pipefd);
	// t_command	*command;
	if (!node)
		return ;
	if (node->type == PIPE)
		execute_pipeline(node, node, convert_env(minishell->env), pipefd);
	// else if (node->type == COMMAND)
	// {
	// 	command = node_to_command(node, convert_env(minishell->env));
	// 	execute_command(command, convert_env(minishell->env));
	// }
	// if (node->token->type == T_PIPE)
	// {
	//     printf("(((T_PIPE : %s)))\n", node->token->value);
	//     pipe(fd_pipe);
	//     pid = fork();
	//     if (pid == -1)
	//         exit(1);
	//     if (pid == 0)
	//     {
	//         dup2(fd_pipe[1], 1);
	//         close(fd_pipe[0]);
	//         close(fd_pipe[1]);
	//         exec(node->left);
	//     }
	//     pid = fork();
	//     if (pid == -1)
	//         exit(1);
	//     if (pid == 0)
	//     {
	//         dup2(fd_pipe[0], 0);
	//         close(fd_pipe[0]);
	//         close(fd_pipe[1]);
	//         exec(node->right);
	//     }
	//     close(fd_pipe[0]);
	//     close(fd_pipe[1]);
	// }
	// else if (node->token->type == T_OR)
	// {
	//     printf("(((T_OR : %s)))\n", node->token->value);
	//     exec(node->left);
	// }
	// else if (node->token->type == T_AND)
	// {
	//     printf("(((T_AND : %s)))\n", node->token->value);
	//     exec(node->left);
	//     exec(node->right);
	// }
	// else if (node->token->type == T_WORD)
	// {
	//     printf("(((T_WORD : %s)))\n", node->token->value);
	//     pid = fork();
	//     if (pid == -1)
	//         exit(1);
	//     if (pid == 0)
	//     {
	//         char **splited;
	//         splited = ft_split(node->token->value, " ");
	//         if (execve(ft_strjoin("/usr/bin/", splited[0]), splited, NULL) ==
	// -1)
	//         {
	//             exit (1);
	//         }
	//     }
	// }
}

void	prep_exec(t_node *node, t_minishell *minishell)
{
	// minishell->stdin = dup(0);
	// minishell->stdout = dup(1);
	exec(node, minishell);
	wait_all(3);
}