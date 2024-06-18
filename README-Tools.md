# CAEN card connection parameters

The following settings are supported in the configuration files for V1290 and
V792 tools:

* `link`: connection link type, corresponds to the `LinkType` argument of
  `CAENComm_OpenDevice2` function of the
  [`CAENComm` library](https://www.caen.it/products/caencomm-library/).
  Allowed values:
  - `usb`: connection is via a USB link to a VME bridge.
  - `optical`: connection is via an optical link to a VME bridge.
  - `A4818-V2718`: connection is via a USB link to a A4818 Conet module which is
    connected by an optical link to a V2718 VME bridge.
  - `A4818-V3718`: connection is via a USB link to a A4818 Conet module which is
    connected by an optical link to a V3718 VME bridge.
  - `A4818-V4718`: connection is via a USB link to a A4818 Conet module which is
    connected by an optical link to a V4718 VME bridge.
  - `eth-V4718`: connection is via an Ethernet link to a V4718 VME bridge.
  - `usb-V4718`: connection is via a USB link to a V4718 VME bridge.
* `ip`: only used if `link` is `eth-V4718` --- IP address of the V4718.
* `arg` (0 by default):
  - in the case of a USB connection, the USB link number of the host. Unless
    there are multiple USB connections to CAEN devices, can be left as 0.
    Otherwise would require trial and error to determine.
  - in the case of an optical connection via V2718, V3718 or V4718, the optical
    link number.
  - in the case of a connection through A4818 or to the V4718, PID of the
    device, where PID is the product identification number and should be written
    on the device.
  - not used in the case of an Ethernet connection.
* `conet` (0 by default):
  - in the case of an optical link connection, it is the number of the device in
    the daisy chain of the optical link.
  - should be left as 0 for other kinds of connections.
* `vme`:
  - the VME base address of the device. Should be in the form 0x12340000, where
    `1234` is the number set by the rotary switches on the board.

# V792 configuration parameters

* `geo_address`, integer from 0 to 31: the GEO address of the board. It is
  available in the readout data and allows for determination of which board the
  readout came from. See GEO Address register (0x1002) in the documentation.
* `interrupt_level`, integer from 0 to 7: see Interrupt Level register (0x100A)
  in the documentation.
* `interrupt_vector`, integer from 0 to 255: see Interrupt Vector register
  (0x100C) in the documentation.
* `block_readout`, boolean: when set, the module sends the data until the end
  of the first event, otherwise it sends all available data. See bit 2 of
  Control 1 register (0x1010) in the documentation.
* `panel_resets_software`, boolean: when set, the RESET button on the fron
  panel of the board resets the module software, when unset it resets the data
  (default). See bit 4 of Control 1 register (0x1010) in the documentation.
* `bus_error_enabled`: when set, the module generates a bus error at the end of
  the transfer, when unset the module fills the rest of the transfer with
  filler packets. See bit 5 of Control 1 register (0x1010) in the
  documentation.
* `align_64`, boolean: when set, and during a block transfer an odd number of
  packets is transferred, a filler packet is appended to the transfer to make
  the size of the transfer aligned to 64 bits. See bit 6 of the Control 1
  register (0x1010) in the documentation.
* `event_trigger`, integer: when the number of events stored in the memory
  equals this value an interrupt request is generated. Allowed values are 0
  to 31. Default is 0 (disabled). See Event Trigger register (0x1020) in the
  documentation.
* `overflow_enabled`, boolean: when set, the data causing the ADC overflow will
  be written into the output buffer, otherwise it will be discarded. Default:
  0. See bit 3 of Bit Set 2 register (0x1032) in the documentation.
* `threshold_enabled`, boolean: when set, only the data above the threshold are
  written into the output buffer. Default is 1 (enabled). See bit 4 of Bit Set
  2 register (0x1032) in the documentation (the bit value is inverted).
* `slide_enabled`, boolean: enable the sliding scale technique improving on
  conversion linearity but reducing the dynamic range. Default is 1 (enabled).
  See bit 7 of Bit Set 2 register (0x1032) and section 2.2 in the
  documentation.
* `shift_enabled`, boolean: in order to determine whether the digitized value
  is over the threshold, it is compared with the value set by
  `channel_i_threshold` multiplied by 16 if `shift_enabled` is set and by 2 if
  it isn't. See bit 8 of Bit Set 2 register (0x1032) in the documentation.
* `empty_enabled`, boolean: when set and there is no data over threshold at the
  time of the trigger, a header followed by a trailer packets are written to
  the outbut buffer. When unset, nothing is written to the buffer. See bit 12
  of Bit Set 2 register (0x1032) in the documentation.
* `slide_subtraction_enabled`, boolean: when unset, the subtraction section of
  the sliding scale is disabled --- useful for test purposes only. See bit 13
  of Bit Set 2 register (0x1032) in the documentation (the bit value is
  inverted).
* `all_triggers`, boolean: when set, the event counter is incremented on all
  triggers, otherwise it is incremented only on accepted triggers. See bit 14
  of Bit Set 2 register (0x1032) in the documentation.
* `crate_number`, integer: the number of the crate which the board is plugged
  into. Available in the readout data and allows for determination of which
  board the readout came from, along with `geo_address`. Allowed values are 0
  to 255. See Crate Select register (0x103C) in the documentation.
* `current_pedestal`, integer: controls the board sensitivity. Allowed values
  are 0 to 255. See Iped register (0x1060) in the documentation.
* `slide_constant`, integer: constant to which the sliding scale is set when
  the sliding scale is disabled by `slide_subtraction_enabled`. Allowed values
  are 0 to 255. See Slide Constant register (0x106A) in the documentation.
* `enable_channels`, hexadecimal: a 32 bit mask designating enabled channels.
* `channel_i_threshold` for i from 0 to 31, integer: threshold of channel i.
  Allowed values are 0 to 255. See Thresholds Memory register (0x1080) in the
  documentation and also `shift_enabled`.

# V1290 configuration parameters

* `bus_error_enabled`, boolean: when set, the board will use bus error to
  designate the end of a block transfer. See bit 0 of the Control register
  (0x1000) in the documentation.
* `sw_termination`, boolean: sets the termination status of the board. See bit
  1 of the Control register (0x1000) in the documentation.
* `sw_termination_enabled`, boolean: enables setting of the termination status
  by the software. See bit 2 of the Control register (0x1000) in the
  documentation.
* `emit_empty_events`, boolean: when set, and a trigger happens and there were
  no hits registered by the TDC, a global header packet followed by a global
  trailer packet is written to the output buffer. When unset, no packets are
  written to the output buffer in this case. See bit 3 of the Control register
  (0x1000) in the documentation.
* `align_64`, boolean: when set, and during a block transfer an odd number of
  packets is transferred, a packet marked as not valid datum is appended to the
  transfer to make the size of the transfer aligned to 64 bits. See bit 4 of
  the Control register (0x1000) in the documentation.
* `compensation_enabled`, boolean: see bit 5 of the Control register (0x1000)
  in the documentation.
* `ettt_enabled`, boolean: when set, an extra packet with additional
  significant bits for the trigger time (extended trigger time tag) is emitted
  for each event. See bit 6 of the Control register (0x1000) in the
  documentation.
* `interrupt_level`, integer from 0 to 7: see Interrupt Level register (0x100A)
  in the documentation.
* `interrupt_vector`, integer from 0 to 255: see Interrupt Vector register
  (0x100C) in the documentation.
* `geo_address`, integer from 0 to 31: the GEO address of the board. It is
  available in the readout data and allows for determination of which board the
  readout came from. See GEO Address register (0x100E) in the documentation.
* `triggered_mode`, boolean: when set, the board works in triggered mode and
  outputs only the hits that arrived during the trigger window. When unset, the
  board outputs every hit. See "Set Trigger Matching mode" (00xx) and "Set
  Continous Matching mode" (01xx) opcodes in the documentation.
* `window_width`, floating point: trigger window width in seconds. Allowed
  values are 25e-9 s to 52.2e-6 s, in steps of 25e-9 s. Intermediate values
  will be rounded to the nearest multiple of 25e-9 s. Default is 500e-9 s. See
  "Set window width" (10xx) opcode in the documentation.
* `window_offset`, floating point: offset of the match window with respect to
  the trigger, in seconds. Allowed values are -51.2e-6 s to 1e-6 s, in steps
  of 25e-9 s. Intermedate values will be rounded to the nearest multiple of
  25e-9 s. Default is -1e-6 s. See "Set window offset" (11xx) opcode in the
  documentation.
* `search_margin`, floating point: width of the extra search field of the match
  window. Allowed values are 0 to 102.4e-6 s in steps of 25e-9 s, though
  reasonable values are no greater than 1.25e-6 s. Intermediate values will be
  rounded to the nearest multiple of 25e-9 s. Default is 200e-9 s. See "Set
  extra search margin" (12xx) opcode in the documentation.
* `reject_margin`, floating point: width of the reject margin of the match
  window. Allowed values are 0 to 102.4e-6 s in steps of 25e-9 s; it is
  recommended to set the margin greater or equal to 25e-9 s. Intermediate
  values will be rounded to the nearest multiple of 25e-9 s. Default is 100e-9
  s. See "Set reject margin" (13xx) opcode in the documentation.
* `trigger_time_subtraction`, boolean: enables the trigger time tag
  subtraction; in this operating mode the time measurements are referred to the
  beginning of the match window. See opcodes 14xx and 15xx in the
  documentation.
* `edge_detection`, string, allowed values: "leading", "trailing", "both". Sets
  whether the recorded time is of a leading, trailing or both edges of the
  signal. When both edges are recorded, pulse width is measured; this is also
  referred to as the pair mode in the documentation. See opcode 22xx.
* `resolution` or `edge_resolution`, float: intended resolution for the edge of
  the signal in seconds. When operated in the single mode (the time of only one
  of the signal edges is recorded), allowed values are 25e-12 s, 100e-12 s,
  200e-12 s, 800e-12 s, when operated in the pair mode (times of both edges are
  recorded), allowed values are 100e-12 s, 200e-12 s, 400e-12 s, 800e-12 s,
  1.6e-9 s, 3.12e-9 s, 6.25e-9 s, 12.5e-9 s, 25e-9 s, 50e-9 s, 100e-9 s, 400e-9
  s, 800e-9 s. Intermediate values will be rounded to the nearest allowed.
  Default is the lowest resolution available: 25e-12 s in the single mode and
  100e-12 s in the pair mode. See opcodes 24xx and 25xx in the
  documentation.
* `pulse_resolution`, floating point: when operating in the pair mode, intended
  resolution for the signal width in seconds. Allowed and default values are
  the same as for `edge_resolution` in the pair mode above. See opcode 25xx in
  the documentation.
* `dead_time`, floating point: channel dead time between hits in seconds.
  Allowed values are 5e-9 s, 10e-9 s, 30e-9 s, 100e-9 s. Intermedate values
  will be rounded to the nearest allowed. Default is 5e-9 s. See opcode 28xx in
  the documentation.
* `header_and_trailer_enabled`, boolean: when set, header and trailer for each
  of the TDC chips will be written into the output. See opcodes 30xx and 31xx
  in the documentation.
* `event_size`, integer: maximum number of hits per event. Allowed values are
  0, 1, 2, 4, 8, 16, 32, 64, 128, unlimited (greater than 128). Intermediate
  values will be rounded up. See opcode 33xx in the documentation.
* `enable_error_mark`, boolean: when set, an error mark will be written to the
  output buffer when a global error occurs. Default: 1. See opcodes 35xx, 36xx
  in the documentation.
* `enable_error_bypass`, boolean: see opcode 37xx, 38xx in the documentation.
* `enable_vernier_error`, boolean: enable signalling of vernier errors (DLL
  unlocked or excessive jitter). See opcode 39xx in the documentation.
* `enable_coarse_error`, boolean: enable signalling of coarse error (parity
  error on coarse count). See opcode 39xx in the documentation.
* `enable_channel_error`, boolean: enable signalling of channel select error
  (synchronization error). See opcode 39xx in the documentation.
* `enable_l1_parity_error`, boolean: enable signalling of L1 buffer parity
  error. See opcode 39xx in the documentation.
* `enable_trigger_fifo_error`, boolean: enable signalling of trigger FIFO
  parity error. See opcode 39xx in the documentation.
* `enable_trigger_error`, boolean: enable signalling of trigger matching error
  (state error). See opcode 39xx in the documentation.
* `enable_readout_fifo_error`, boolean: enable signalling of readout FIFO
  parity error. See opcode 39xx in the documentation.
* `enable_setup_error`, boolean: enable signalling of setup parity error. See
  opcode 39xx in the documentation.
* `enable_control_error`, boolean: enable signalling of control parity error.
  See opcode 39xx in the documentation.
* `enable_jtag_error`, boolean: enable signalling of Jtag instruction parity
  error. See opcode 39xx in the documentation.
* `fifo_size`, integer: sets the actual size of the L1 buffer. Allowed values
  (in words): 2, 4, 8, 16, 32, 64, 128, 256. Intermediate values will be
  rounded up. Default is 256 words. See opcode 3Bxx in the documentation.
* `enabled_channels`, hexadecimal: a 32-bit mask of channels to enable. See
  opcodes 40nn, 41nn in the documentation.
* `enabled_channels_i` for i from 0 to 3, hexadecimal: a 32-bit mask of
  channels to enable on each TDC chip of the board. See opcode 44xx in the
  documentation. Note that TDC chip channels are not the channels of the board
  --- use `enabled_channel` to enable/disable the board channels. Use this
  option only if you are know what you are doing.
* `global_offset_coarse`, integer: adds a global offset to the coarse counter.
  Allowed values are 0 to 2047. See opcode 50xx in the documentation.
* `global_offset_fine`, integer: adds a global offset to the fine (vernier)
  counter. Allowed values are 0 to 31. See opcode 50xx in the documentation.
* `adjust_channel_i` for i from 0 to 32, integer: adds a positive offset to the channel
  i. Allowed values are 1 to 65535. See opcode 52nn in the documentation.
* `adjust_rc_i` for i from 0 to 3, integer: adjusts the RC delay line of the
  TDC c. Allowed values are 0 to 65535. See opcode 540n in the documentation.
* `load_scan_path`, boolean: when set, load the current scan path setup from
  internal memory. See opcode 72xx in the documentation.
* `load_scan_path_tdc_i` for i from 0 to 3, boolean: when set, load the current
  scan path setup to TDC i only. See opcode 770n in the documentation.
* `dll_clock`, string: set the DLL clock source. Allowed values:
  - direct_40: direct 40 MHz clock (low resolution)
  - PLL_40: PLL 40 MHz clock (low resolution)
  - PLL_160: PLL 160 MHz clock (medium resolution)
  - PLL_320: PLL 320 MHz clock (high resolution)
  See opcode C8xx in the documentation.
