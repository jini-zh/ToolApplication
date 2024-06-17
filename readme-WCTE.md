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
