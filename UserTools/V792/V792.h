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
      uint32_t (*readout)(Board&, caen::V792::Buffer&);
      int      vme_handle;
      uint32_t vme_address;
      unsigned event_map[32];
    };

    std::vector<Board> boards;
    caen::V792::Buffer buffer;

    void connect();
    void configure() final;

    void init(unsigned& nboards, VMEReadout<QDCHit>*& output) final;
    void fini() final;

    void start_acquisition() final;

    void readout(
        unsigned                         qdc_index,
        std::vector<caen::V792::Packet>& data
    ) final;

    static uint32_t readout_blt(Board&, caen::V792::Buffer&);
    static uint32_t readout_fifoblt(Board&, caen::V792::Buffer&);

    void process(
        size_t                                  cycle,
        const std::function<Event& (uint32_t)>& get_event,
        unsigned                                qdc_index,
        std::vector<caen::V792::Packet>         qdc_data
    ) final;
};

#endif
