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
  bool Send(zmq::socket_t* sock);
  bool Receive(zmq::socket_t* sock);
  unsigned int size() { return readout.size(); }
  
private:
  std::mutex mutex;
  std::deque<std::vector<Hit> > readout;
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

namespace VMEReadout_ {
  template <typename Hit> struct Header {};

  template <> struct Header<QDCHit> {
    static constexpr const char signature[] = "QDC";

  };

  template <> struct Header<TDCHit> {
    static constexpr const char signature[] = "TDC";
  };
};

template <typename Hit>
bool VMEReadout<Hit>::Send(zmq::socket_t* socket) {
  if (this->readout.empty()) return true;

  std::deque<std::vector<Hit>> readout;
  {
    std::lock_guard<std::mutex> lock(mutex);
    readout.swap(this->readout);
  };

  zmq::message_t header(4);
  memcpy(
      header.data(),
      VMEReadout_::Header<Hit>::signature,
      sizeof(VMEReadout_::Header<Hit>::signature) - 1
  );
  if (!socket->send(header, ZMQ_SNDMORE)) return false;

  auto event = readout.begin();
  while (event != readout.end()) {
    size_t size = sizeof(Hit) * event->size();
    zmq::message_t packet(size);
    memcpy(packet.data(), event->data(), size);
    if (!socket->send(packet, ++event == readout.end() ? 0 : ZMQ_SNDMORE))
      return false;
  };

  return true;
};

template <typename Hit>
bool VMEReadout<Hit>::Receive(zmq::socket_t* socket) {
  std::deque<std::vector<Hit>> readout;
  while (true) {
    zmq::message_t packet;
    if (!socket->recv(&packet)) return false;
    std::vector<Hit> event(packet.size() / sizeof(Hit));
    memcpy(event.data(), packet.data(), packet.size());
    readout.push_back(std::move(event));
    if (!packet.more()) break;
  };

  std::lock_guard<std::mutex> lock(mutex);
  for (auto& event : readout) this->readout.push_back(std::move(event));

  return true;
};
#endif
