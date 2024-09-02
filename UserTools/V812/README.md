This tools operates with CAEN V812 constant fraction discriminator. The tool
connects to a number of V812s and configures them.

* `V812::Initialise` connects to and configures the cards.
* `V812::Execute` does nothing.
* `V812::Finalise` closes the connections.

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
enable_channels FF
enable_channels_1 0F
```
The first form, `setting_name` sets the default value for all of the cards. The
second form overrides the default value for card index (card 1 in the example).
Enumeration of the cards begins with 0.

The following settings describe how to connect to a V812:

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

So, to connect to three V812 cards with rotary switches set to 8800, 8900, and
8A00 via a V3718 VME bridge, you can write
```
link usb
vme_0 8800
vme_1 8900
vme_2 8A00
```

The following settings describe V812 configuration. Most of them result in a
straghtforward write of the provided value to a register of the V812 during the
tool initialisation.

* `channel_i_threshold`, float: set threshold for channel i in volts. Allowed values are
  from -1e-3 V to -255e-3 V with 1e-3 V steps, although documentation says that
  a minimum threshold of -5e-3 V is required. See registers %00 -- %1E in the
  documentation.

* `dead_time`, integer: dead time value. Allowed values are from 0 to 255 with 0
  meaning 118 ns, 255 --- 1625 ns. See registers %44, %46 in the documentation.

* `dead_time_0-7`, integer: dead time value for channels 0-7. See `dead_time`
  above and register %44 in the documentation.

* `dead_time_8-15`, integer: dead time value for channels 8-15. See `dead_time`
  above and register %46 in the documentation.

* `enable_channels`, hexadecimal: a 16 bit mask designating the enabled
  channels. See register %4A in the documentation.

* `enable_channel_i`, boolean: enable channel i for all boards. To enable
  channel i for board j, use two subscripts: `enable_channel_i_j`.

* `majority_threshold`, integer: threshold for the majority output connector of
  the board. Allowed values are 1 to 244. See register %48 in the documentation.

* `output_width`, integer: output pulse width. Allowed values are from 0 to 255
  with 0 meaning 12 ns, 255 --- 206 ns, and non-linear relation for
  intermediate values. See registers %40, %42 in the documentation.

* `output_width_0-7`, integer: output pulse width for channels 0-7. See
  `output_width` above and register %40 in the documentation.

* `output_width_8-15`, integer: output pulse width for channels 8-15. See
  `output_width` above and register %42 in the documentation.

Please note that V812 configuration on power up is not specified and it has no
reset function, so enabling and setting thresholds of the channels is required
for successful operation.
