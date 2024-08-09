#ifndef THREAD_LOOP_H
#define THREAD_LOOP_H

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

class ThreadLoop {
  private:
    struct Task;

    struct Subscription {
      std::list<Task>::iterator task;
      bool erased;
    };

    struct Task {
      std::function<bool ()> function;
      Subscription* subscription;
    };

  public:
    class Thread {
      public:
        Thread() {};
        Thread(Thread&& thread):
          loop(thread.loop), subscription(thread.subscription)
        {
          thread.loop = nullptr;
        };

        Thread(ThreadLoop& loop, std::function<bool ()> function):
          loop(&loop), subscription(loop.add_(std::move(function)))
        {};

        ~Thread() { terminate(); };

        Thread& operator=(Thread&& thread) {
          terminate();
          loop         = thread.loop;
          subscription = thread.subscription;
          thread.loop  = nullptr;
          return *this;
        };

        bool alive() const { return loop; };

        void terminate() {
          if (!loop) return;
          loop->remove(subscription);
          loop = nullptr;
        };

      private:
        ThreadLoop* loop = nullptr;
        std::list<Subscription>::iterator subscription;
    };

    ~ThreadLoop();

    Thread add(std::function<bool ()> function) {
      return Thread(*this, std::move(function));
    };

  private:
    std::list<Task> tasks;
    std::list<Task>::iterator current = tasks.end();
    std::list<Subscription> subscriptions;
    std::thread os_thread;
    std::mutex mutex;
    std::condition_variable cv;
    bool pause  = false;
    bool paused = false;
    bool stop   = false;

    std::list<Subscription>::iterator add_(std::function<bool ()>);
    void remove(std::list<Subscription>::iterator);

    void loop();
    void lock();
    void unlock();
};

#endif
