This tool polls data from `DataModel::v792_readout` and
`DataModel::v1290_readout` and dumps it to two files.

* `Dumper::Initialise`: opens the files.
* `Dumper::Execute`: starts the polling thread.
* `Dumper::Finalise`: stops the polling thread and closes the files.

Configuration options:

* `qdc`: name of the file for the QDC (V792) data.
* `tdc`: name of the file for the TDC (V1290) data.
