#include "Store.h"
#include "DataModel.h"

#include "V792.h"
#include "caen.h"

void V792::connect() {
  auto connections = caen_connections(m_variables);
  boards.reserve(connections.size());
  for (auto& connection : connections) {
    caen_report_connection(*m_log << ML(3), "V792", connection);
    boards.emplace_back(Board { caen::V792(connection) });
  };
};

template <typename T>
static bool cfg_get(
    ToolFramework::Store& variables,
    std::string name,
    int index,
    T& var
) {
  std::stringstream ss;
  ss << name << '_' << index;
  return variables.Get(ss.str(), var) || variables.Get(std::move(name), var);
};

void V792::configure() {
  *m_log << ML(3) << "Configuring V792... " << std::flush;

  for (int qdc_index = 0; qdc_index < boards.size(); ++qdc_index) {
    Board& board = boards[qdc_index];
    caen::V792& qdc = board.qdc;
    qdc.reset();
    qdc.clear();

    bool  flag;
    int   i;
    float x;
    std::string s;
    std::string var;
#define cfgvar(name, var) \
    if (cfg_get(m_variables, #name, qdc_index, var)) qdc.set_ ## name(var)
#define cfgbool(name)  cfgvar(name, flag)
#define cfgint(name)   cfgvar(name, i)

    cfgint(geo_address);
    cfgint(interrupt_level);
    cfgint(interrupt_vector);

    {
      auto control = qdc.control1();
      auto old_control = control;
#define defbit(name) \
      if (m_variables.Get(#name, flag)) control.set_ ## name(flag)
      defbit(block_readout);
      defbit(panel_resets_software);
      if (m_variables.Get("enable_bus_error", flag))
        control.set_bus_error_enabled(flag);
      defbit(align_64);
#undef defbit
      if (old_control != control) qdc.set_control1(control);
    };

    cfgint(event_trigger);

    {
      auto bitset = qdc.bitset2();
      auto old_bitset = bitset;
#define defbit(name) \
      if (m_variables.Get("enable_" #name, flag)) \
        bitset.set_ ## name ## _enabled(flag)
      defbit(overflow);
      defbit(threshold);
      defbit(slide);
      if (m_variables.Get("shift_threshold", flag))
        bitset.set_shift_threshold(flag);
      defbit(empty);
      defbit(slide_subtraction);
      if (m_variables.Get("all_triggers", flag))
        bitset.set_all_triggers(flag);
#undef defbit
      if (old_bitset != bitset) qdc.set_bitset2(bitset);
    };

    cfgint(crate_number);
    cfgint(current_pedestal);
    cfgint(slide_constant);

    uint32_t mask = 0xFFFFFFFF;
    if (cfg_get(m_variables, "enable_channels", qdc_index, s)) {
      size_t j;
      mask = std::stol(s, &j, 16);
      if (j != s.size())
        throw std::runtime_error(
            std::string("V792: invalid value for enable_channels: ") + s
        );
    };
    std::stringstream ss;
    for (uint8_t channel = 0; channel < 32; ++channel) {
      ss.str({});
      ss << "channel_" << static_cast<int>(channel) << "_threshold";
      bool enable = mask & 1 << channel;
      if (cfg_get(m_variables, ss.str(), qdc_index, i))
        qdc.set_channel_settings(channel, i, enable);
      else
        qdc.set_channel_enabled(channel, enable);
    };
#undef cfgint
#undef cfgbool
#undef cfgvar
  };

  *m_log << ML(3) << "success" << std::endl;
};

void V792::init(unsigned& nboards) {
  connect();
  configure();
  nboards = boards.size();
};

void V792::fini() {
  boards.clear();
};

void V792::start_acquisition() {
  for(auto& board : boards) {
    board.qdc.clear();
    board.qdc.reset_event_counter();
  };
  Digitizer<caen::V792::Packet, QDCHit>::start_acquisition();
};

void V792::readout(unsigned qdc_index, std::vector<caen::V792::Packet>& data) {
  boards[qdc_index].qdc.readout_wa(buffer);

  // Find the first Invalid packet, or skip to the end of the buffer
  // TODO: move it to process?
  uint32_t n = buffer.size();
  if (n > 0 && buffer[n-1].type() == caen::V792::Packet::Invalid)
    if (buffer[0].type() == caen::V792::Packet::Invalid)
      n = 0;
    else {
      // binary search
      uint32_t m = 0;
      while (n - m > 1) {
        uint32_t k = (n + m) / 2;
        if (buffer[k].type() == caen::V792::Packet::Invalid)
          n = k;
        else
          m = k;
      };
    };

  data.insert(data.end(), buffer.begin(), buffer.begin() + n);
};

void V792::process(
    size_t                                  cycle,
    const std::function<Event& (uint32_t)>& get_event,
    unsigned                                qdc_index,
    std::vector<caen::V792::Packet>         qdc_data
) {
  if (qdc_data.empty()) return;

  if (qdc_data.front().type() != caen::V792::Packet::Header)
    // should never happen
    throw std::runtime_error("QDC: unexpected packet");

  std::vector<caen::V792::Packet>::iterator pheader;
  for (auto packet = qdc_data.begin(); packet != qdc_data.end(); ++packet)
    switch (packet->type()) {
      case caen::V792::Packet::Header:
        pheader = packet;
        break;

      case caen::V792::Packet::EndOfBlock: {
        Event& event = get_event(packet->as<caen::V792::EndOfBlock>().event());

        auto ptrailer = packet;
        for (packet = pheader + 1; packet != ptrailer; ++packet)
          if (packet->type() == caen::V792::Packet::Data)
            event.push_back(QDCHit(*pheader, *packet, *ptrailer));

        break;
      };
    };
};

void V792::submit(
    std::map<uint32_t, Event>::iterator begin,
    std::map<uint32_t, Event>::iterator end
) {
  std::lock_guard<std::mutex> readout_lock(m_data->v792_mutex);
  for (auto event = begin; event != end; ++event)
    m_data->v792_readout.push_back(std::move(event->second));
};
