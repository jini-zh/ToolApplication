This tool talks to CAEN V1495 FPGA board. The tool connects to a single V1495
board and writes values to its registers. The values are read from a JSON file
supplied by the user via tool's configuration file.

The following settings describe how to connect to the V1495:

* `link`: connection link type, corresponds to the `LinkType` argument of
  `CAENComm_OpenDevice2` function of the
  [`CAENComm` library](https://www.caen.it/products/caencomm-library/).
  Allowed values:
  - `usb`: connection is via a USB link to a VME bridge (default).
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
  - a hexadecimal number representing 16 most significant bits of the VME base
    address of the device, e.g., `90AB`. This is the number set by the rotary
    switches on the board.

The only setting specific to V1495 is `config` which should provide the path to
a JSON file containing a single object with keys being register addresses and
values being the what is needed to be written to these address. Both keys and
values should be string containing hexadecimal numbers (unfortunately, JSON
format doesn't support native hexadecimal numbers). For example,

```json
{
  "0x300c": "0x01234567",
  "0x3010": "0x89abcdef"
}
```
