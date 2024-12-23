#pragma once
#include<vector>
#include<queue>
#include<condition_variable>
#include<future>
//#include<thread>
//#include<mutex>
//#include<atomic>
//#include<functional>

class ThreadPool {
private:
	std::vector<std::thread> workThreads;//工作线程容器
	std::queue<std::function<void()>> taskQueue;//任务队列
	std::mutex queue_mutex;//任务队列的互斥锁
	std::condition_variable condition;//条件变量，用于通知线程接收任务
	std::atomic<bool> isRunning;//线程池运行状态

	//禁止拷贝构造和赋值操作
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

public:
	//析构函数，自动停止线程池
	~ThreadPool() {
		shutdown();
	}
	//构造函数，初始化线程池，同时使用explicit禁止隐式类型转换
	explicit ThreadPool(size_t thread_count): isRunning(true) {
		// 创建指定数量的线程
		for (size_t i = 0; i < thread_count; ++i) {
			workThreads.emplace_back([this] {
				// 每个线程的工作函数
				while (true) {
					std::function<void()> task; // 用于存储取出的任务
					{ // 进入临界区，保护任务队列
						std::unique_lock<std::mutex> lock(queue_mutex);
						// 等待任务或线程池停止信号
						condition.wait(lock, [this] {
							return !isRunning || !taskQueue.empty();
							});
						// 如果线程池停止且没有任务，退出线程
						if (!isRunning && taskQueue.empty())
							return;
						// 从队列中取出任务
						task = std::move(taskQueue.front());
						taskQueue.pop();
					}
					// 执行任务
					task();
				}
				});
		}
	}
	// 提交任务到线程池
	template <class F, class... Args>
	auto submit(F&& f, Args&&... args)-> std::future<std::invoke_result_t<F, Args...>> 
	{//使用尾置返回类型和自动推导函数返回类型
		using ReturnType = std::invoke_result_t<F, Args...>;
		// 将任务封装为一个可调用的std::function对象
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<ReturnType> result = task->get_future(); // 获取任务返回值的future
		{ // 进入临界区，保护任务队列
			std::unique_lock<std::mutex> lock(queue_mutex);
			// 如果线程池已经停止，抛出异常
			if (!isRunning)
				throw std::runtime_error("ThreadPool has stopped");
			// 将任务加入队列
			taskQueue.emplace([task]() { (*task)(); });
		}
		// 通知一个线程有新任务
		condition.notify_one();
		return result; // 返回future对象
	}
	// 停止线程池并等待所有线程完成
	void shutdown() {
		{ // 修改运行标志，通知所有线程
			std::unique_lock<std::mutex> lock(queue_mutex);
			isRunning = false;
		}
		condition.notify_all(); // 唤醒所有线程
		for (std::thread& worker : workThreads) {
			if (worker.joinable())
				worker.join(); // 等待线程结束
		}
	}
};

int main() {
	ThreadPool pool(4); // 创建线程池，包含4个线程

	// 提交任务到线程池
	auto future1 = pool.submit([](int a, int b) {
		return a + b;
		}, 5, 10);

	auto future2 = pool.submit([] {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return std::string("任务完成");
		});

	// 获取任务结果
	/*std::cout << "任务1结果: " << future1.get() << std::endl;
	std::cout << "任务2结果: " << future2.get() << std::endl;*/

	return 0; // 主线程退出时，线程池自动停止
}
