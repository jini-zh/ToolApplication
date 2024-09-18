#ifndef VME_READOUT_H
#define VME_READOUT_H

#include <deque>
#include <mutex>
#include <vector>

template <typename Hit>
class VMEReadout {
  public:
    template <typename Iterator>
    void push(Iterator begin, Iterator end);

    std::deque<std::vector<Hit>> get();
  private:
    std::mutex mutex;
    std::deque<std::vector<Hit>> readout;
};

template <typename Hit>
template <typename Iterator>
void VMEReadout<Hit>::push(Iterator begin, Iterator end) {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto i = begin; i != end; ++i) readout.push_back(std::move(*i));
};

template <typename Hit>
std::deque<std::vector<Hit>> VMEReadout<Hit>::get() {
  std::lock_guard<std::mutex> lock(mutex);
  return std::move(readout);
};


#endif
