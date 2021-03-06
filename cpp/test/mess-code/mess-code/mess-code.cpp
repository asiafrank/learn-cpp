// mess-code.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>

class Config
{
public:
	Config();
	virtual ~Config();
};

Config::Config()
{
	std::cout << "config constructed" << std::endl;
}

Config::~Config()
{
	std::cout << "config descruted" << std::endl;
}

class BConfig : public Config
{
public:
	BConfig();
	~BConfig();
};

BConfig::BConfig()
{
	std::cout << "bconfig constructed" << std::endl;
}

BConfig::~BConfig()
{
	std::cout << "bconfig descruted" << std::endl;
}

int main()
{
	Config *c = new BConfig();
	delete c;
    return 0;
}

