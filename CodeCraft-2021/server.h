//
// Created by Lan Haoning on 2021/3/15.
//

#ifndef CODECRAFT_2021_SERVER_H
#define CODECRAFT_2021_SERVER_H

#include "string"
#include "unordered_map"
#include "virtual_machine.h"
#include "set"

using namespace std;

// 定义结构体用于存储服务器信息
struct ServerInfo
{
	// 服务器型号
	std::string serverType;
	// A点CPU核数
	int cpuCoresA;
	// A点内存大小
	int memorySizeA;
	// B点CPU核数
	int cpuCoresB;
	// B点内存大小
	int memorySizeB;
	// 购买整机消耗
	int serverCost;
	// 用户购买一天的价格
	int powerCost;
	ServerInfo() {}

	ServerInfo(std::string _serverType, int _cpuCoresA, int _memorySizeA,
				int _cpuCoresB, int _memorySizeB, int _serverCost, int _powerCost) :
	serverType(_serverType), cpuCoresA(_cpuCoresA), cpuCoresB(_cpuCoresB), 
	memorySizeA(_memorySizeA), memorySizeB(_memorySizeB),
	serverCost(_serverCost), powerCost(_powerCost) {}

	ServerInfo(const ServerInfo &rhs)
	{
		serverType = rhs.serverType;
		cpuCoresA = rhs.cpuCoresA;
		memorySizeA = rhs.memorySizeA;
		cpuCoresB = rhs.cpuCoresB;
		memorySizeB = rhs.memorySizeB;
		serverCost = rhs.serverCost;
		powerCost = rhs.powerCost;
	}
	ServerInfo& operator=(const ServerInfo &rhs)
	{
		serverType = rhs.serverType;
		cpuCoresA = rhs.cpuCoresA;
		memorySizeA = rhs.memorySizeA;
		cpuCoresB = rhs.cpuCoresB;
		memorySizeB = rhs.memorySizeB;
		serverCost = rhs.serverCost;
		powerCost = rhs.powerCost;
		return *this;
	}

	bool operator<(const ServerInfo &rhs)
	{
		int tmpCmp1 = this->serverCost / (this->cpuCoresA + this->cpuCoresB) + 
			this->serverCost / (this->memorySizeA + this->memorySizeB) +
			this->powerCost;
		int tmpCmp2 = rhs.serverCost / (rhs.cpuCoresA + rhs.cpuCoresB) + 
			rhs.serverCost / (rhs.memorySizeA + rhs.memorySizeB) +
			rhs.powerCost;
		return tmpCmp1 < tmpCmp2;
	}

	void swap(ServerInfo &rhs)
	{
		std::swap(this->serverType, rhs.serverType);
		std::swap(this->cpuCoresA, rhs.cpuCoresA);
		std::swap(this->memorySizeA, rhs.memorySizeA);
		std::swap(this->cpuCoresB, rhs.memorySizeB);
		std::swap(this->memorySizeB, rhs.memorySizeB);
		std::swap(this->serverCost, rhs.serverCost);
		std::swap(this->powerCost, rhs.powerCost);
	}
};

#endif //CODECRAFT_2021_SERVER_H
