This tool operates with CAEN V1290 time to digital converters. The tool connects
to a number of V1290s, reads them out sequentially and stores the data in
`DataModel::v1290_readout` for further processing. Readout is performed by
continuous polling of the cards.

* `V1290::Initialise` connects to and configures the cards.
* `V1290::Execute` starts the readout thread.
* `V1290::Finalise` stops the readout thread and closes the connections.

Each setting in the configuration file can be specified either as
```
setting_name value
```
of as
```
setting_name_index value
```
for example,
```
enable_channels FFFF
enable_channels_1 0FFF
```
The first form, `setting_name` sets the default value for all of the cards. The
second form overrides the default value for card index (card 1 in the example).
Enumeration of the cards begins with 0.

The following settings describe how to connect to a V1290:

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
  - hexadecimal number representing 16 most significant bits of the VME base
    address of the device, e.g., `90AB`. This is the number set by the rotary
    switches on the board.

So, to connect to two V1290 cards with rotary switches set to 00AA and 10AA via
a V3718 VME bridge, you can write
```
link usb
vme_0 00AA
vme_1 10AA
```

The following settings describe V1290 configuration. Most of them result in a
straghtforward write of the provided value to a register of the V1290 during the
tool initialisation.

* `adjust_channel_i` for i from 0 to 32, integer: adds a positive offset to the
  channel i. Allowed values are 1 to 65535. See opcode 52nn in the
  documentation.

* `adjust_rc_i` for i from 0 to 3, integer: adjusts the RC delay line of the TDC
  c. Allowed values are 0 to 65535. See opcode 540n in the documentation.

* `align_64`, boolean: when set, and during a block transfer an odd number of
  packets is transferred, a packet marked as not valid datum is appended to the
  transfer to make the size of the transfer aligned to 64 bits. See bit 4 of the
  Control register (0x1000) in the documentation.

* `dead_time`, floating point: channel dead time between hits in seconds.
  Allowed values are 5e-9 s, 10e-9 s, 30e-9 s, 100e-9 s. Intermedate values will
  be rounded to the nearest allowed. Default is 5e-9 s. See opcode 28xx in the
  documentation.

* `dll_clock`, string: set the DLL clock source. Allowed values:
  - direct_40: direct 40 MHz clock (low resolution)
  - PLL_40: PLL 40 MHz clock (low resolution)
  - PLL_160: PLL 160 MHz clock (medium resolution)
  - PLL_320: PLL 320 MHz clock (high resolution)
  See opcode C8xx in the documentation.

* `edge_detection`, string, allowed values: "leading", "trailing", "both". Sets
  whether the recorded time is of a leading, trailing or both edges of the
  signal. When both edges are recorded, pulse width is measured; this is also
  referred to as the pair mode in the documentation. See opcode 22xx.

* `emit_empty_events`, boolean: when set, and a trigger happens and there were
  no hits registered by the TDC, a global header packet followed by a global
  trailer packet is written to the output buffer. When unset, no packets are
  written to the output buffer in this case. See bit 3 of the Control register
  (0x1000) in the documentation.

* `enable_bus_error`, boolean: when set, the board will use bus error to
  designate the end of a block transfer. See bit 0 of the Control register
  (0x1000) in the documentation.

* `enable_channel_error`, boolean: enable signalling of channel select error
  (synchronization error). See opcode 39xx in the documentation.

* `enable_coarse_error`, boolean: enable signalling of coarse error (parity
  error on coarse count). See opcode 39xx in the documentation.

* `enable_compensation`, boolean: see bit 5 of the Control register (0x1000) in
  the documentation.

* `enable_control_error`, boolean: enable signalling of control parity error.
  See opcode 39xx in the documentation.

* `enabled_channels`, hexadecimal: a 32-bit mask of channels to enable. See
  opcodes 40nn, 41nn in the documentation.

* `enable_channel_i`, boolean: enable channel i. To enable channel i on board j,
  use two subscripts: `enable_channel_i_j`.

* `enable_tdc_channels`, array of hexadecimal: 4 32-bit masks of channels to
  enable for each of the TDC chips of the board. See opcode 44xx in the
  documentation. Note that TDC chip channels are not the channels of the board
  --- see `enable_channels` to enable/disable the board channels. Use this
  option only if you know what you are doing.

* `enable_tdc_i_channels`, hexadecimal: a 32-bit mask of channels to enable for
  TDC chip i of the board. See notes for `enable_tdc_channels` above.

* `enable_tdc_i_channel_j`, boolean: enable channel j for TDC chip i for all
  boards. To enable channel j for TDC chip i on board k, use two subscripts:
  `enable_tdc_i_channel_j_k`. See notes for `enable_tdc_channels` above.

* `enable_error_bypass`, boolean: see opcode 37xx, 38xx in the documentation.

* `enable_error_mark`, boolean: when set, an error mark will be written to the
  output buffer when a global error occurs. Default: 1. See opcodes 35xx, 36xx
  in the documentation.

* `enable_ettt`, boolean: when set, an extra packet with additional significant
  bits for the trigger time (extended trigger time tag) is emitted for each
  event. See bit 6 of the Control register (0x1000) in the documentation.

* `enable_header_and_trailer`, boolean: when set, header and trailer for each of
  the TDC chips will be written into the output. See opcodes 30xx and 31xx in
  the documentation.

* `enable_jtag_error`, boolean: enable signalling of Jtag instruction parity
  error. See opcode 39xx in the documentation.

* `enable_l1_parity_error`, boolean: enable signalling of L1 buffer parity
  error. See opcode 39xx in the documentation.

* `enable_readout_fifo_error`, boolean: enable signalling of readout FIFO parity
  error. See opcode 39xx in the documentation.

* `enable_setup_error`, boolean: enable signalling of setup parity error. See
  opcode 39xx in the documentation.

* `enable_sw_termination`, boolean: enables setting of the termination status by
  the software. See bit 2 of the Control register (0x1000) in the documentation.

* `enable_trigger_error`, boolean: enable signalling of trigger matching error
  (state error). See opcode 39xx in the documentation.

* `enable_trigger_fifo_error`, boolean: enable signalling of trigger FIFO parity
  error. See opcode 39xx in the documentation.

* `enable_vernier_error`, boolean: enable signalling of vernier errors (DLL
  unlocked or excessive jitter). See opcode 39xx in the documentation.

* `event_size`, integer: maximum number of hits per event. Allowed values are 0,
  1, 2, 4, 8, 16, 32, 64, 128, unlimited (greater than 128, default).
  Intermediate values will be rounded up. See opcode 33xx in the documentation.

* `fifo_size`, integer: sets the actual size of the L1 buffer. Allowed values
  (in words): 2, 4, 8, 16, 32, 64, 128, 256. Intermediate values will be rounded
  up. Default is 256 words. See opcode 3Bxx in the documentation.

* `geo_address`, integer from 0 to 31: the GEO address of the board. It is
  available in the readout data and allows for determination of which board the
  readout came from. See GEO Address register (0x100E) in the documentation.

* `global_offset_coarse`, integer: adds a global offset to the coarse counter.
  Allowed values are 0 to 2047. See opcode 50xx in the documentation.

* `global_offset_fine`, integer: adds a global offset to the fine (vernier)
  counter. Allowed values are 0 to 31. See opcode 50xx in the documentation.

* `interrupt_level`, integer from 0 to 7: see Interrupt Level register (0x100A)
  in the documentation.

* `interrupt_vector`, integer from 0 to 255: see Interrupt Vector register
  (0x100C) in the documentation.

* `load_scan_path`, boolean: when set, load the current scan path setup from
  internal memory. See opcode 72xx in the documentation.

* `load_scan_path_tdc_i` for i from 0 to 3, boolean: when set, load the current
  scan path setup to TDC i only. See opcode 770n in the documentation. To load
  scan path for TDC i of board j, use two subscripts: `load_scan_path_tdc_i_j`.

* `pulse_resolution`, floating point: when operating in the pair mode, intended
  resolution for the signal width in seconds. Allowed and default values are the
  same as for `edge_resolution` in the pair mode above. See opcode 25xx in the
  documentation.

* `reject_margin`, floating point: width of the reject margin of the match
  window. Allowed values are 0 to 102.4e-6 s in steps of 25e-9 s; it is
  recommended to set the margin greater or equal to 25e-9 s. Intermediate values
  will be rounded to the nearest multiple of 25e-9 s. Default is 100e-9 s. See
  "Set reject margin" (13xx) opcode in the documentation.

* `resolution` or `edge_resolution`, float: intended resolution for the edge of
  the signal in seconds. When operated in the single mode (the time of only one
  of the signal edges is recorded), allowed values are 25e-12 s, 100e-12 s,
  200e-12 s, 800e-12 s, when operated in the pair mode (times of both edges are
  recorded), allowed values are 100e-12 s, 200e-12 s, 400e-12 s, 800e-12 s,
  1.6e-9 s, 3.12e-9 s, 6.25e-9 s, 12.5e-9 s, 25e-9 s, 50e-9 s, 100e-9 s, 400e-9
  s, 800e-9 s. Intermediate values will be rounded to the nearest allowed.
  Default is the lowest resolution available: 25e-12 s in the single mode and
  100e-12 s in the pair mode. See opcodes 24xx and 25xx in the documentation.

* `search_margin`, floating point: width of the extra search field of the match
  window. Allowed values are 0 to 102.4e-6 s in steps of 25e-9 s, though
  reasonable values are no greater than 1.25e-6 s. Intermediate values will be
  rounded to the nearest multiple of 25e-9 s. Default is 200e-9 s. See "Set
  extra search margin" (12xx) opcode in the documentation.

* `sw_termination`, boolean: sets the termination status of the board. See bit 1
  of the Control register (0x1000) in the documentation.

* `triggered_mode`, boolean: when set, the board works in triggered mode and
  outputs only the hits that arrived during the trigger window. When unset, the
  board outputs every hit. See "Set Trigger Matching mode" (00xx) and "Set
  Continous Matching mode" (01xx) opcodes in the documentation.

* `trigger_time_subtraction`, boolean: enables the trigger time tag subtraction;
  in this operating mode the time measurements are referred to the beginning of
  the match window. See opcodes 14xx and 15xx in the documentation.

* `window_offset`, floating point: offset of the match window with respect to
  the trigger, in seconds. Allowed values are -51.2e-6 s to 1e-6 s, in steps of
  25e-9 s. Intermedate values will be rounded to the nearest multiple of 25e-9
  s. Default is -1e-6 s. See "Set window offset" (11xx) opcode in the
  documentation.

* `window_width`, floating point: trigger window width in seconds. Allowed
  values are 25e-9 s to 52.2e-6 s, in steps of 25e-9 s. Intermediate values will
  be rounded to the nearest multiple of 25e-9 s. Default is 500e-9 s. See "Set
  window width" (10xx) opcode in the documentation.
