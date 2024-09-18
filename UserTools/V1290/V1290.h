#ifndef V1290_H
#define V1290_H

#include <list>
#include <map>
#include <memory>
#include <vector>

#include <caen++/v1290.hpp>

#include "Digitizer.h"
#include "TDCHit.h"

class V1290: public Digitizer<caen::V1290::Packet, TDCHit> {
  private:
    struct Board {
      caen::V1290 tdc;
      uint16_t errors;
    };

    struct RawEvent {
      caen::V1290::GlobalHeader           header;
      caen::V1290::GlobalTrailer          trailer;
      caen::V1290::ExtendedTriggerTimeTag ettt;
      caen::V1290::TDCMeasurement         hits[32];
      int                                 nhits;

      void merge(RawEvent& event, bool tail);
    };

    std::vector<Board> boards;
    std::mutex tdc_errors_mutex;
    caen::V1290::Buffer buffer;
    std::map<size_t, RawEvent> chops;
    std::mutex chops_mutex;

    bool chop_event(size_t cycle, RawEvent&, bool head);

    void connect();
    void configure() final;

    void init(unsigned& nboards, VMEReadout<TDCHit>*& output) final;
    void fini() final;

    void start_acquisition() final;
    void stop_acquisition()  final;

    void process(
        size_t                                  cycle,
        const std::function<Event& (uint32_t)>& get_event,
        unsigned                                tdc_index,
        std::vector<caen::V1290::Packet>        tdc_data
    ) final;

    void process(
        const std::function<Event& (uint32_t)>& get_event,
        RawEvent&
    );

    void readout(
        unsigned                          tdc_index,
        std::vector<caen::V1290::Packet>& data
    ) final;

    void report_error(unsigned tdc_index, caen::V1290::TDCError);
};

#endif
