#include "mythreads.h"

namespace ami {
namespace threads {

void task::send_signal() {
  this->pool->receive_signal(this->id);
}
void task::pool_method() {
  this->method(this->thread_handle->is_working);
  this->send_signal();
}

threads_pool::threads_pool(int threads_count) {
  for(int i = 0; i < threads_count; i++) {
    thread* t = new thread;
    t->handle = std::thread(&threads_pool::run, this, t);
    t->is_working = false;
    threads.push_back(t);
  }
}

threads_pool::~threads_pool() {
  this->stopped = true;
  tasks_access.notify_all();
	for (auto& t : threads) {
    t->is_working = false;
		t->handle.join();
		delete t;
	}
}

bool threads_pool::run_allowed() const {
	return (!task_queue.empty() && !paused);
}

void threads_pool::run(thread* _thread) {
  while(!stopped) {
    std::unique_lock<std::mutex> lock(task_queue_mutex);

    _thread->is_working = false;
    tasks_access.wait(lock, [this]() { return run_allowed() || stopped; });
    _thread->is_working = true;

    if (run_allowed()) {
      auto task = std::move(task_queue.front());
      task.get()->thread_handle = _thread;
      task_queue.pop();
      lock.unlock();

      task->pool_method();
      task.get()->thread_handle = nullptr;
    }

    wait_access.notify_all();
  }
}

void threads_pool::start() {
  if (paused) {
		paused = false;
		// Даеn всем потокам разрешающий сигнал для доступа к очереди невыполненных задач
		tasks_access.notify_all();
	}
}
void threads_pool::stop() {
  paused = true;
}

void threads_pool::receive_signal(taskid_t id) {
  std::lock_guard<std::mutex> lock(signal_queue_mutex);
	signal_queue.emplace(id);
	if (!ignore_signals) stop();
}

bool threads_pool::is_completed() const {
  return task_queue.empty();
}
bool threads_pool::is_standby() const {
  if(!paused) return false;
  for (const auto& t : threads)
		if (t->is_working) return false;
	
  return true;
}


taskid_t threads_pool::add_task(const task& _task) {
  std::lock_guard<std::mutex> lock(task_queue_mutex);
  task_queue.push(std::make_unique<task>(_task));
  
  task_queue.back()->id = ++last_task_id;
  task_queue.back()->pool = this;
  tasks_access.notify_one();

  return last_task_id;
}

void threads_pool::wait() {
  std::lock_guard<std::mutex> lock_wait(wait_mutex);

	start();

	std::unique_lock<std::mutex> lock(task_queue_mutex);
	wait_access.wait(lock, [this]() { return this->is_completed(); });

	stop();
}

taskid_t threads_pool::wait_signal() {
  std::lock_guard<std::mutex> lock_wait(wait_mutex);

  this->ignore_signals = false;

  signal_queue_mutex.lock();
	if (signal_queue.empty()) start();
	else stop();
	signal_queue_mutex.unlock();

  std::unique_lock<std::mutex> lock(task_queue_mutex);
	wait_access.wait(lock, [this]() { return is_completed() || is_standby(); });

  ignore_signals = true;

  std::lock_guard<std::mutex> lock_signals(signal_queue_mutex);
	if (signal_queue.empty()) return 0;
	else {
		taskid_t signal = std::move(signal_queue.front());
		signal_queue.pop();
		return signal;
	}
}

}
}