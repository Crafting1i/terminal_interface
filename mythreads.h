#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <functional>
#include <memory>

namespace threads {
  using taskid_t = size_t;
	using taskcb_t = std::function<void(std::atomic<bool>&)>;
	class threads_pool;

  struct thread {
    std::thread handle;
    std::atomic<bool> is_working;
  };
  class task {
    friend class threads_pool;
  private:
    taskid_t id = 0;
		threads_pool* pool = nullptr;
		thread* thread_handle = nullptr;

    void pool_method();
	public:
    taskcb_t method;
  public:
    task();
		task(taskcb_t m): method(m) {};
    void send_signal();

  };

  class threads_pool {
    friend void task::send_signal();
  public:
		explicit threads_pool(int threads_count);
		taskid_t add_task(const task& task);

		// ожидание полной обработки текущей очереди задач или приостановки,
		// возвращает id задачи, которая первая подала сигнал и 0 иначе
		taskid_t wait_signal();

		// ожидание полной обработки текущей очереди задач, игнорируя любые
		// сигналы о приостановке
		void wait();

		// приостановка обработки
		void stop();

		// возобновление обработки
		void start();

		~threads_pool();
  private:
		// мьютексы, блокирующие очереди для потокобезопасного обращения
		std::mutex task_queue_mutex;
		std::mutex completed_tasks_mutex;
		std::mutex signal_queue_mutex;

		// мьютекс, блокирующий логер для последовательного вывода
		std::mutex logger_mutex;

		// мьютекс, блокирующий функции ожидающие результаты (методы wait*)
		std::mutex wait_mutex;

		std::condition_variable tasks_access;
		std::condition_variable wait_access;

		// набор доступных потоков
		std::vector<thread*> threads;

		// очередь задач
		std::queue<std::unique_ptr<task>> task_queue;
		taskid_t last_task_id = 0;

		// массив выполненных задач в виде хэш-таблицы
		std::unordered_map<taskid_t, task> completed_tasks;

		std::queue<taskid_t> signal_queue;

		// флаг остановки работы пула
		std::atomic<bool> stopped = false;
		// флаг приостановки работы
		std::atomic<bool> paused = true;
		std::atomic<bool> ignore_signals = true;
  private:
		// основная функция, инициализирующая каждый поток
		void run(thread* thread);

		// приостановка обработки c выбросом сигнала
		void receive_signal(taskid_t id);

		// разрешение запуска очередного потока
		bool run_allowed() const;

		// проверка выполнения всех задач из очереди
		bool is_completed() const;

		// проверка, занятости хотя бы одного потока
		bool is_standby() const;
  };
}