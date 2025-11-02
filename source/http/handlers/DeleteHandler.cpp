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

#include "../../../include/http/handlers/DeleteHandler.hpp"
#include <cstdio>

Response DeleteHandler::handleDelete(const std::string& filePath) {
	Response resp;
	
	if (std::remove(filePath.c_str()) != 0) {
		resp.setStatus(500);
		resp.setErrorBody(500);
	} else {
		resp.setStatus(204);
		resp.setBody("");
	}
	
	return resp;
}
