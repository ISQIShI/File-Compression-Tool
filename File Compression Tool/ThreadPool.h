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
	std::vector<std::thread> workThreads;//�����߳�����
	std::queue<std::function<void()>> taskQueue;//�������
	std::mutex queue_mutex;//������еĻ�����
	std::condition_variable condition;//��������������֪ͨ�߳̽�������
	std::atomic<bool> isRunning;//�̳߳�����״̬

	//��ֹ��������͸�ֵ����
	ThreadPool(const ThreadPool&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;

public:
	//�����������Զ�ֹͣ�̳߳�
	~ThreadPool() {
		shutdown();
	}
	//���캯������ʼ���̳߳أ�ͬʱʹ��explicit��ֹ��ʽ����ת��
	explicit ThreadPool(size_t thread_count): isRunning(true) {
		// ����ָ���������߳�
		for (size_t i = 0; i < thread_count; ++i) {
			workThreads.emplace_back([this] {
				// ÿ���̵߳Ĺ�������
				while (true) {
					std::function<void()> task; // ���ڴ洢ȡ��������
					{ // �����ٽ����������������
						std::unique_lock<std::mutex> lock(queue_mutex);
						// �ȴ�������̳߳�ֹͣ�ź�
						condition.wait(lock, [this] {
							return !isRunning || !taskQueue.empty();
							});
						// ����̳߳�ֹͣ��û�������˳��߳�
						if (!isRunning && taskQueue.empty())
							return;
						// �Ӷ�����ȡ������
						task = std::move(taskQueue.front());
						taskQueue.pop();
					}
					// ִ������
					task();
				}
				});
		}
	}
	// �ύ�����̳߳�
	template <class F, class... Args>
	auto submit(F&& f, Args&&... args)-> std::future<std::invoke_result_t<F, Args...>> 
	{//ʹ��β�÷������ͺ��Զ��Ƶ�������������
		using ReturnType = std::invoke_result_t<F, Args...>;
		// �������װΪһ���ɵ��õ�std::function����
		auto task = std::make_shared<std::packaged_task<ReturnType()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<ReturnType> result = task->get_future(); // ��ȡ���񷵻�ֵ��future
		{ // �����ٽ����������������
			std::unique_lock<std::mutex> lock(queue_mutex);
			// ����̳߳��Ѿ�ֹͣ���׳��쳣
			if (!isRunning)
				throw std::runtime_error("ThreadPool has stopped");
			// ������������
			taskQueue.emplace([task]() { (*task)(); });
		}
		// ֪ͨһ���߳���������
		condition.notify_one();
		return result; // ����future����
	}
	// ֹͣ�̳߳ز��ȴ������߳����
	void shutdown() {
		{ // �޸����б�־��֪ͨ�����߳�
			std::unique_lock<std::mutex> lock(queue_mutex);
			isRunning = false;
		}
		condition.notify_all(); // ���������߳�
		for (std::thread& worker : workThreads) {
			if (worker.joinable())
				worker.join(); // �ȴ��߳̽���
		}
	}
};

int main() {
	ThreadPool pool(4); // �����̳߳أ�����4���߳�

	// �ύ�����̳߳�
	auto future1 = pool.submit([](int a, int b) {
		return a + b;
		}, 5, 10);

	auto future2 = pool.submit([] {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		return std::string("�������");
		});

	// ��ȡ������
	/*std::cout << "����1���: " << future1.get() << std::endl;
	std::cout << "����2���: " << future2.get() << std::endl;*/

	return 0; // ���߳��˳�ʱ���̳߳��Զ�ֹͣ
}
