#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

err(char *str)
{
	while (*str)
		write (2, str++, 1);
	return (1);
}

int	cd(int ac, char **av)
{
	if (ac != 2)
		return (err("error: cd: bad arguments\n"));
	if (chdir(av[1]) == -1)
		return (err("error: cd: cannot change directory to"), err(av[1]), err("\n"));
	return (0);
}
// Función para ejecutar comandos
int	exec(int i, char **av, char **envp)
{
	int fd[2];           // Array para los descriptores de archivo de la tubería
	int status;          // Variable para almacenar el estado del proceso hijo
	int has_pipe = av[i] && !strcmp(av[i], "|");  // Verifica si el comando actual tiene una tubería

	// Si no hay tubería y el comando es 'cd', llama a la función cd
	if (!has_pipe && !strcmp(*av, "cd"))
		return cd(av, i);
	// Si hay tubería, crea la tubería
	if (has_pipe && pipe(fd) == -1)
		return err("error: fatal\n");

	// Crea un proceso hijo
	int pid = fork();
	if (!pid)
	{
        av[i] = 0;  // Termina la lista de argumentos en el índice 'i' poniendo NULL
        // Si hay tubería, redirige la salida estándar al extremo de escritura de la tubería
        if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
            return err("error: fatal\n");
        // Si el comando es 'cd', llama a la función cd
        if (!strcmp(*av, "cd"))
            return cd(av, i);
        // Ejecuta el comando especificado en av[0] con sus argumentos y el entorno envp
        execve(*av, av, envp);
        // Si execve falla, imprime un mensaje de error y el comando que no se pudo ejecutar
        return err("error: cannot execute "), err(*av), err("\n");
    }
	// Espera a que el proceso hijo termine
	waitpid(pid, &status, 0);

	// Si hay tubería, redirige la entrada estándar al extremo de lectura de la tubería
	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return err("error: fatal\n");
	// Devuelve el estado del proceso hijo
	return (WIFEXITED(status) && WEXITSTATUS(status));
}
//int	executor(int ac, char **av, char **envp);

int	main(int ac, char **av, char **envp)
{
	int	status = 0;
	int	i = 0;

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