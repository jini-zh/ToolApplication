#include "ThreadLoop.h"

ThreadLoop::~ThreadLoop() {
  if (!thread) return;
  stop_ = true;
  thread->join();
  delete thread;
};

void ThreadLoop::clear() {
  stop_ = true;
  if (thread) {
    thread->join();
    delete thread;
  };
  actions.clear();
  subscriptions.clear();
  current = actions.end();
};

void ThreadLoop::lock() {
  if (!mutex.try_lock()) {
    pause_ = true;
    mutex.lock();
  };
};

void ThreadLoop::unlock() {
  mutex.unlock();
  if (pause_) {
    pause_ = false;
    cv.notify_all();
  };
};

ThreadLoop::handle ThreadLoop::subscribe(std::function<bool ()> function) {
  lock();
  try {
    actions.push_front({ std::move(function), nullptr });
    subscriptions.push_front({ actions.begin(), false });
    actions.front().subscription = &subscriptions.front();

    unlock();
  } catch (...) {
    unlock();
    throw;
  };

  if (thread && stop_) {
    thread->join();
    delete thread;
    thread = nullptr;
  };

  if (!thread) {
    stop_ = false;
    thread = new std::thread(&ThreadLoop::loop, this);
  };

  return subscriptions.begin();
};

void ThreadLoop::loop() {
  std::unique_lock<std::mutex> lock(mutex);
  while (true) {
    if (pause_) {
      paused_ = true;
      cv.notify_all();
      cv.wait(lock, [this]() -> bool { return !pause_; });
      paused_ = false;
    };

    if (stop_) return;

    if (current == actions.end()) {
      current = actions.begin();
      if (current == actions.end()) return;
    };

    lock.unlock();
    bool keep = current->function();
    lock.lock();
    if (keep)
      ++current;
    else {
      current->subscription->erased = true;
      actions.erase(current++);
    };
  };
};

void ThreadLoop::unsubscribe(ThreadLoop::handle handle) {
  if (!handle->erased) {
    lock();
    try {
      if (current == handle->iterator && !paused_) {
        pause_ = true;
        std::unique_lock<std::mutex> lock(mutex, std::adopt_lock);
        cv.wait(lock, [this]() -> bool { return paused_; });
        lock.release();
      };

      if (current == handle->iterator) ++current;

      actions.erase(handle->iterator);

      unlock();
    } catch (...) {
      unlock();
      throw;
    };
  };

  subscriptions.erase(handle);

  if (actions.empty() && thread) {
    thread->join();
    delete thread;
    thread = nullptr;
  };
};
