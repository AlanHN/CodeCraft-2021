#include "CodeCraft-2021.h"
/*
输入对应要清空的服务器id，在已购买的purchaseInfos中将对应的服务器清空，
同时将部署在这个服务器上的对应的虚拟机ID压入到RunningID中，
同时在vmIdToServerId，vmIdToEnd中抹除对应的虚拟机id；
*/

void EraseServerID(
	int TempServerId,		// 对应的要清空的服务器ID
	vector<int> &RunningID) //当天需要添加的虚拟机id，包括从command中取出的“add”操作。以及利用率低于60%中的服务器的所有虚拟机
{
	purchaseInfos[TempServerId] = serverInfos[serverNametoIndex[purchaseInfos[TempServerId].serverType]];
	for (auto it = vmIdToServerId.begin(); it != vmIdToServerId.end(); it++)
	{
		if (it->second == TempServerId)
		{
			RunningID.push_back(it->first);
			vmIdToEnd.erase(it->first);
			vmIdToServerId.erase(it);
		}
	}
}

/*
输入的RunningID包含第“i - 1”天的所有虚拟机ID和第“i”天对应的“add”操作的虚拟机id，
使用一种算法将其最好的分配，不能分配就购买虚拟机
*/
void Allocation(vector<int> &RunningID, int day, unordered_map<int, int> &lastday_vmIdToServerId, unordered_map<int, int> &lastday_vmIdToEnd)
{
	auto command = commands[day];
	int CurrentCPU = 0;
	int CurrentHardware = 0;
	int CPUNeed = 0;
	int HardWareNeed = 0;
	float CPUefficiency = 0;
	float HardWareefficiency = 0;

	
	std::unordered_map<int, int> purchaseToday;// 中间变量保存 当天购买的服务器的型号 与 对应的数量
	std::vector<std::string> tmpRes; // 存储中间输出结果
	std::vector<int> purchaseOrderToday;	// 中间变量保存申请服务器序号的顺序
	std::unordered_map<int, int> purchaseHashToday;// 中间变量表示 当前申请的编号 与 服务器编号 的哈希表
	int purchaseCntToday = 0;


	std::vector<int> migrationOrderToday;	// 中间变量保存迁移的 虚拟机序号 的顺序
	std::unordered_map<int, pair<int, int>> migrationHashToday;// 中间变量表示 当前迁移的虚拟器编号 与 <服务器Id,end >的哈希表

	int migrationCntToday = 0;
	for (auto it = purchaseInfos.begin(); it != purchaseInfos.end(); ++it)
	{
		CurrentCPU = (it->second).cpuCoresA + (it->second).cpuCoresB;
		CurrentHardware = (it->second).memorySizeA + (it->second).memorySizeB;
		CPUefficiency = (float)CurrentCPU / (float)(serverInfos[serverNametoIndex[it->second.serverType]].cpuCoresA * 2);
		HardWareefficiency = (float)CurrentHardware / (float)(serverInfos[serverNametoIndex[it->second.serverType]].memorySizeA * 2);
		if (CPUefficiency <= 0.6 || HardWareefficiency <= 0.6)
		{
			//将利用率过低的服务器取出重新排列
			int TempServerId = serverNametoIndex[it->second.serverType];
			EraseServerID(TempServerId, RunningID);
		}
	}

	Reallocation(RunningID, day,purchaseOrderToday,purchaseHashToday,tmpRes,purchaseToday,purchaseCntToday);
	Compare_with_yesterday(lastday_vmIdToServerId, vmIdToServerId, lastday_vmIdToEnd, vmIdToEnd, migrationCntToday, migrationOrderToday, migrationHashToday);

	int start_index = purchaseInfos.size() - purchaseCntToday;
	int cur_cnt = purchaseInfos.size() - purchaseCntToday;
	std::unordered_map<int, ServerInfo> tmpServerInfos;
	std::unordered_map<int, int> numberHash;
	for (int k = 0; k < purchaseOrderToday.size(); ++k)
	{
		for (int j = 0; j < purchaseHashToday.size(); ++j)
		{
			auto tmpServerNumber = purchaseHashToday[j + start_index];
			if (tmpServerNumber == purchaseOrderToday[k])
			{
				numberHash[j + start_index] = cur_cnt;
				// 将对应的信息进行交换
				tmpServerInfos.insert({cur_cnt, purchaseInfos[j + start_index]});
				cur_cnt++;
			}
		}
	}
	for (int j = 0; j < purchaseHashToday.size(); ++j)
	{
		purchaseInfos[j + start_index] = tmpServerInfos[j + start_index];
	}

	// 调整虚拟机对应的服务器ID
	for (auto it = vmIdToServerId.begin(); it != vmIdToServerId.end(); ++it)
	{
		// 在start_index之前的就不用调整了
		if (it->second < start_index)
			continue;
		it->second = numberHash[it->second];
	}

	// 调整输出信息
	std::vector<std::string> resForRequest;
	for (auto &s : tmpRes)
	{
		int tmpNum = 0;
		int j = 1;
		for (j = 1; j < s.size(); ++j)
		{
			if (s[j] >= '0' && s[j] <= '9')
				tmpNum = tmpNum * 10 + s[j] - '0';
			else
				break;
		}
		// 如果编号小于本次开始的编号, 则不用调整
		if (tmpNum < start_index)
		{
			resForRequest.emplace_back(s);
			continue;
		}
		std::string tmpStr = '(' + std::to_string(numberHash[tmpNum]) + s.substr(j, s.size() - j);
		resForRequest.emplace_back(tmpStr);
	}

	// 整理当天的购买信息
	res.emplace_back("(purchase, " + std::to_string(purchaseOrderToday.size()) + ")\n");
	for (int j = 0; j < purchaseOrderToday.size(); ++j)
	{
		std::string tmpStr = "";
		tmpStr += '(';
		tmpStr += serverInfos[purchaseOrderToday[j]].serverType;
		tmpStr += ", ";
		tmpStr += std::to_string(purchaseToday[purchaseOrderToday[j]]);
		tmpStr += ")\n";
		res.emplace_back(tmpStr);
	}
	//这里改过代码，整理当天migration信息
	res.emplace_back("(migration, " + std::to_string(migrationOrderToday.size()) + ")\n");
	for (int j = 0; j < migrationOrderToday.size(); ++j) //有migrationOrderToday.size() 条迁移命令
	{
		std::string tmpStr = "";
		tmpStr += '(';
		tmpStr += std::to_string(migrationOrderToday[j]); //输出当前迁移命令的 虚拟器的Id
		tmpStr += ", ";
		tmpStr += std::to_string(migrationHashToday[migrationOrderToday[j]].first); //输出迁移的目标服务器的Id
		if (migrationHashToday[j].second == 1 || migrationHashToday[j].second == 2)
		{ //如果是迁移的单节点，需要增加输出节点是A还是B
			tmpStr += ", ";
			tmpStr += migrationHashToday[migrationOrderToday[j]].second == 1 ? "A" : "B"; //增加输出节点是A还是B
		}
		tmpStr += ")\n";
		res.emplace_back(tmpStr);
	}

	// 整理当天的请求输出信息
	for (auto &s : resForRequest)
	{
		res.emplace_back(s);
	}
}

//处理主函数
void process()
{
	// TODO:process
	// 当前购买的服务器的编号
	//*************************************************************************************************
	// 先保存上一次虚拟机的信息，再将当天所有的操作增加操作加入虚拟机队列，排列时判断是否需要加服务器，尽可能往每日耗能小的服务器中放。
	
	//前一天的虚拟机状态和服务器状态，用于在计算服务器购买数量和迁移之后比较，得出当天的操作

	auto lastday_vmIdToType = vmIdToType;
	auto lastday_vmIdToServerId = vmIdToServerId;
	auto lastday_vmIdToEnd = vmIdToEnd;
	//auto lastday_Running_VMID = Running_VMID;

	for (int i = 0; i < commands.size(); ++i)
	{
		vector<int> RunningID;
		// get all vmId running
		for (int j = 0; j < commands[i].size(); ++j)
		{
			auto tmpCommand = commands[i][j];
			// if is add
			if (tmpCommand.commandType == 1)
			{
				RunningID.push_back(tmpCommand.vmId);
			}
		}
		// allocation.
		Allocation(RunningID, i, lastday_vmIdToServerId, lastday_vmIdToEnd);

		// delete vmId
		for (int j = 0; j < commands[i].size(); ++j)
		{
			auto tmpCommand = commands[i][j];
			// if is delete
			if (tmpCommand.commandType == 0)
			{
				for (auto it = RunningID.begin(); it != RunningID.end(); ++it)
				{
					if (*it == tmpCommand.vmId)
					{
						RunningID.erase(it);
						break;
					}
				}
			}
		}
	}
}


//Reallocation输入为当天需要插入的所有虚拟机id，完成布置和服务器购买之后删除对应的服务器id
void Reallocation(
	vector<int> &RunningID, //当天的增加的虚拟机以及服务被清空的服务器上的虚拟机
	int day,					//第i天
	vector<int> &purchaseOrderToday,
	unordered_map<int, int> &purchaseHashToday,
	vector<std::string> &tmpRes,
	unordered_map<int, int> &purchaseToday,
	int &purchaseCntToday
)
{
	for (int j = 0; j < RunningID.size(); ++j)
	{
		// std::cout << purchaseInfos.size() << std::endl;
		Command tmpCommand(1, vmIdToType[RunningID[j]], RunningID[j]);
		int cnt = purchaseInfos.size();
		// 如果是部署虚拟机命令
		if (tmpCommand.commandType == 1)
		{
			int isDouble = vmInfos[tmpCommand.vmType].isDouble;
			int cpuCores = vmInfos[tmpCommand.vmType].cpuCores;
			int memorySize = vmInfos[tmpCommand.vmType].memorySize;

			bool judge = false;
			// 首先判断当前购买的机器是否有足够的空间
			// 双节点部署
			if (isDouble)
			{
				for (auto it = purchaseInfos.begin(); it != purchaseInfos.end(); ++it)
				{
					if ((it->second).cpuCoresA >= cpuCores / 2 && (it->second).memorySizeA >= memorySize / 2 &&
						(it->second).cpuCoresB >= cpuCores / 2 && (it->second).memorySizeB >= memorySize / 2)
					{

						std::string tmpStr = '(' + std::to_string(it->first) + ")\n";
						tmpRes.emplace_back(tmpStr);
						(it->second).cpuCoresA -= cpuCores / 2;
						(it->second).memorySizeA -= memorySize / 2;
						(it->second).cpuCoresB -= cpuCores / 2;
						(it->second).memorySizeB -= memorySize / 2;

						judge = true;
						//进行状态的更新
						vmIdToServerId[tmpCommand.vmId] = it->first;
						vmIdToEnd[tmpCommand.vmId] = 0;
						break;
					}
				}
				if (judge)
					continue;
				// 没有足够的服务器，需要重新购买服务器
				for (int k = 0; k < serverInfos.size(); ++k)
				{
					auto server = serverInfos[k];
					// std::cout << server.cpuCoresA << " " << server.memorySizeA << std::endl;
					if (server.cpuCoresA >= cpuCores / 2 && server.memorySizeA >= memorySize / 2 &&
						server.cpuCoresB >= cpuCores / 2 && server.memorySizeB >= memorySize / 2)
					{
						// std::cout << server.serverType << std::endl;
						// 加入到新购买服务器表中
						purchaseCntToday++;
						purchaseInfos.insert({cnt++, server});
						purchaseInfos[cnt - 1].cpuCoresA -= cpuCores / 2;
						purchaseInfos[cnt - 1].memorySizeA -= memorySize / 2;
						purchaseInfos[cnt - 1].cpuCoresB -= cpuCores / 2;
						purchaseInfos[cnt - 1].memorySizeB -= memorySize / 2;
						// 购买服务器总编号与服务器编号的哈希对应表
						purchaseHashToday[cnt - 1] = k;
						// 将当前申请的序号加入到purchaseOrderToday中
						if (find(purchaseOrderToday.begin(), purchaseOrderToday.end(), k) == purchaseOrderToday.end())
							purchaseOrderToday.push_back(k);
						if (purchaseToday.find(k) == purchaseToday.end())
						{
							purchaseToday[k] = 0;
						}
						purchaseToday[k]++;
						std::string tmpStr = '(' + std::to_string(cnt - 1) + ")\n";
						tmpRes.emplace_back(tmpStr);
						vmIdToServerId[tmpCommand.vmId] = cnt - 1;
						vmIdToEnd[tmpCommand.vmId] = 0;
						break;
					}
				}
			}
			else
			{
				for (auto it = purchaseInfos.begin(); it != purchaseInfos.end(); ++it)
				{
					if ((it->second).cpuCoresA >= cpuCores && (it->second).memorySizeA >= memorySize)
					{

						(it->second).cpuCoresA -= cpuCores;
						(it->second).memorySizeA -= memorySize;
						judge = true;
						vmIdToEnd[tmpCommand.vmId] = 1;
						vmIdToServerId[tmpCommand.vmId] = it->first;
						break;
					}
					else if ((it->second).cpuCoresB >= cpuCores && (it->second).memorySizeB >= memorySize)
					{
						(it->second).cpuCoresB -= cpuCores;
						(it->second).memorySizeB -= memorySize;
						judge = true;
						vmIdToEnd[tmpCommand.vmId] = 2;
						vmIdToServerId[tmpCommand.vmId] = it->first;
						break;
					}
				}
				if (judge)
					continue;
				// 没有足够的服务器，需要重新购买服务器
				for (int k = 0; k < serverInfos.size(); ++k)
				{
					if (serverInfos[k].cpuCoresA >= cpuCores && serverInfos[k].memorySizeA >= memorySize)
					{
						// 加入到新购买服务器表中
						purchaseCntToday++;
						purchaseInfos.insert({cnt++, serverInfos[k]});
						purchaseInfos[cnt - 1].cpuCoresA -= cpuCores;
						purchaseInfos[cnt - 1].memorySizeA -= memorySize;
						purchaseHashToday[cnt - 1] = k;
						// 将当前申请的序号加入到purchaseOrderToday中
						if (find(purchaseOrderToday.begin(), purchaseOrderToday.end(), k) == purchaseOrderToday.end())
							purchaseOrderToday.push_back(k);
						if (purchaseToday.find(k) == purchaseToday.end())
						{
							purchaseToday[k] = 0;
						}
						purchaseToday[k]++;
						// 部署在任意一个节点上面
						std::string tmpStr = '(' + std::to_string(cnt - 1) + ", A)\n";
						tmpRes.emplace_back(tmpStr);
						vmIdToEnd[tmpCommand.vmId] = 1;
						vmIdToServerId[tmpCommand.vmId] = cnt - 1;
						break;
					}
				}
			}
		}

		auto command = commands[day];
		for (int j = 0; j < command.size(); j++)
		{
			if (command[j].commandType != 1)
			{
				int delVmId = command[j].vmId;
				int serverId = vmIdToServerId[delVmId];
				// 恢复对应的服务器的资源
				auto tmpVmInfo = vmInfos[vmIdToType[delVmId]];
				// 删除双节点部署的虚拟机
				if (tmpVmInfo.isDouble)
				{
					purchaseInfos[serverId].cpuCoresA += tmpVmInfo.cpuCores / 2;
					purchaseInfos[serverId].memorySizeA += tmpVmInfo.memorySize / 2;
					purchaseInfos[serverId].cpuCoresB += tmpVmInfo.cpuCores / 2;
					purchaseInfos[serverId].memorySizeB += tmpVmInfo.memorySize / 2;
					vmIdToType.erase(delVmId);
				}
				// 删除单节点部署的虚拟机
				else
				{
					// 在A点部署
					if (vmIdToEnd[delVmId] == 1)
					{
						purchaseInfos[serverId].cpuCoresA += tmpVmInfo.cpuCores;
						purchaseInfos[serverId].memorySizeA += tmpVmInfo.memorySize;
					}
					// 在B点部署
					else if (vmIdToEnd[delVmId] == 2)
					{
						purchaseInfos[serverId].cpuCoresB += tmpVmInfo.cpuCores;
						purchaseInfos[serverId].memorySizeB += tmpVmInfo.memorySize;
					}
					vmIdToType.erase(delVmId);
					vmIdToEnd.erase(delVmId);
				}
			}
		}
	}
}

//输入昨天虚拟机与服务器对应的id和今天调整后的虚拟机与服务器对应id，整理后输出对应migration信息
//比较今天和昨天的配置的变化，并将变化赋值给output相关变量，用于migration部分的输出
void Compare_with_yesterday(unordered_map<int, int> &vmIdToServerIdYesterday,
							unordered_map<int, int> &vmIdToServerIdToday,
							unordered_map<int, int> &vmIdToEndYesterday,
							unordered_map<int, int> &vmIdToEndToday,
							int &migrationCntToday, vector<int> &migrationOrderToday,
							unordered_map<int, pair<int, int>> &migrationHashToday)
{ /* 中间变量表示 当前迁移的虚拟器编号 与 <服务器Id,end >的哈希表*/
	for (auto it = vmIdToServerIdYesterday.begin(); it != vmIdToServerIdYesterday.end(); it++)
	{
		if (vmIdToServerIdToday[it->first])
		{ //首先，vmIdToServerIdToday[i]必须是没有被删除的。
			//只有保存节点配置不一样的时候，才会输出到migration信息
			if (vmIdToEndToday[it->first] != vmIdToEndYesterday[it->first] || (vmIdToServerIdToday[it->first] != vmIdToServerIdYesterday[it->first]))
			{ //当IdToEnd不同，或者IdToServer不同，需要migration (migration,ServerId,End)
				migrationOrderToday[migrationCntToday++] = it->first;
				migrationHashToday[it->first].first = vmIdToServerIdToday[it->first];
				migrationHashToday[it->first].second = vmIdToEndToday[it->first];
			}
		}
	}
}

int main()
{
	getInput();
	process();
	getOutput();
	return 0;
}