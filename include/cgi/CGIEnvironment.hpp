/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jvittoz <marvin@42.fr>                     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/02 19:49:00 by jvittoz           #+#    #+#             */
/*   Updated: 2025/11/02 19:49:04 by jvittoz          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_ENVIRONMENT_HPP
#define CGI_ENVIRONMENT_HPP

#include <string>
#include <map>
#include <vector>

class Request;

class CGIEnvironment {
public:
	static std::map<std::string, std::string> prepare(const Request& req, const std::string& scriptPath);
	static std::vector<char*> createEnvArray(const std::map<std::string, std::string>& environment);

private:
	static void addBasicVariables(std::map<std::string, std::string>& env, const Request& req, const std::string& scriptPath);
	static void addUrlVariables(std::map<std::string, std::string>& env, const Request& req);
	static void addHttpHeaders(std::map<std::string, std::string>& env, const Request& req);
	static void addServerDefaults(std::map<std::string, std::string>& env);
};

#endif
