/*
 * ConfigRead.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: mcn
 */

#include "ConfigRead.h"
#include <string>
#include <map>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>

ConfigRead::ConfigRead(char* file)
{
	std::string line;
	std::ifstream is(file);   // open file

	while (!is.eof()){
		std::getline(is,line);
		if(line.length()==0) continue;
		std::istringstream singleLine(line);
		std::string entry;
		singleLine >> entry;
		if(entry.length()==0) continue;
		assert(entry.back()==':');
		entry.pop_back();
		dict[entry] = std::vector<std::string>();
		while(!singleLine.eof()){
			std::string value;
			singleLine >> value;
			dict[entry].push_back(value);
		}
	}
	is.close();
}

std::vector<std::string> ConfigRead::read_vector(std::string entry){
	assert(dict.count(entry)>0 );
	return (dict[entry]);
}

std::string ConfigRead::read_single(std::string entry){
	assert(dict.count(entry)>0 && dict[entry].size() > 0);
	return (dict[entry])[0];
}

int ConfigRead::read_single_i(std::string entry){
	assert(dict.count(entry)>0 && dict[entry].size() > 0);
	return std::stoi((dict[entry])[0]);
}
