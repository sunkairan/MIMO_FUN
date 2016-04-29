/*
 * ConfigRead.h
 *
 *  Created on: Apr 19, 2016
 *      Author: mcn
 */

#ifndef CONFIGREAD_H_
#define CONFIGREAD_H_

#include <string>
#include <map>
#include <vector>

class ConfigRead {
public:
	ConfigRead(char* file);
	std::vector<std::string> read_vector(std::string entry);
	std::string read_single(std::string entry);
	int read_single_i(std::string entry);
private:
	std::map<std::string, std::vector<std::string>> dict;
};

#endif /* CONFIGREAD_H_ */
