#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <time.h>
#include <algorithm>
#include <assert.h>

#include "server.h"
#include "virtual_machine.h"
#include "command.h"

#define DEBUG_TYPE 1 // 本地debug设为1，提交服务器改为0

// 原型声明
void getInput();
void process();
void getOutput();
void Reallocation(
	vector<int> &RunningID, //当天的增加的虚拟机以及服务被清空的服务器上的虚拟机
	int day,					//第i天
	vector<int> &purchaseOrderToday,
	unordered_map<int, int> &purchaseHashToday,
	vector<std::string> &tmpRes,
	unordered_map<int, int> &purchaseToday,
	int &purchaseCntToday
);

void Compare_with_yesterday(unordered_map<int, int> &vmIdToServerIdYesterday,
							unordered_map<int, int> &vmIdToServerIdToday,
							unordered_map<int, int> &vmIdToEndYesterday,
							unordered_map<int, int> &vmIdToEndToday,
							int &migrationCntToday, vector<int> &migrationOrderToday,
							unordered_map<int, pair<int, int>> &migrationHashToday);
// 全局变量
std::vector<ServerInfo> serverInfos;			   //所有服务器原型信息
std::unordered_map<string, int> serverNametoIndex; //所有服务器名字和其在serverINfo中的位置
std::unordered_map<std::string, VmInfo> vmInfos;   //所有虚拟机原型信息
std::vector<std::vector<Command>> commands;		   // 所有命令

std::vector<std::vector<int>> totalResources; // 每一天需要的 A 节点的资源和 B 节点的资源总数

std::unordered_map<int, ServerInfo> purchaseInfos; // 定义已经购买的服务器的ID与服务器的哈希表
std::unordered_map<int, std::string> vmIdToType;   // 定义虚拟机ID与虚拟机名称的哈希表
std::unordered_map<int, int> vmIdToServerId;	   // 定义虚拟机ID与服务器ID对应的哈希表
std::unordered_map<int, int> vmIdToEnd;			   // 定义虚拟机部署A与B节点对应的哈希表

int serverNum; // 服务器种类数
int vmNum;	   // 虚拟机种类数
int dayNum;	   // 总天数

std::vector<std::string> res; //要输出的命令序列

// 生成服务器信息
void generateServer(std::string &serverType, std::string &cpuCores, std::string &memorySize,
					std::string &serverCost, std::string &powerCost)
{
	std::string _serverType = "";
	for (int i = 1; i < serverType.size() - 1; ++i)
	{
		_serverType += serverType[i];
	}

	int _cpuCores = 0, _memorySize = 0, _serverCost = 0, _powerCost = 0;
	for (int i = 0; i < cpuCores.size() - 1; ++i)
	{
		_cpuCores = 10 * _cpuCores + cpuCores[i] - '0';
	}

	for (int i = 0; i < memorySize.size() - 1; ++i)
	{
		_memorySize = 10 * _memorySize + memorySize[i] - '0';
	}

	for (int i = 0; i < serverCost.size() - 1; ++i)
	{
		_serverCost = 10 * _serverCost + serverCost[i] - '0';
	}

	for (int i = 0; i < powerCost.size() - 1; ++i)
	{
		_powerCost = 10 * _powerCost + powerCost[i] - '0';
	}
	// 原地构造
	ServerInfo tmpInfo(_serverType, _cpuCores / 2, _memorySize / 2,
					   _cpuCores / 2, _memorySize / 2, _serverCost, _powerCost);
	serverInfos.emplace_back(tmpInfo);
	serverNametoIndex[_serverType] = serverInfos.size() - 1;
}

// 生成虚拟机信息
void generateVm(std::string &vmType, std::string &cpuCores, std::string &memorySize, std::string &isDouble)
{
	std::string _vmType = "";
	// type样式 (xxxxxx,
	for (int i = 1; i < vmType.size() - 1; ++i)
	{
		_vmType += vmType[i];
	}

	int _cpuCores = 0, _memorySize = 0;
	bool _isDouble = 0;

	for (int i = 0; i < cpuCores.size() - 1; ++i)
	{
		_cpuCores = 10 * _cpuCores + cpuCores[i] - '0';
	}

	for (int i = 0; i < memorySize.size() - 1; ++i)
	{
		_memorySize = 10 * _memorySize + memorySize[i] - '0';
	}

	_isDouble = isDouble[0] - '0';

	vmInfos[_vmType] = VmInfo{_cpuCores, _memorySize, _isDouble};
}

// 生成命令信息
void generateCommand(int day, std::string &commandType, std::string &vmType, std::string &vmId)
{
	int _commandType = 0, _vmId = 0;
	std::string _vmType = "";
	if (commandType[1] == 'a')
		_commandType = 1;
	else
		_commandType = 0;

	if (vmType.size())
		for (int i = 0; i < vmType.size() - 1; ++i)
		{
			_vmType += vmType[i];
		}

	for (int i = 0; i < vmId.size() - 1; ++i)
	{
		_vmId = 10 * _vmId + vmId[i] - '0';
	}
	// 添加虚拟机Id 与 Type哈希表
	if (_commandType)
		vmIdToType[_vmId] = _vmType;
	commands[day].emplace_back(_commandType, _vmType, _vmId);
}

//获取输入
void getInput()
{
	if (DEBUG_TYPE == 1)
	{
		// for debug
		std::string filePath = "../training-data/training-1.txt";
		// 重定向输入到标准输入
		std::freopen(filePath.c_str(), "rb", stdin);
	}
	// 输入服务器信息
	std::string tmp;
	std::string serverType, cpuCores, memorySize, serverCost, powerCost;
	scanf("%d", &serverNum);
	for (int i = 0; i < serverNum; i++)
	{
		std::cin >> serverType >> cpuCores >> memorySize >> serverCost >> powerCost;
		generateServer(serverType, cpuCores, memorySize, serverCost, powerCost);
	}

	std::sort(serverInfos.begin(), serverInfos.end());
	//存储各型号服务器的容量大小
	const vector<ServerInfo> ServerConsultInfo = serverInfos;
	// 输入虚拟机信息
	std::string vmType, isDouble;
	scanf("%d", &vmNum);
	for (int i = 0; i < vmNum; ++i)
	{
		std::cin >> vmType >> cpuCores >> memorySize >> isDouble;
		generateVm(vmType, cpuCores, memorySize, isDouble);
	}

	// 一次性读入所有天数的信息
	int commandNum;
	std::string vmId, commandType;
	scanf("%d", &dayNum);
	// 预留空间
	commands.resize(dayNum);
	for (int i = 0; i < dayNum; ++i)
	{
		scanf("%d", &commandNum);
		// 预留空间
		commands[i].reserve(commandNum);
		for (int j = 0; j < commandNum; ++j)
		{
			std::cin >> commandType;
			// 当前命令是add
			if (commandType[1] == 'a')
			{
				std::cin >> vmType >> vmId;
			}
			else
			{
				std::cin >> vmId;
				vmType = "";
			}
			// 生成命令信息
			generateCommand(i, commandType, vmType, vmId);
		}
	}
	cerr<< " Input down";
}

//输出
void getOutput()
{
    cerr<< "Output start";
	if (DEBUG_TYPE == 1)
	{
		std::string outputPath = "./output.txt";
		// 重定向输入到标准输入
		std::freopen(outputPath.c_str(), "wb", stdout);
	}
	for (auto &s : res)
	{
		std::cout << s;
	}
	fflush(stdout);
}