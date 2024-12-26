#include"ThreadPool.h"

void ThreadPool::ThreadLoop(ThreadInfo* threadinfo)
{
	if (threadinfo == nullptr) {
		ErrorMessageBox(NULL, _T("线程信息指针为空"));
		return;
	}
	//每个线程的工作函数
	while (true) {
		std::function<void()> task; //用于存储取出的任务
		{ //进入临界区，保护任务队列(共享数据)被正常读写
			std::unique_lock<std::mutex> lock(queueMutex);
			//线程休眠，直到有可用任务或线程池停止运行再唤醒
			threadWaitTask.wait(lock, [this] {return !isRunning || !taskQueue.empty(); });
			//如果线程池停止且没有任务，退出线程
			if (!isRunning && taskQueue.empty())return;
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
		//如果完成的任务与等待任务ID一致,发送通知
		if (threadinfo->taskID == waitTaskID)completeTask.notify_all();
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
		if (worker.mThread && worker.mThread->joinable()) {
			worker.mThread->join(); // 等待线程结束
			delete worker.mThread;
			worker.mThread = nullptr;
		}
	}
}

bool ThreadPool::FindTask(size_t taskID)
{
	//保护任务队列
	std::unique_lock<std::mutex> lock(queueMutex);
	std::queue<TaskInfo> tempTaskQueue(taskQueue);
	lock.unlock();

	while (!tempTaskQueue.empty()) {
		if (tempTaskQueue.front().taskID == taskID)return true;
		tempTaskQueue.pop();
	}
	return false;
}

void ThreadPool::WaitTask(size_t taskID)
{
	waitTaskID = taskID;
	std::unique_lock<std::mutex> lock(completeMutex);
	completeTask.wait(lock, [this,taskID]() {return !FindTask(taskID); });
	waitTaskID = 0;
}

