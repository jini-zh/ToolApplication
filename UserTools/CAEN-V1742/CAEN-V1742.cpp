#include "CAEN-V1742.h"

static bool parse_hex(const std::string& string, unsigned& result) {
  char* end = NULL;
  result = strtol(string.c_str(), &end, 16);
  return !string.empty() && !*end;
};

void CAEN_V1742::log(int level, const char* message, va_list args) {
  va_list ap;
  va_copy(ap, args);

  const char* const prefix = "CAEN_V1742: ";
  static const int prefix_len = strlen(prefix);

  int size = vsnprintf(NULL, 0, message, ap);
  va_end(ap);
  if (size < 0) {
    Log(
        std::string(prefix) + "error while formatting message " + message,
        0,
        m_verbose
    );
    return;
  };

  char* msg = new char[prefix_len + size + 1];
  memcpy(msg, prefix, prefix_len);
  try {
    vsnprintf(msg + prefix_len, size + 1, message, args);
    Log(msg, level, m_verbose);
  } catch (...) {
    delete[] msg;
    throw;
  };
  delete[] msg;
}

void CAEN_V1742::log(int level, const char* message, ...) {
  va_list args;
  va_start(args, message);
  log(level, message, args);
  va_end(args);
}

void CAEN_V1742::error(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log(0, message, args);
  va_end(args);
}

void CAEN_V1742::warn(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log(1, message, args);
  va_end(args);
}

void CAEN_V1742::info(const char* message, ...) {
  va_list args;
  va_start(args, message);
  log(2, message, args);
  va_end(args);
}

void CAEN_V1742::configure(caen::Digitizer&) {}; // stub

bool CAEN_V1742::Initialise(std::string configfile, DataModel& data) {
  try {
    if (configfile != "") m_variables.Initialise(configfile);
    //m_variables.Print();

    m_data = &data;
    m_log  = m_data->Log;

    if (!m_variables.Get("verbose", m_verbose)) m_verbose = 1;

    std::string string;
    if (!m_variables.Get("link_type", string)) {
      error("link_type not found: don't know how to connect to the digitizer");
      return false;
    };

    for (std::string::iterator c = string.begin(); c != string.end(); ++c)
      *c = tolower(*c);

    CAEN_DGTZ_ConnectionType link;
    if (string == "usb")
      link = CAEN_DGTZ_USB;
    else if (string == "optical")
      link = CAEN_DGTZ_OpticalLink;
    else if (string == "usb_a4818_v2718")
      link = CAEN_DGTZ_USB_A4818_V2718;
    else if (string == "usb_a4818_v3718")
      link = CAEN_DGTZ_USB_A4818_V3718;
    else if (string == "usb_a4818_v4718")
      link = CAEN_DGTZ_USB_A4818_V4718;
    else if (string == "usb_a4818")
      link = CAEN_DGTZ_USB_A4818;
    else if (string == "usb_a4718")
      link = CAEN_DGTZ_USB_V4718;
    else if (string == "eth_a4718")
      link = CAEN_DGTZ_ETH_V4718;
    else {
      error("unknown link_type: %s", string.c_str());
      return false;
    };

    if (link == CAEN_DGTZ_ETH_V4718) {
      error("ethernet connection is not supported yet");
      return false;
    };

    std::string vme_str;

    int num      = 0;
    int conet    = 0;
    m_variables.Get("link_num",   num);
    m_variables.Get("link_conet", conet);

    unsigned vme = 0;
    if (m_variables.Get("link_vme", vme_str) && !parse_hex(vme_str, vme)) {
      error("invalid VME card number: `%s'", vme_str.c_str());
      return false;
    };

    std::string output;
    m_variables.Get("output", output);
    if (output.empty()) {
      error("The name of output ROOT file is not provided");
      return false;
    };

    args = new Args();
    args->tool = this;

    info(
        "connecting via %s link with parameters: num = %d, conet = %d, vme = %X",
        string.c_str(), num, conet, vme
    );
    args->digitizer = new caen::Digitizer(link, num, conet, vme << 16);
    if (m_verbose > 1) {
      auto& i = args->digitizer->info();
      info("model name: %s", i.ModelName);
      info("model: %u", i.Model);
      info("number of channels: %u", i.Channels);
      info("ROC firmware: %s", i.ROC_FirmwareRel);
      info("AMC firmware: %s", i.AMC_FirmwareRel);
      info("serial number: %u", i.SerialNumber);
      info("license: %s", i.License);
    };

    if (args->digitizer->DPPFirmwareCode() != STANDARD_FW_CODE) {
      error(
          "unexpected firmware code: %u, expected less than %u (D-WAVE)",
          args->digitizer->DPPFirmwareCode(),
          0x80
      );
      return false;
    };

    if (args->digitizer->info().FamilyCode != CAEN_DGTZ_XX742_FAMILY_CODE) {
      error(
          "unexpected board family code: %u, expected %u (XX742)",
          args->digitizer->info().FamilyCode,
          CAEN_DGTZ_XX742_FAMILY_CODE
      );
      return false;
    };

    configure(*args->digitizer);

    args->buffer = new caen::Digitizer::ReadoutBuffer(*args->digitizer);
    args->event  = new caen::Digitizer::WaveEvent(*args->digitizer);

    args->waveforms.resize(32);

    args->output = new TFile(output.c_str(), "RECREATE");
    args->tree   = new TTree("CAEN-V1742", "CAEN-V1742");

    args->timestamp = args->tree->Branch(
        "timestamp", static_cast<uint32_t*>(nullptr)
    );
    args->tree->Branch("waveforms", &args->waveforms);

    info("starting aquisition");
    args->digitizer->SWStartAcquisition();

    util = new Utilities();
    util->CreateThread("CAEN-V1742", &thread, args);

    return true;
  } catch (std::exception& e) {
    error("%s", e.what());
    return false;
  };
};

bool CAEN_V1742::Execute() {
  sleep(1); // no delay in tool execution?
  return true;
};

bool CAEN_V1742::Finalise() {
  if (args) {
    if (util) {
      util->KillThread(args);
      delete util;
      util = nullptr;
    };

    if (args->event)  delete args->event;
    if (args->buffer) delete args->buffer;

    if (args->digitizer) {
      info("stopping acquisition");
      args->digitizer->SWStopAcquisition();
      delete args->digitizer;
    };

    if (args->tree)   delete args->tree;
    if (args->output) delete args->output;

    delete args;
    args = nullptr;
  };

  return true;
};

void CAEN_V1742::thread(Thread_args* arg) {
  sleep(1); // no delay in thread execution?
  Args* args = static_cast<Args*>(arg);
  try {
    args->digitizer->readData(
        CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, *args->buffer
    );

    uint32_t nevents = args->digitizer->getNumEvents(*args->buffer);
    if (nevents == 0) return;

    args->tool->info("acquiring event");

    for (uint32_t e = 0; e < nevents; ++e) {
      args->digitizer->getEvent(*args->buffer, e, args->event);
      auto event = static_cast<CAEN_DGTZ_X742_EVENT_t*>(args->event->data);
      for (int g = 0; g < 4; ++g)
        if (event->GrPresent[g]) {
          CAEN_DGTZ_X742_GROUP_t* group = event->DataGroup + g;
          args->timestamp->SetAddress(&group->TriggerTimeTag);
          for (int c = 0; c < 8; ++c) {
            int channel = g * 8 + c;
            args->waveforms[channel].resize(group->ChSize[c]);
            for (uint32_t i = 0; i < group->ChSize[c]; ++i)
              args->waveforms[channel][i] = group->DataChannel[c][i];
          };
        } else
          for (int c = 0; c < 8; ++c) args->waveforms[g * 8 + c].clear();
      args->tree->Fill();
      args->tree->Write();
    };

    args->tool->stop(); // stop after acquiring one event
  } catch (std::exception& e) {
    args->tool->error("%s", e.what());
  };
};

void CAEN_V1742::stop() {
  m_data->vars.Set("StopLoop", true);
};
