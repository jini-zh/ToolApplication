#ifndef DIGITIZER_H
#define DIGITIZER_H

#include <functional>
#include <map>
#include <thread>
#include <vector>

#include "DataModel.h"
#include "ThreadLoop.h"
#include "Tool.h"

template <typename Packet, typename Hit>
class Digitizer: public ToolFramework::Tool {
  public:
    bool Initialise(std::string configfile, DataModel& data);
    bool Execute();
    bool Finalise();

  protected:
    using Event = std::deque<Hit>;
    // Connects and configures the boards. Returns the number of boards and the
    // total number of channels across all boards. This method is called from
    // Initialise and is wrapped in a try-catch block.
    virtual void init(unsigned& nboards) = 0;

    // This method is called from Finalise and is wrapped in a try-catch block.
    virtual void fini() {};

    // Reads board `board_index` and fills `board_data` with its data. This
    // method is called from the readout thread and shall be as fast as
    // possible.
    virtual void readout(
        unsigned board_index, std::vector<Packet>& board_data
    ) = 0;

    // Processes the data read from board `board_index`. The data shall be
    // converted to hits and stored in the result of `get_event` which takes an
    // event number and returns a vector of hits, one hit per channel for all
    // channels of all boards. Use `board_index` to calculate the offset for
    // the boards' hits.
    // This method if called from the worker job queues
    virtual void process(
      size_t                                  cycle,
      const std::function<Event& (uint32_t)>& get_event,
      unsigned                                board_index,
      std::vector<Packet>                     board_data
    ) = 0;

    // Sends the events designated by the iterators down the processing chain.
    // Events data should be moved out of the map; map nodes are erased after
    // the call to this function. The events are ordered by the event number
    // and this method will be called for event chunks in the same order.
    virtual void submit(
        typename std::map<uint32_t, Event>::iterator begin,
        typename std::map<uint32_t, Event>::iterator end
    ) = 0;

  private:
    struct Readout {
      std::vector<std::vector<Packet>> data;
      size_t                           cycle;
    };

    unsigned nboards   = 0;

    ThreadLoop::Thread readout_thread;

    size_t readout_cycle; // number of the last readout cycle
    size_t process_cycle; // number of the next cycle to be submitted
    bool last_readout_empty;

    // `cycles` keys is a set of cycles that have been stored in `events` but
    // not submitted yet. `cycles` values are the numbers of the latest events
    // that are safe to write to the DataModel provided that:
    // for cycles[i].first:  the previous cycles (with index less than i)
    //                       have been processed;
    // for cycles[i].second: both the previous cycles and the next cycle have
    //                       been processed.
    // cycles[i].second is the largest event number seen during processing of this cycle.
    // cycles[i].first the smallest event number across largest event number per board.
    std::map<size_t, std::pair<uint32_t, uint32_t>> cycles;
    std::mutex cycles_mutex;


    std::map<uint32_t, Event> events;
    std::mutex                events_mutex;

    void readout();
    bool process(Readout);
};

template <typename Packet, typename Hit>
void Digitizer<Packet, Hit>::readout() {
  std::vector<std::vector<Packet>> data(nboards);
  bool empty = true;
  for (unsigned i = 0; i < nboards; ++i) {
    readout(i, data[i]);
    empty = empty && data[i].empty();
  };

  // If this readout is empty and the previous readout wasn't empty, send empty
  // data for processing so that the previous readout could be submitted

  if (empty && last_readout_empty) return;
  last_readout_empty = empty;

  using arg_t = std::pair<Digitizer<Packet, Hit>&, Readout>;

  Job* job = new Job("Digitizer::readout");
  job->func = [](void* job_data) -> bool {
    auto arg = static_cast<arg_t*>(job_data);
    bool result = arg->first.process(std::move(arg->second));
    delete arg;
    return result;
  };
  job->data = new arg_t { *this, { std::move(data), readout_cycle++ } };

  m_data->job_queue.AddJob(job);
};

template <typename Packet, typename Hit>
bool Digitizer<Packet, Hit>::process(Readout readout) {
  try {
    uint32_t max_event[nboards];
    std::memset(max_event, 0xFF, sizeof(max_event));

    for (unsigned iboard = 0; iboard < nboards; ++iboard)
      process(
          readout.cycle,
          [&](uint32_t ievent) -> Event& {
            // XXX: assuming no overflow of the event number
            if (max_event[iboard] == ~0U || max_event[iboard] < ievent)
              max_event[iboard] = ievent;
            std::lock_guard<std::mutex> events_lock(events_mutex);
            auto pevent = events.lower_bound(ievent);
            if (pevent == events.end() || pevent->first != ievent)
              pevent = events.insert(pevent, { ievent, Event() });
            return pevent->second;
          },
          iboard,
          std::move(readout.data[iboard])
      );

    // Events read during the last readout cycle may be incomplete since the
    // boards are read sequentially at different times. We keep track of the
    // last event read for each board in max_event; we can submit events that
    // have the numbers less than the smallest stored in max_event _and_ each
    // preceding readout cycle has been processed.

    uint32_t min_max_event = ~0U;
    uint32_t max_max_event = ~0U;
    int i = 0;
    for (; i < nboards; ++i)
      if (max_event[i] != ~0U) {
        min_max_event = max_max_event = max_event[i];
        break;
      };
    for (++i; i < nboards; ++i)
      if (max_event[i] != ~0U) {
        if (max_event[i] < min_max_event)
          min_max_event = max_event[i];
        else if (max_event[i] > max_max_event)
          max_max_event = max_event[i];
      };

    std::lock_guard<std::mutex> cycles_lock(cycles_mutex);

    // Note that for an empty readout we store ~0U as both event numbers
    cycles[readout.cycle] = std::pair<uint32_t, uint32_t> {
      min_max_event, max_max_event
    };

    auto done = cycles.begin();
    if (done->first != process_cycle)
      // the next sequential cycle is not processed yet
      return true;

    // look for a gap in the cycle numbers
    size_t pcycle = process_cycle;
    while (++done != cycles.end() && done->first == ++pcycle);
    --done;

    // Now [ cycles.begin(); done ] is a continuous sequence of cycles that
    // have been processed. Only the last cycle in this sequence may have
    // incomplete events.

    if (done == cycles.begin()) {
      if (done->second.first == ~0U) cycles.erase(done);
      return true;
    };

    uint32_t last_ievent = done->second.first;
    for (auto cycle = done; last_ievent == ~0U && cycle-- != cycles.begin();)
      last_ievent = cycle->second.second;
    if (last_ievent != ~0U) {
      std::lock_guard<std::mutex> events_lock(events_mutex);
      auto last_event = events.upper_bound(last_ievent);
      submit(events.begin(), last_event);
      events.erase(events.begin(), last_event);
    };
    cycles.erase(cycles.begin(), done);
    process_cycle = pcycle;

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

template <typename Packet, typename Hit>
bool Digitizer<Packet, Hit>::Initialise(std::string configfile, DataModel& data) {
  try {
    if (configfile != "") m_variables.Initialise(configfile);

    m_data = &data;
    m_log  = m_data->Log;

    if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

    init(nboards);
    readout_cycle = 0;
    process_cycle = 0;
    last_readout_empty = true;

    return true;
  } catch (std::exception& e) {
    if (m_log)
      *m_log << ML(0) << e.what() << std::endl;
    else
      fprintf(stderr, "%s\n", e.what());
    return false;
  };
};

template <typename Packet, typename Hit>
bool Digitizer<Packet, Hit>::Execute() {
  if (nboards == 0) return true;
  try {
    readout_thread = m_data->vme_readout.add(
        [this]() -> bool {
          try {
            readout();
            return true;
          } catch (std::exception& e) {
            *m_log << ML(0) << e.what() << std::endl;
            return false;
          }
        }
    );

    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

template <typename Packet, typename Hit>
bool Digitizer<Packet, Hit>::Finalise() {
  try {
    if (readout_thread.alive()) readout_thread.terminate();
    fini();
    return true;
  } catch (std::exception& e) {
    *m_log << ML(0) << e.what() << std::endl;
    return false;
  };
};

#endif
