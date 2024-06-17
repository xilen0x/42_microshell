#include "microshell.h"

int	put_std_err(char *str)
{
	while (*str)
		write (2, str++, 1);
	return (1);
}

int	cd(int ac, char **av)
{
	if (ac != 2)
		return (put_std_err("error: cd: bad arguments\n"));
	if (chdir(av[1]) == -1)
		return (put_std_err("error: cd: cannot change directory to "), put_std_err(av[1]), put_std_err("\n"));
	return (0);
}
int executor(int i, char **av, char **envp)
{
	int	has_pipe;
	int	fd[2];
	int pid;
	int status;
	
	has_pipe = av[i] && !strcmp(av[i], "|");
	if(!has_pipe && !strcmp(*av, "cd"))
		return (cd(i, av));
	if(has_pipe && pipe(fd) == -1)
		return (put_std_err("error: fatal\n"));
	pid = fork();
	if(!pid)
	{
		av[i] = 0;
		if(has_pipe && (dup2(fd[1], STDOUT_FILENO) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return (put_std_err("error: fatal\n"));
		if(!strcmp(*av, "cd"))
			return (cd(i, av));
		execve(*av, av, envp);
		return (put_std_err("error: cannot execute "), put_std_err(*av), put_std_err("\n"));
	}
	waitpid(pid, &status, 0);
	if(has_pipe && (dup2(fd[0], STDIN_FILENO) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return (put_std_err("error: fatal\n"));
	return (WIFEXITED(status) && WEXITSTATUS(status));
}
int	main(int ac, char **av, char **envp)
{
	int status = 0;
	int i = 0;

	if (ac > 1)
	{
		while (av[i] && av[++i])
		{
			av += i;
			i = 0;
			while (av[i] && (strcmp(av[i], "|") && strcmp(av[i], ";")))
				i++;
			if (i)
				status = executor(i, av, envp);			
		}
	}
	return (status);
}