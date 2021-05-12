//
// Created by Lan Haoning on 2021/3/15.
//

#ifndef CODECRAFT_2021_COMMAND_H
#define CODECRAFT_2021_COMMAND_H

#include <string>
// 定义结构体用于存储添加和删除命令
struct Command
{
	int commandType; // 0->delete, 1->add
	std::string vmType;
	int vmId;
	Command(int _commandType, std::string _vmType, int _vmId) :
		commandType(_commandType), vmType(_vmType), vmId(_vmId) {}
};

#endif //CODECRAFT_2021_VIRTUAL_MACHINE_H
