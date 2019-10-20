#include "Task.h"

std::unordered_map<Task::BehaviorTreeKey, Task::ClonePtr> Task::behaviorTreeLibrary;

Task::ReturnValue operator! (Task::ReturnValue res) {
	switch (res)
	{
	case Task::FAILLURE:
		return Task::SUCCESS;
		break;
	case Task::SUCCESS:
		return Task::FAILLURE;
		break;
	case Task::RUNNING:
		return Task::RUNNING;
		break;
	default:
		break;
	}
	return res;
}