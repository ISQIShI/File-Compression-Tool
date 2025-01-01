#include"ThreadPool.h"

void ThreadPool::ThreadLoop(ThreadInfo* threadinfo)
{
	//注意若将线程容器中元素删除, threadinfo 会变成野指针,若之后扩展功能需考虑该情况
	//每个线程的工作函数
	while (true) {
		{ //进入临界区，保护任务队列(共享数据)被正常读写
			std::unique_lock<std::mutex> lock(queueMutex);
			//线程休眠，直到有可用任务或线程池停止运行或线程被终止再唤醒
			threadWaitTask.wait(lock, [this, threadinfo] {return !isRunning || threadinfo->willTerminate || !taskQueue.empty(); });
			//如果线程被终止或者线程池停止且没有任务，退出线程
			if (threadinfo->willTerminate || (!isRunning && taskQueue.empty()))return;
			//从队列中取出任务
			threadinfo->taskInfo = taskQueue.front();
			workThreadAmount.fetch_add(1);
			taskQueue.pop();
		}
		if (!threadinfo->taskInfo) {
			throw std::runtime_error("任务信息为空");
			continue;
		}
		//执行任务
		threadinfo->taskInfo->taskFunc();

		if (threadinfo->taskInfo->needWait) {
			//如果执行完的任务需要等待，则将具有该任务ID的等待任务数减一
			std::shared_lock<std::shared_mutex> lock(waitTaskMutex);
			auto amount = waitTasks.at(threadinfo->taskInfo->taskID).first.fetch_sub(1);
			if (amount <= 2) {
				if (amount == 1) {
					//若具有该任务ID的等待任务数为0，发送通知
					lock.unlock();
					std::unique_lock<std::shared_mutex> uniquelock(waitTaskMutex);
					//二次确定后移除任务
					if (waitTasks.at(threadinfo->taskInfo->taskID).first.load() == 0) {
						waitTasks.at(threadinfo->taskInfo->taskID).second.notify_all();
						waitTasks.erase(threadinfo->taskInfo->taskID);
					}
				}
				//查看该任务ID是否关联有回调函数
				std::unique_lock<std::mutex> callbackfunclock(callBackFuncMutex);
				if (callBackFuncs.find(threadinfo->taskInfo->taskID) != callBackFuncs.end()) {
					SubmitTask(callBackFuncs.at(threadinfo->taskInfo->taskID));
					callBackFuncs.erase(threadinfo->taskInfo->taskID);
				}
			}
		}
		delete threadinfo->taskInfo;
		threadinfo->taskInfo = nullptr;
		workThreadAmount.fetch_sub(1);
	}
}

void ThreadPool::ShutDownThreadPool()
{
	{//修改运行标志，通知所有线程
		std::unique_lock<std::mutex> lock(queueMutex);
		isRunning.store(false);
	}
	threadWaitTask.notify_all(); // 唤醒所有线程
	for (auto& worker : workThreads) {
		if (worker->mThread && worker->mThread->joinable()) {
			worker->mThread->join(); // 等待线程结束
			delete worker->mThread;
		}
	}
    workThreads.clear();
}

void ThreadPool::DestroyThread(std::thread::id threadID)
{
	ThreadInfo* threadinfo;
	{
		std::lock_guard<std::mutex> lock(threadMutex);
		auto temp = find_if(workThreads.begin(), workThreads.end(), [threadID](const std::unique_ptr<ThreadInfo>& worker) { return worker->threadID == threadID; });
		threadinfo = (*temp).get();
		if (temp == workThreads.end()) return;
	}
	if (workThreads.size() <= 1)
	{
		ShutDownThreadPool();
		return;
	}

	threadinfo->willTerminate = true;
	threadWaitTask.notify_all();
	if (threadinfo->mThread && threadinfo->mThread->joinable()) {
		threadinfo->mThread->join();
		delete threadinfo->mThread;
	}
	{
		std::lock_guard<std::mutex> lock(threadMutex);
		auto temp = find_if(workThreads.begin(), workThreads.end(), [threadID](const std::unique_ptr<ThreadInfo>& worker) { return worker->threadID == threadID; });
		workThreads.erase(temp);
	}

}

ThreadInfo* ThreadPool::FindThread(std::thread::id threadID)
{
    std::lock_guard<std::mutex> lock(threadMutex);
	auto temp = find_if(workThreads.begin(), workThreads.end(), [threadID](const std::unique_ptr<ThreadInfo>& worker) { return worker->threadID == threadID; });
    if (temp != workThreads.end())return (*temp).get();
	return nullptr;
}

bool ThreadPool::FindTask(size_t taskID)
{
	//保护任务队列
	std::unique_lock<std::mutex> lock(queueMutex);
	std::queue<TaskInfo*> tempTaskQueue(taskQueue);
	lock.unlock();
	//循环判断队列中是否有任务
	while (!tempTaskQueue.empty()) {
		if (tempTaskQueue.front()->taskID == taskID)return true;
		tempTaskQueue.pop();
	}
	//检查是否有线程在执行该任务
	for (auto& worker : workThreads) {
        if (worker->taskInfo && worker->taskInfo->taskID == taskID)return true;
	}
	return false;
}

void ThreadPool::WaitTask(size_t taskID)
{
	std::shared_lock<std::shared_mutex> lock(waitTaskMutex);
	//查找等待容器内是否存在拥有该ID的任务,若没找到直接返回
	if (waitTasks.find(taskID) == waitTasks.end())return;
	lock.unlock();
	ThreadInfo* temp = FindThread(std::this_thread::get_id());
	std::thread::id threadid;
	if (temp) {
		auto currentThreadInfo = std::make_unique<ThreadInfo>(nullptr, false, nullptr, this);
		//使线程与线程信息互相关联建立联系
		currentThreadInfo->mThread = new std::thread(&ThreadPool::ThreadLoop, this, currentThreadInfo.get());
		currentThreadInfo->threadID = currentThreadInfo->mThread->get_id();
		threadid = currentThreadInfo->threadID;
		//转移所有权到容器内
		std::lock_guard<std::mutex> lock(threadMutex);
		workThreads.emplace_back(std::move(currentThreadInfo));
	}
	lock.lock();
	if (waitTasks.find(taskID) != waitTasks.end() && waitTasks.at(taskID).first.load() > 0) {
		waitTasks.at(taskID).second.wait(lock, [this, taskID]() {
			return waitTasks.find(taskID) == waitTasks.end() || waitTasks.at(taskID).first.load() == 0;
			}
		);
	}
	lock.unlock();
	
	if (temp) {
		//销毁刚刚创建的那个线程
		std::thread(&ThreadPool::DestroyThread, this, threadid).detach();
	}
}

