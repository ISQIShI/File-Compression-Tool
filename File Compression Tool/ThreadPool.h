#pragma once
#include<vector>
#include<queue>
#include<future>
#include <windows.h>
#include <tchar.h>
//#include <windows.h>
//#include<thread>
//#include<mutex>
//#include<atomic>
//#include<functional>
//#include<condition_variable>

//前向声明
class ThreadPool;

struct ThreadInfo {
	std::thread* mThread;//指向关联线程的指针
	bool isWorking;//线程是否正在工作
	size_t taskID;//正在执行的任务ID
    ThreadPool* threadPool; //关联线程属于的线程池的指针
	//构造函数
	ThreadInfo(std::thread* mthread, bool isworking, size_t taskid, ThreadPool* threadpool)
		:mThread(mthread), isWorking(isworking), taskID(taskid), threadPool(threadpool) {
	}
};

struct TaskInfo {
    size_t taskID;
	std::function<void()> taskFunc;
};

class ThreadPool {
	std::vector<ThreadInfo> workThreads;//工作线程容器
	std::queue<TaskInfo> taskQueue;//任务队列
	std::mutex queueMutex;//任务队列的互斥锁
	std::condition_variable threadWaitTask;//条件变量,用于使线程休眠等待任务
	std::mutex completeMutex;
	std::condition_variable completeTask;//条件变量,用于通知某任务已经完成
	size_t waitTaskID;//等待的任务的ID
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
	explicit ThreadPool(size_t threadAmount = std::thread::hardware_concurrency()): isRunning(true),waitTaskID(0) {
		if (threadAmount == 0) threadAmount = 1;
		// 创建指定数量的线程
		ThreadInfo* currentThreadInfo = nullptr;
		for (size_t i = 0; i < threadAmount; ++i) {
			currentThreadInfo = &workThreads.emplace_back(nullptr, false, 0, this);
			//使线程与线程信息互相关联建立联系
			currentThreadInfo->mThread = new std::thread(&ThreadPool::ThreadLoop,this,currentThreadInfo);
		}
	}
	// 提交任务到线程池
	template <class F, class... Args>
	auto SubmitTask(size_t taskID,F&& f, Args&&... args)-> std::future<std::invoke_result_t<F, Args...>>
	{//使用尾置返回类型和自动推导函数返回类型
		using ReturnType = std::invoke_result_t<F, Args...>;
		//将任务封装为一个可调用的std::function对象
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<ReturnType> result = task->get_future(); //获取任务函数的返回值future
		{ // 进入临界区，保护任务队列
			std::unique_lock<std::mutex> lock(queueMutex);
			// 如果线程池已经停止，抛出异常
			if (!isRunning)throw std::runtime_error("线程池已经停止运行");
			// 将任务加入队列
			taskQueue.emplace(TaskInfo{ taskID,[task]() { (*task)(); } });
		}
		//通知一个线程有新任务
		threadWaitTask.notify_one();
		return result; //返回future对象，即任务的返回值
	}
	//停止线程池并等待所有线程完成
	void ShutDownThreadPool();
	//查找任务队列中是否存在某个任务
    bool FindTask(size_t taskID);
	//阻塞调用方线程，直到具有 该任务ID 的任务全部完成,暂时只支持单线程
	void WaitTask(size_t taskID);
};
