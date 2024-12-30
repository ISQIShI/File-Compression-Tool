#pragma once
#include<future>
#include<queue>
#include <tchar.h>
#include <utility>
#include<vector>
#include <windows.h>
#include <shared_mutex>

//前向声明
class ThreadPool;


struct TaskInfo {
	size_t taskID;
	bool needWait;
	std::function<void()> taskFunc;
	TaskInfo(size_t taskid, bool needwait, std::function<void()>&& taskfunc)
		:taskID(taskid), needWait(needwait), taskFunc(std::move(taskfunc)) {
	}
};

struct ThreadInfo {
	std::thread* mThread;//指向关联线程的指针
	std::atomic<bool> willTerminate;//线程是否要终止
	TaskInfo * taskInfo;//正在执行的任务信息
	std::thread::id	threadID;//线程ID
    ThreadPool* threadPool; //关联线程属于的线程池的指针
	//构造函数
	ThreadInfo(std::thread* mthread, bool willterminate,TaskInfo * taskinfo ,ThreadPool* threadpool)
		:mThread(mthread),willTerminate(willterminate),taskInfo(taskinfo), threadPool(threadpool) { }
};

class ThreadPool {
	std::atomic<size_t> workThreadAmount;//工作线程数量
	std::vector<std::unique_ptr<ThreadInfo>> workThreads;//工作线程容器
	std::queue<TaskInfo*> taskQueue;//任务队列
	std::mutex queueMutex;//任务队列的互斥锁
	std::mutex threadMutex;//线程的互斥锁
	std::condition_variable threadWaitTask;//条件变量,用于使线程休眠等待任务
	std::shared_mutex waitTaskMutex;//等待任务的互斥锁
	std::unordered_map<size_t,std::pair<std::atomic<size_t>,std::condition_variable_any >> waitTasks;
	std::unordered_map<size_t, TaskInfo*> callBackFuncs;//回调函数容器
	std::mutex callBackFuncMutex;//回调函数的互斥锁
	std::atomic<bool> isRunning;//线程池运行状态

	//禁止拷贝构造和赋值操作
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

public:
	void ErrorMessageBox(const HWND& hwnd, const TCHAR* msg, bool showErrorCode = true)
	{
		//创建字符串用于接收错误代码
		TCHAR errorCode[20] = _T("");
		if (showErrorCode) _stprintf_s(errorCode, _T("\n错误代码:%lu"), GetLastError());
		//将msg内容与错误代码拼接起来
		size_t length = _tcslen(msg) + _tcslen(errorCode) + 1;
		TCHAR* finalMsg = new TCHAR[length];
		_stprintf_s(finalMsg, length, _T("%s%s"), msg, errorCode);
		//弹出错误弹窗
		MessageBox(hwnd, finalMsg, _T("错误信息"), MB_OK | MB_ICONERROR | MB_TASKMODAL);
		//销毁使用new开辟的空间
		delete[]finalMsg;
		//退出程序
		exit(GetLastError());
	}

	//析构函数，自动停止线程池
	~ThreadPool() {	ShutDownThreadPool();}

	//线程循环
	void ThreadLoop(ThreadInfo* threadinfo);

	//构造函数，初始化线程池，同时使用explicit禁止隐式类型转换
	explicit ThreadPool(size_t threadAmount = std::thread::hardware_concurrency()): isRunning(true){
		//线程数为0则设定为1
		if (threadAmount == 0) threadAmount = 1;
		// 创建指定数量的线程
		for (size_t i = 0; i < threadAmount; ++i) {
			auto currentThreadInfo = std::make_unique<ThreadInfo>(nullptr, false, nullptr , this);
			//使线程与线程信息互相关联建立联系
			currentThreadInfo->mThread = new std::thread(&ThreadPool::ThreadLoop, this, currentThreadInfo.get());
			currentThreadInfo->threadID = currentThreadInfo->mThread->get_id();
			//转移所有权到容器内
			workThreads.emplace_back(std::move(currentThreadInfo));
		}
	}

	//提交任务到线程池
	template <class F, class... Args>
	auto SubmitTask(size_t taskID,bool needWait,F&& f, Args&&... args)-> std::future<std::invoke_result_t<F, Args...>>
	{//使用尾置返回类型和自动推导函数返回类型
		if (needWait) {
			std::shared_lock<std::shared_mutex> lock(waitTaskMutex);
			if (waitTasks.find(taskID) == waitTasks.end()) {
				lock.unlock();
				std::unique_lock<std::shared_mutex> uniquelock(waitTaskMutex);
				waitTasks[taskID].first.fetch_add(1);
			}
			else {
				waitTasks.at(taskID).first.fetch_add(1);
			}
		}
		using ReturnType = std::invoke_result_t<F, Args...>;
		//将任务封装为一个可调用的std::function对象
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<ReturnType> result = task->get_future(); //获取任务函数的返回值future
		{ // 进入临界区，保护任务队列
			std::lock_guard<std::mutex> lock(queueMutex);
			// 如果线程池已经停止，抛出异常
			if (!isRunning)throw std::runtime_error("线程池已经停止运行");
			// 将任务加入队列
			taskQueue.emplace(new TaskInfo(taskID,needWait ,[task]() { (*task)(); }));
		}
		//通知一个线程有新任务
		threadWaitTask.notify_one();

		return result; //返回future对象，即任务的返回值
	}
	//提交任务到线程池--提交任务信息
	template <class F, class... Args>
	void SubmitTask(TaskInfo * taskInfo)
	{
		{//进入临界区，保护任务队列
			std::lock_guard<std::mutex> lock(queueMutex);
			// 如果线程池已经停止，抛出异常
			if (!isRunning)throw std::runtime_error("线程池已经停止运行");
			// 将任务加入队列
			taskQueue.emplace(taskInfo);
		}
		//通知一个线程有新任务
		threadWaitTask.notify_one();
	}

	//停止线程池并等待所有线程完成
	void ShutDownThreadPool();

	//根据线程ID销毁一个线程池内已有的线程
	void DestroyThread(std::thread::id threadID);

	//根据线程ID查找线程是否存在线程池中
	ThreadInfo* FindThread(std::thread::id threadID);

	//查找任务队列中是否存在某个任务
    bool FindTask(size_t taskID);

	//阻塞调用方线程，直到具有 该任务ID 的任务全部完成，适用于需要等待的同ID的任务很多时
	void WaitTask(size_t taskID);

	//注册回调函数
	template <class F, class... Args>
	auto RegisterCallBackFunc(size_t taskID, F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
	{//使用尾置返回类型和自动推导函数返回类型
		using ReturnType = std::invoke_result_t<F, Args...>;
		//将任务封装为一个可调用的std::function对象
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<ReturnType> result = task->get_future(); //获取任务函数的返回值future
		{ // 进入临界区，保护回调函数容器
			std::lock_guard<std::mutex> lock(callBackFuncMutex);
			// 如果线程池已经停止，抛出异常
			if (!isRunning)throw std::runtime_error("线程池已经停止运行");
			// 将任务加入容器
			callBackFuncs.emplace(taskID,new TaskInfo(taskID, false, [task]() { (*task)(); }));
		}
		return result; //返回future对象，即任务的返回值
	}

	//获取线程总数
	size_t GetThreadTotalAmount()const { return workThreads.size(); }
	//获取工作线程数
    size_t GetWorkThreadAmount()const { return workThreadAmount.load(); }
	//获取任务队列中的任务数
    size_t GetTaskAmount()const { return taskQueue.size(); }
};
