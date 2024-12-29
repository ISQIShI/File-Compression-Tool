#include"ThreadPool.h"

void ThreadPool::ThreadLoop(ThreadInfo* threadinfo)
{
	//注意若将线程容器中元素删除, threadinfo 会变成野指针,若之后扩展功能需考虑该情况
	//每个线程的工作函数
	while (true) {
		std::function<void()> task; //用于存储取出的任务
		{ //进入临界区，保护任务队列(共享数据)被正常读写
			std::unique_lock<std::mutex> lock(queueMutex);
			//线程休眠，直到有可用任务或线程池停止运行或线程被终止再唤醒
			threadWaitTask.wait(lock, [this, threadinfo] {return !isRunning || threadinfo->willTerminate ||!taskQueue.empty();});
			//如果线程被终止或者线程池停止且没有任务，退出线程
			if (threadinfo->willTerminate || (!isRunning && taskQueue.empty()))return;
			//从队列中取出任务
			task = std::move(taskQueue.front().taskFunc);
			//获取任务ID
			threadinfo->taskID = taskQueue.front().taskID;
			taskQueue.pop();
		}
		if (!task) {
			ErrorMessageBox(NULL, _T("任务函数为空"));
			continue;
		}
		//执行任务
		threadinfo->isWorking = true;
		task();
		threadinfo->isWorking = false;
		threadinfo->taskID = 0;
	}
}

void ThreadPool::ShutDownThreadPool()
{
	{//修改运行标志，通知所有线程
		std::unique_lock<std::mutex> lock(queueMutex);
		isRunning = false;
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
	std::queue<TaskInfo> tempTaskQueue(taskQueue);
	lock.unlock();
	//循环判断队列中是否有任务
	while (!tempTaskQueue.empty()) {
		if (tempTaskQueue.front().taskID == taskID)return true;
		tempTaskQueue.pop();
	}
	//检查是否有线程在执行该任务
	for (auto& worker : workThreads) {
        if (worker->taskID == taskID)return true;
	}
	return false;
}

void ThreadPool::WaitTask(size_t taskID)
{
	ThreadInfo* temp = FindThread(std::this_thread::get_id());
	std::thread::id threadid;
	if (temp) {
		auto currentThreadInfo = std::make_unique<ThreadInfo>(nullptr, false, false, 0, this);
		//使线程与线程信息互相关联建立联系
		currentThreadInfo->mThread = new std::thread(&ThreadPool::ThreadLoop, this, currentThreadInfo.get());
		currentThreadInfo->threadID = currentThreadInfo->mThread->get_id();
		threadid = currentThreadInfo->threadID;
		//转移所有权到容器内
		std::lock_guard<std::mutex> lock(threadMutex);
		workThreads.emplace_back(std::move(currentThreadInfo));
	}
	while (true) {
		for (size_t i = 0; i < waitTasks.size(); ++i) {
			if (waitTasks[i].first == taskID) {
				if (!waitTasks[i].second)continue;
				waitTasks[i].second->wait();
				std::lock_guard<std::mutex> lock(waitTaskMutex);
				//二次判断
				if (i < waitTasks.size() && waitTasks[i].first == taskID && waitTasks[i].second->_Is_ready()) {
					waitTasks.erase(waitTasks.begin() + i);
					--i;//修正索引值
				}
			}
		}
		//二次验证重新遍历以保证行为安全
		std::lock_guard<std::mutex> lock(waitTaskMutex);
		if (find_if(waitTasks.begin(), waitTasks.end(), [taskID](const std::pair<size_t, std::unique_ptr<FutureWrapperBase>>& waittask) {return waittask.first == taskID; }) == waitTasks.end()) {
			//未找到直接跳出循环
			break;
		}
		//二次验证时若还有对应ID的任务未完成,说明第一次遍历可能出现错误,继续循环
	}
	if (temp) {
		//销毁刚刚创建的那个线程
		std::thread(&ThreadPool::DestroyThread, this, threadid).detach();
	}
}

