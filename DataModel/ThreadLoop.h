#ifndef THREAD_LOOP_H
#define THREAD_LOOP_H

#include <condition_variable>
#include <functional>
#include <list>
#include <mutex>
#include <thread>

class ThreadLoop {
  private:
    struct Action;

    struct Subscription {
      std::list<Action>::iterator iterator;
      bool erased;
    };

    struct Action {
      std::function<bool ()> function;
      Subscription* subscription;
    };

  public:
    using handle = std::list<Subscription>::iterator;

    ~ThreadLoop();

    handle subscribe(std::function<bool ()>);
    void unsubscribe(handle);

    void pause()  { pause_ = true;  };
    void resume() { pause_ = false; };
    bool paused() const { return pause_;  };

    void clear();

  private:
    std::list<Action> actions;
    std::list<Action>::iterator current = actions.end();
    std::list<Subscription> subscriptions;
    std::thread* thread = nullptr;
    std::mutex mutex;
    std::condition_variable cv;
    bool pause_  = false;
    bool paused_ = false;
    bool stop_   = false;

    void loop();
    void lock();
    void unlock();
};

#endif
