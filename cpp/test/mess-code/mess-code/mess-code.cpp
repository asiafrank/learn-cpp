// mess-code.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>

int main()
{
	std::cout << "房租计算器" << std::endl;
	std::cout << "当前电费:";

	double electricityNum;
	std::cin >> electricityNum; 
	
	std::cout << "当前水费:";
	double waterNum;
	std::cin >> waterNum;

	std::cout << "上次电费:";

	double lastElectricityNum;
	std::cin >> lastElectricityNum;

	std::cout << "上次水费:";
	double lastWaterNum;
	std::cin >> lastWaterNum;

	double electicityFee = electricityNum - lastElectricityNum;
	double waterFee = (waterNum - lastWaterNum) * 4;
	
	double allFee = 8100 + electicityFee + waterFee;
	std::cout << "电费：" << electicityFee << "元" << std::endl;
	std::cout << "水费：" << waterFee << "元" << std::endl;
	std::cout << "房租：" << allFee << "元" << std::endl;

	// TODO: GMP 或 MPIR 实现
    return 0;
}
