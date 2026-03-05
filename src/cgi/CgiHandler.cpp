/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 10:49:39 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 10:14:33 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/cgi/CgiHandler.hpp"
#include <wait.h>


/**
 * @brief Retrieves the binary path for executing a CGI script.
 *
 * Extracts the file extension from the script path in the request
 * and looks it up in the server's CGI configuration.
 *
 * @param req The HTTP request containing CGI information.
 * @return A dynamically allocated C-string of the binary path if found,
 *         or NULL if the extension is not configured.
 */
char*	CgiHandler::getBinaryPath(const Request& req)
{
	size_t dotPos = req.cgi.scriptPath.rfind('.');
	if (dotPos == std::string::npos)
		return NULL;

	std::string ext = req.cgi.scriptPath.substr(dotPos);
	std::map<std::string, std::string>::const_iterator it = req.config.cgi_conf.find(ext);

	if (it == req.config.cgi_conf.end())
		return NULL;

	return strdup(it->second.c_str());
}

/**
 * @brief Builds the argument vector for executing a CGI script.
 *
 * Constructs a null-terminated array containing:
 * - The interpreter binary path (e.g., /usr/bin/python)
 * - The script path
 * - A terminating NULL pointer
 *
 * @param req The HTTP request containing CGI script information.
 * @return A dynamically allocated char** array of arguments, or NULL if no interpreter is found.
 */
char**	CgiHandler::buildArguments(const Request& req)
{
	char* interpreter = getBinaryPath(req);
	if (!interpreter)
		return NULL;

	char** argv = new char*[3];
	argv[0] = interpreter;
	argv[1] = strdup(req.cgi.scriptPath.c_str());
	argv[2] = NULL;

	return argv;
}

/**
 * @brief Builds the environment variables array for a CGI script.
 *
 * Converts the HTTP request data into a null-terminated array of
 * environment variable strings in the format KEY=VALUE.
 * Includes standard CGI variables, query parameters, and headers.
 *
 * @param req The HTTP request containing CGI data.
 * @return A dynamically allocated char** array of environment variables.
 */
char**	CgiHandler::buildEnvVariables(const Request& req)
{
	std::vector<std::string> envVect;

	envVect.push_back("REQUEST_METHOD=" + req.cgi.method);
	envVect.push_back("SCRIPT_NAME=" + req.cgi.scriptPath);
	envVect.push_back("SERVER_PROTOCOL=HTTP/1.1");
	envVect.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envVect.push_back("SERVER_SOFTWARE=webserv/1.0");
	envVect.push_back("SERVER_NAME=" + req.cgi.host);
	envVect.push_back("SERVER_PORT=" + req.cgi.port);

	if (!req.cgi.query.empty())
		envVect.push_back("QUERY_STRING=" + req.cgi.query);

	if (!req.cgi.pathInfo.empty())
		envVect.push_back("PATH_INFO=" + req.cgi.pathInfo);

	if (req.cgi.method == "POST")
	{
		envVect.push_back("CONTENT_TYPE=" + req.cgi.contentType);
		envVect.push_back("CONTENT_LENGTH=" + toString(req.cgi.contentLength));
	}

	std::map<std::string, std::string>				headers = req.cgi.headers;
	std::map<std::string, std::string>::iterator	it = headers.begin();
	for (; it != headers.end(); ++it)
	{
		std::string	key = "HTTP_" + it->first;
		std::replace(key.begin(), key.end(), '-', '_');
		envVect.push_back(key + "=" + it->second);
	}

	char**	envp = new char*[envVect.size() + 1];
	for (size_t i = 0; i < envVect.size(); i++)
	{
		envp[i] = strdup(envVect[i].c_str());
	}
	envp[envVect.size()] = NULL;
	
	return (envp);
}

/**
 * @brief Executes a CGI script in the current process.
 *
 * Redirects standard input and output to the provided pipe ends,
 * then calls execve() with the given arguments and environment variables.
 *
 * @param stdinReadEnd File descriptor to use as STDIN for the CGI script.
 * @param stdoutWriteEnd File descriptor to use as STDOUT for the CGI script.
 * @param argv Null-terminated array of arguments for execve().
 * @param envp Null-terminated array of environment variables for execve().
 *
 * @throws Runtime error if execve() fails. Exits with code 127 after failure.
 */
void	CgiHandler::executeScript(int stdinReadEnd, int stdoutWriteEnd, char **argv, char **envp)
{
	dup2(stdinReadEnd, STDIN_FILENO);
	dup2(stdoutWriteEnd, STDOUT_FILENO);

	close(stdoutWriteEnd);
	close(stdinReadEnd);
	
	execve(argv[0], argv, envp);

	throwError("execve()");
	_exit(127);
}

/**
 * @brief Starts a CGI script as a child process.
 *
 * Creates pipes for stdin and stdout, builds the argument and environment
 * arrays, forks a child, and executes the CGI script. For POST requests,
 * writes the request body to the script's stdin.
 *
 * @param req The HTTP request containing CGI information.
 * @param outPid Reference to store the child process ID.
 * @return The read-end file descriptor of the script's stdout pipe, or -1 on failure.
 */
int	CgiHandler::startCgiScript(const Request& req, pid_t& outPid)
{
	int stdinPipe[2];
	int stdoutPipe[2];
	char **argv = NULL;
	char **envp = NULL;

	if (pipe(stdinPipe) < 0)
		throwError("pipe(stdin)");

	if (pipe(stdoutPipe) < 0)
		throwError("pipe(stdout)");

	setNonBlocking(stdoutPipe[0]);
		
	argv = buildArguments(req);
	if (!argv)
	{
		cleanUpPipes(stdinPipe, stdoutPipe);
		return (-1);
	}
	envp = buildEnvVariables(req);
	
	pid_t	pid = fork();
	if (pid < 0)
	{
		cleanUpPipes(stdinPipe, stdoutPipe);
		throwError("fork()");
	}
	
	if (pid == 0)
	{
		close(stdinPipe[1]);
		close(stdoutPipe[0]);
		executeScript(stdinPipe[0], stdoutPipe[1], argv, envp);
		_exit(127);
	}
	close(stdinPipe[0]);
	close(stdoutPipe[1]);

	if (req.cgi.method == "POST")
	{
		write(stdinPipe[1], req.cgi.body.c_str(), req.cgi.body.size());
	}
	close(stdinPipe[1]);

	cleanUpArguments(argv);
	cleanUpEnvVariables(envp);

	outPid = pid;
	return (stdoutPipe[0]);
}

/**
 * @brief Checks the exit status of a CGI child process without blocking.
 *
 * Uses waitpid with WNOHANG to check if the process has terminated.
 *
 * @param pid The child process ID to check.
 * @param exitStatus Reference to store the exit status if finished.
 * @return true if the process has exited, false otherwise.
 */
bool	CgiHandler::checkCgiStatus(pid_t pid, int& exitStatus)
{
	int	status;

	pid_t	result = waitpid(pid, &status, WNOHANG);

	if (result == 0)
		return (false);
	else if (result > 0)
	{
		exitStatus = WEXITSTATUS(status);
		return (true);
	}
	// case error
	return (false);
}

/**
 * @brief Reads a chunk of data from a CGI stdout pipe.
 *
 * @param pipeFd The file descriptor to read from.
 * @param buffer Pointer to the buffer where data will be stored.
 * @param size Maximum number of bytes to read.
 * @return Number of bytes read, or -1 on error.
 */
ssize_t	CgiHandler::readChunk(int pipeFd, char *buffer, size_t size)
{
	return (read(pipeFd, buffer, size));
}

/**
 * @brief Frees memory allocated for the argument array of a CGI script.
 *
 * @param argv Null-terminated array of arguments to clean up.
 */
void	CgiHandler::cleanUpArguments(char **argv)
{
	if (!argv)
		return ;
	for (int i = 0; argv[i] != NULL; i++)
		free(argv[i]);
	delete[] argv;
}

/**
 * @brief Frees memory allocated for the environment variables array of a CGI script.
 *
 * @param envp Null-terminated array of environment variables to clean up.
 */
void	CgiHandler::cleanUpEnvVariables(char **envp)
{
	if (!envp)
		return ;
	
	for (int i = 0; envp && envp[i] != NULL; i++)
		free(envp[i]);
	delete[] envp;
}

/**
 * @brief Closes the stdin and stdout pipes used for a CGI script.
 *
 * @param stdinPipe Array of two file descriptors for stdin (read and write).
 * @param stdoutPipe Array of two file descriptors for stdout (read and write).
 */
void	CgiHandler::cleanUpPipes(int stdinPipe[2], int stdoutPipe[2])
{
	close(stdinPipe[0]);
	close(stdinPipe[1]);
	close(stdoutPipe[0]);
	close(stdoutPipe[1]);
}