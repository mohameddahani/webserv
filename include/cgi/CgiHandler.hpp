/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mait-all <mait-all@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/19 10:43:47 by mait-all          #+#    #+#             */
/*   Updated: 2026/02/25 10:25:46 by mait-all         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "CgiUtils.hpp"
#define CGI_TIMEOUT 20 // 20s


// ************************************************************************** //
//                               CgiHandler Class                             //
// ************************************************************************** //

class CgiHandler : public Helper {

	public:
		
		char**	buildArguments(const Request& req);
		char**	buildEnvVariables(const Request& req);
		char*	getBinaryPath(const Request& req);
		void	cleanUpArguments(char **argv);
		void	cleanUpEnvVariables(char **envp);
		void	cleanUpPipes(int stdinPipe[2], int stdoutPipe[2]);
		void	executeScript(int stdinReadEnd, int stdoutWriteEnd, char **argv, char **envp);
		int		startCgiScript(const Request& req, pid_t& outPid);
		ssize_t	readChunk(int pipeFd, char *buffer, size_t size);
		bool	checkCgiStatus(pid_t pid, int& exitStatus);

};