This tool listens to alerts distributed by the RunControl tool and sets the
corresponding flags and values in the DataModel. Its purpose is to provide run
control for a toolchain running separately from the main toolchain governed by
RunControl.

WARNING: Configuration tool resets the change\_config flag, so if
both SlaveRunControl and Configuration are used in a toolchain, Configuration
should be executed immediately before.
