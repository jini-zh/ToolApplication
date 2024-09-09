This tool operates with CAEN V792 charge to digital converters. The tool
connects to a number of V792s, reads them out sequentially and stores the data
in `DataModel::v792_readout` for further processing. Readout is performed by
continuous polling of the cards.

* `V792::Initialise` connects to and configures the cards.
* `V792::Execute` starts the readout thread.
* `V792::Finalise` stops the readout thread and closes the connections.

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

The following settings describe how to connect to a V792:

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

So, to connect to two V792 cards with rotary switches set to 00AA and 10AA via a
V3718 VME bridge, you can write
```
link usb
vme_0 00AA
vme_1 10AA
```

The following settings describe V792 configuration. Most of them result in a
straghtforward write of the provided value to a register of the V792 during the
tool initialisation.

* `align_64`, boolean: when set, and during a block transfer an odd number of
  packets is transferred, a filler packet is appended to the transfer to make
  the size of the transfer aligned to 64 bits. See bit 6 of the Control 1
  register (0x1010) in the documentation.

* `all_triggers`, boolean: when set, the event counter is incremented on all
  triggers, otherwise it is incremented only on accepted triggers. See bit 14 of
  Bit Set 2 register (0x1032) in the documentation.

* `block_readout`, boolean: when set, the module sends the data until the end of
  the first event, otherwise it sends all available data. See bit 2 of Control 1
  register (0x1010) in the documentation.

* `channel_i_threshold` for i from 0 to 31, integer: threshold of channel i.
  Allowed values are 0 to 255. See Thresholds Memory register (0x1080) in the
  documentation and also `enable_shift`.

* `crate_number`, integer: the number of the crate which the board is plugged
  into. Available in the readout data and allows for determination of which
  board the readout came from, along with `geo_address`. Allowed values are 0
  to 255. See Crate Select register (0x103C) in the documentation.

* `current_pedestal`, integer: controls the board sensitivity. Allowed values
  are 0 to 255. See Iped register (0x1060) in the documentation.

* `enable_bus_error`: when set, the module generates a bus error at the end of
  the transfer, when unset the module fills the rest of the transfer with filler
  packets. See bit 5 of Control 1 register (0x1010) in the documentation.

* `enable_channels`, hexadecimal: a 32 bit mask designating enabled channels.

* `enable_empty`, boolean: when set and there is no data over threshold at the
  time of the trigger, a header followed by a trailer packets are written to the
  outbut buffer. When unset, nothing is written to the buffer. See bit 12 of Bit
  Set 2 register (0x1032) in the documentation.

* `enable_overflow`, boolean: when set, the data causing the ADC overflow will
  be written into the output buffer, otherwise it will be discarded. Default: 0.
  See bit 3 of Bit Set 2 register (0x1032) in the documentation.

* `enable_slide`, boolean: enable the sliding scale technique improving on
  conversion linearity but reducing the dynamic range. Default is 1 (enabled).
  See bit 7 of Bit Set 2 register (0x1032) and section 2.2 in the documentation.

* `enable_slide_subtraction`, boolean: when unset, the subtraction section of
  the sliding scale is disabled --- useful for test purposes only. See bit 13 of
  Bit Set 2 register (0x1032) in the documentation (the bit value is inverted).

* `enable_threshold`, boolean: when set, only the data above the threshold are
  written into the output buffer. Default is 1 (enabled). See bit 4 of Bit Set 2
  register (0x1032) in the documentation (the bit value is inverted).

* `event_trigger`, integer: when the number of events stored in the memory
  equals this value an interrupt request is generated. Allowed values are 0
  to 31. Default is 0 (disabled). See Event Trigger register (0x1020) in the
  documentation.

* `geo_address`, integer from 0 to 31: the GEO address of the board. It is
  available in the readout data and allows for determination of which board the
  readout came from. See GEO Address register (0x1002) in the documentation.

* `interrupt_level`, integer from 0 to 7: see Interrupt Level register (0x100A)
  in the documentation.

* `interrupt_vector`, integer from 0 to 255: see Interrupt Vector register
  (0x100C) in the documentation.

* `panel_resets_software`, boolean: when set, the RESET button on the fron panel
  of the board resets the module software, when unset it resets the data
  (default). See bit 4 of Control 1 register (0x1010) in the documentation.
* `shift_threshold`, boolean: in order to determine whether the digitized value
  is over the threshold, it is compared with the value set by
  `channel_i_threshold` multiplied by 16 if `enable_shift` is set and by 2 if it
  isn't. See bit 8 of Bit Set 2 register (0x1032) in the documentation.

* `slide_constant`, integer: constant to which the sliding scale is set when the
  sliding scale is disabled by `slide_subtraction_enabled`. Allowed values are 0
  to 255. See Slide Constant register (0x106A) in the documentation.
