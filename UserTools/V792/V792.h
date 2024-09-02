#ifndef V792_H
#define V792_H

#include <memory>

#include <caen++/v792.hpp>

#include "Digitizer.h"
#include "QDCHit.h"

class V792: public Digitizer<caen::V792::Packet, QDCHit> {
  private:
    struct Board {
      caen::V792 qdc;
      unsigned event_map[32];
    };

    std::vector<Board> boards;
    caen::V792::Buffer buffer;

    void connect();
    void configure();

    void init(unsigned& nboards) final;

    void fini() final;

    void readout(
        unsigned                         qdc_index,
        std::vector<caen::V792::Packet>& data
    ) final;

    void process(
        size_t                                  cycle,
        const std::function<Event& (uint32_t)>& get_event,
        unsigned                                qdc_index,
        std::vector<caen::V792::Packet>         qdc_data
    ) final;

    void submit(
        std::map<uint32_t, Event>::iterator begin,
        std::map<uint32_t, Event>::iterator end
    ) final;
};

#endif
