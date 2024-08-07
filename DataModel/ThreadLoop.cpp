#include "ThreadLoop.h"

ThreadLoop::~ThreadLoop() {
  if (!os_thread.joinable()) return;
  stop = true;
  os_thread.join();
};

void ThreadLoop::lock() {
  if (!mutex.try_lock()) {
    pause = true;
    mutex.lock();
  };
};

void ThreadLoop::unlock() {
  mutex.unlock();
  if (pause) {
    pause = false;
    cv.notify_all();
  };
};

std::list<ThreadLoop::Subscription>::iterator
ThreadLoop::add_(std::function<bool ()> function) {
  lock();
  std::list<Subscription>::iterator subscription;
  try {
    tasks.push_front({ std::move(function), nullptr });
    subscriptions.push_front({ tasks.begin(), false });
    tasks.front().subscription = &subscriptions.front();
    subscription = subscriptions.begin();
    unlock();
  } catch (...) {
    unlock();
    throw;
  };

  if (stop && os_thread.joinable()) os_thread.join();

  if (!os_thread.joinable()) {
    stop = false;
    os_thread = std::thread(&ThreadLoop::loop, this);
  };

  return subscription;
};

void ThreadLoop::loop() {
  std::unique_lock<std::mutex> lock(mutex);
  while (true) {
    if (pause) {
      paused = true;
      cv.notify_all();
      cv.wait(lock, [this]() -> bool { return !pause; });
      paused = false;
    };

    if (stop) return;

    if (current == tasks.end()) {
      current = tasks.begin();
      if (current == tasks.end()) return;
    };

    lock.unlock();
    bool keep = current->function();
    lock.lock();

    if (keep)
      ++current;
    else {
      current->subscription->erased = true;
      tasks.erase(current++);
    };
  };
};

void ThreadLoop::remove(std::list<Subscription>::iterator subscription) {
  if (!subscription->erased) {
    auto task = subscription->task;
    lock();
    try {
      if (current == task && !paused) {
        pause = true;
        std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
        cv.wait(lock, [this]() -> bool { return paused; });
        lock.release();
      };

      if (current == task) ++current;

      tasks.erase(task);

      unlock();
    } catch (...) {
      unlock();
      throw;
    };
  };

  subscriptions.erase(subscription);

  if (tasks.empty() && os_thread.joinable()) os_thread.join();
};
