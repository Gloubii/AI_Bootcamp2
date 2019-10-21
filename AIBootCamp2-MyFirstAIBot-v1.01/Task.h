#ifndef TASK_H
#define TASK_H

#include "Blackboard.h"
#include <vector>
#include <algorithm>
#include <cassert>
#include <future>
#include <condition_variable>
#include <mutex>
#include <type_traits>
#include <utility>

class Task;
struct BehaviorTree : std::unique_ptr<Task>
{
	using ClonePtr = Task *;
	BehaviorTree() = default;
	BehaviorTree(ClonePtr clone) : std::unique_ptr<Task>(clone) {};

	using BehaviorTreeKey = std::string;
	static std::unordered_map<BehaviorTreeKey, Task*> behaviorTreeLibrary;
	static BehaviorTree createBehaviorTree(BehaviorTreeKey key);

};

class Task
{
public:
	enum ReturnValue { FAILLURE, SUCCESS, RUNNING};
	using BlackboardKey = std::string;
	using BlackboardPtr = Blackboard<BlackboardKey>*;
	using ClonePtr = Task*;

	using BehaviorTreeKey = BehaviorTree::BehaviorTreeKey;

	Task() = default;
	Task(const Task&) = delete;
	Task& operator= (const Task&) = delete;


	using ChildPtr = Task*;
	//using ChildPtr = std::unique_ptr<Task>;
	// Task*;

	


public:
	virtual ReturnValue run(BlackboardPtr) = 0;
	virtual ClonePtr clone() = 0;
	virtual void terminate() {};

	virtual ~Task() = default;
};

Task::ReturnValue operator! (Task::ReturnValue res);


// Virtual Tasks

class TaskAction : public Task { };

class TaskCondition : public Task { };

class TaskComposite : public Task {

public:
	TaskComposite(std::vector<ChildPtr> &children) : children{ children } {};

	/*
	virtual ClonePtr clone() const override {
		std::vector<ChildPtr> cloneChild;
		for (auto& child : children) {
			cloneChild.push_back(child->clone());
		}
		return std::make_unique<TaskCompositecloneChild);
	}
	*/
	void terminate() override {
		for (auto& child : children) {
			child->terminate();
		}
	}

	virtual ~TaskComposite() {
		for (auto& child : children) {
			delete child;
		}
	}


protected:
	std::vector<ChildPtr> children;
};

class TaskDecorator : public Task {

public:
	TaskDecorator(ChildPtr child) : child{ child } {};

	/*virtual ClonePtr clone() const override {
		return child->clone();
	}*/

	void terminate() override {
			child->terminate();
	}

	virtual ~TaskDecorator() {
		delete child;
	}

protected:
	ChildPtr child;
};

// Implemented Tasks

class SubTreeReference : public Task {
	BehaviorTreeKey behaviorTreeKey;

public:
	SubTreeReference(const BehaviorTreeKey& behaviorTreeKey) : behaviorTreeKey{ behaviorTreeKey } {}

	ReturnValue run(BlackboardPtr blackboard) {
		throw std::exception("This task can't be run");
	}

	ClonePtr clone() override {
		return BehaviorTree::createBehaviorTree(behaviorTreeKey).release();
	}
};



// Composite

template<Task::ReturnValue Test>
class TaskSelequence : public TaskComposite {

	std::vector<ChildPtr>::iterator runningChild;

	bool terminated = false;

public:
	TaskSelequence(std::vector<ChildPtr> children) : TaskComposite{ children } {
		runningChild = std::begin(this->children);
	}

	ReturnValue run(BlackboardPtr blackboard) override {
		terminated = false;
		ReturnValue res;
		runningChild = std::find_if(std::begin(children), std::end(children), [&](auto child) { res = child->run(blackboard); return res != Test || terminated; });
		if (res != RUNNING) {
			runningChild = std::begin(children);
		}
		return res;
	}

	ClonePtr clone() override {
		std::vector<ChildPtr> childCopy;
		std::for_each(begin(children), end(children), [&childCopy](auto child) {childCopy.push_back(child->clone()); });
		return new TaskSelequence<Test>(childCopy);
	}

	void terminate() override {
		terminated = true;
		for (auto& child : children) {
			child->terminate();
		}
	}

};

using TaskSelector = TaskSelequence<Task::FAILLURE>;
using TaskSequence = TaskSelequence<Task::SUCCESS>;

//class TaskSelector : public TaskSelequence<Task::FAILLURE> { };
//class TaskSequence : public TaskSelequence<Task::SUCCESS> { };


class TaskParallel : public TaskComposite {

	std::vector<ChildPtr> runnningChildren;
	std::vector<ChildPtr> lateChildren;
	BlackboardPtr blackboard;

	bool done;
	std::condition_variable cv;
	std::mutex mutex;

	ReturnValue result = FAILLURE;

public:
	
	TaskParallel(std::vector<ChildPtr> children) : TaskComposite{ children }, blackboard{ nullptr }, done{ false } {}

	ReturnValue run(BlackboardPtr blackboard) override {

		this->blackboard = blackboard;
		done = false;
		auto runchildLambda = [&](ChildPtr child) {runChild(child); };

		std::vector<std::future<void>> threads;
		for (auto& child : lateChildren.size() ? lateChildren : children) {
			
			threads.push_back(std::async(runchildLambda, child));
		}
		{
			std::unique_lock<std::mutex> ulk(mutex);
			cv.wait(ulk, [&] {return done; });
		}
		return result;
	}

	void runChild(ChildPtr child) {
		
		runnningChildren.push_back(child);

		auto res = child->run(blackboard);
		
		auto it = std::find(begin(runnningChildren), end(runnningChildren), child);
		assert(it != end(runnningChildren));
		runnningChildren.erase(it);

		switch (res)
		{
		case Task::FAILLURE:
			{
				std::lock_guard<std::mutex> lk(mutex);
				result = FAILLURE;
				done = true;
			}
			cv.notify_one();
			terminate();
			break;
		case Task::SUCCESS:
			if (!runnningChildren.size() && result != RUNNING) {
				{
					std::lock_guard<std::mutex> lk(mutex);
					done = true;
					result = SUCCESS;
				}
				cv.notify_one();
			}
			break;
		case Task::RUNNING:
		{
			std::lock_guard<std::mutex> lk(mutex);
			result = RUNNING;
			lateChildren.push_back(child);
			if (!runnningChildren.size()) {
				done = true;
			}
		}
		cv.notify_one();
			break;
		default:
			assert(false);
			break;
		}
	}
	
	ClonePtr clone() override {
		std::vector<ChildPtr> childCopy;
		std::for_each(begin(children), end(children), [&childCopy](auto child) {childCopy.push_back(child->clone()); });
		return new TaskParallel(childCopy);
	}

};


// Decorators

class TaskLimit : public TaskDecorator {
	int runLimit;
	int runSoFar;

public:
	TaskLimit(ChildPtr child, int runLimit) : TaskDecorator{ child }, runLimit{ runLimit }, runSoFar{ 0 } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		if (runSoFar < runLimit) {
			auto res = child->run(blackboard);
			if (res != RUNNING)
				++runSoFar;
			return res;
		}
		return FAILLURE;
	}

	virtual ClonePtr clone() override {
		return new TaskLimit(child->clone(), runLimit);
	}
};

class TaskUntilFail : public TaskDecorator {

public:

	TaskUntilFail(ChildPtr child) : TaskDecorator{ child } {};

	ReturnValue run(BlackboardPtr blackboard) override {
		ReturnValue res;
		for (res = child->run(blackboard); res == SUCCESS; res = child->run(blackboard)) {}
		return res == RUNNING ? RUNNING : SUCCESS;
	}

	ClonePtr clone() override {
		return new TaskUntilFail{ child->clone() };
	}
};


class TaskInverter : public TaskDecorator {

public:

	TaskInverter(ChildPtr child) : TaskDecorator{ child } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		return !child->run(blackboard);
	}

	ClonePtr clone() override {
		return new TaskInverter{ child->clone() };
	}
};

template <Task::ReturnValue Return>
class TaskReturnValue : public TaskDecorator {

public:

	TaskReturnValue(ChildPtr child) : TaskDecorator{ child } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		child->run(blackboard);
		return Return;
	}

	ClonePtr clone() override {
		return new TaskReturnValue<Return>{ child->clone() };
	}
};

using TaskFaillure = TaskReturnValue<Task::FAILLURE>;
using TaskSuccess = TaskReturnValue<Task::SUCCESS>;

class TaskCreateBlackboard : public TaskDecorator {

public:

	TaskCreateBlackboard(ChildPtr child) : TaskDecorator{ child } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		BlackboardPtr newBlackboard = new Blackboard<BlackboardKey>(blackboard);
		auto res = child->run(newBlackboard);
		delete newBlackboard;
		return res;
	}

	ClonePtr clone() override {
		return new TaskCreateBlackboard{ child->clone() };
	}
};

class TaskCreateSubTree : public TaskDecorator {
	BehaviorTreeKey subTreeKey;

public:

	TaskCreateSubTree(const BehaviorTreeKey& subTreeKey) : TaskDecorator{ nullptr }, subTreeKey{ subTreeKey } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		if (!child) {
			child = BehaviorTree::createBehaviorTree(subTreeKey).release();
		}
		return child->run(blackboard);
	}

	ClonePtr clone() override {
		return new TaskCreateSubTree(subTreeKey);
	}
};

class TaskPerformInterruption;

class TaskInterrupter : public TaskDecorator {
	bool done = false;
	std::condition_variable cv;
	std::mutex mutex;
	ReturnValue result = FAILLURE;

	using InteruptPerformer = TaskPerformInterruption *;

	TaskInterrupter* lastCloned;
	std::vector<InteruptPerformer> clonedPerformers;
	
	friend class TaskPerformInterruption;


	void runChild(ChildPtr child, BlackboardPtr blackboard) {
		auto res = child->run(blackboard);
		{
			std::lock_guard<std::mutex> lk(mutex);
			result = res;
			done = true;
		}
		cv.notify_one();
	}

public:

	TaskInterrupter(ChildPtr child) : TaskDecorator{ child }, clonedPerformers{} {}

	ReturnValue run(BlackboardPtr blackboard) override {
		done = false;
		auto runchildLambda = [&](ChildPtr child) {runChild(child, blackboard); };
		auto thread = std::async(runchildLambda, child);
		{
			std::unique_lock<std::mutex> ulk(mutex);
			cv.wait(ulk, [&] {return done; });
			
		}
		return result;
	}

	

	ClonePtr clone() override;


private:

	void setResult(const ReturnValue& desiredResult) {
		{
			std::lock_guard<std::mutex> lk(mutex);
			result = desiredResult;
			done = true;
			terminate();
		}
		cv.notify_one();
	}

	void addClonedPerformer(InteruptPerformer performer) {
		clonedPerformers.push_back(performer);
	}
};

class TaskPerformInterruption : public Task {
	TaskInterrupter* interrupter;
	TaskInterrupter* clonedInterrupter;
	const ReturnValue desiredResult;

	friend class TaskInterrupter;

	void setInterupter(TaskInterrupter* interrupter) {
		this->interrupter = interrupter;
	}

public:
	TaskPerformInterruption(TaskInterrupter* interrupter, const ReturnValue& desiredResult) : interrupter{ interrupter }, desiredResult{ desiredResult } {
	}

	ReturnValue run(BlackboardPtr blackboard) override {
		interrupter->setResult(desiredResult);
		return SUCCESS;
	}

	ClonePtr clone() override {
		
		if (clonedInterrupter != interrupter->lastCloned) {
			auto clone = new TaskPerformInterruption(interrupter->lastCloned, desiredResult);
			clonedInterrupter = interrupter->lastCloned;
			return clone;
		}
		else
		{
			auto clone = new TaskPerformInterruption(nullptr, desiredResult);
			interrupter->addClonedPerformer(clone);
			return clone;
		}
	}
};

// Conditions
template<class Predicate>
class TaskPredicate : public TaskCondition {
	Predicate pred;
	BlackboardPtr blackboard;

public:
	TaskPredicate(Predicate predicate) : pred{ predicate } {}

	ReturnValue run(BlackboardPtr blackboard) override {
		if constexpr (std::is_invocable<Predicate, BlackboardPtr>::value) {
			return pred(blackboard) ? SUCCESS : FAILLURE;
		}
		else
		{
			return pred() ? SUCCESS : FAILLURE;
		}
	}

	ClonePtr clone() override {
		return new TaskPredicate(pred);
	}
};


#endif
