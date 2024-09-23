The tool tests the VME toolchain.

Usage:
```bash
    git pull https://github.com/WCTEDAQ/WebServer /tmp/WebServer
    make -C /tmp/WebServer
    docker network create tooldaq
    bridge=`ifconfig | grep -m 1 -o '^br-[0-9a-f]+'`
    ip=`ifconfig $bridge | grep -Po 'inet \K[\d.]+'`
    docker run --rm -v /tmp/WebServer:/web -v "$PWD:$PWD" --network=tooldaq --mount=type=tmpfs,dst=/tmp,size=500M -dt tooldaq/server
    route add -net 239.0.0.0/8 dev $bridge
    psql -h $ip -d daq -c '
      insert into device_config (time, device, version, author, description, data) values (now(), 'VME', 0, '', '', '{}');
      insert into configurations (config_id, time, name, version, description, author, data) values (0, now(), 'test', 0, '', '', '{"VME":0}');
    '
    ./main configfiles/vme-receive/main.cfg
    Initialise
    Execute

    # In another terminal
    docker ps # note the container name
    docker exec -it '<container name>' /bin/bash
    # The container glibc is not compatible with the glibc on my host system, so
    cd
    git clone ~/.../WCTEDAQ
    cd WCTEDAQ
    mkdir Dependencies
    cd Dependencies
    ln -s /opt/boost_1_66_0 .
    ln -s /opt/zeromq-4.0.7 .
    cd ..
    ./GetToolDAQ --no_boost --no_init --no_zmq
    . Setup.sh
    ./main configfiles/VME/main.cfg
    Initialise

    # go to https://172.18.0.2
    # go to services control
    # find the control buttons `reconfigure`, `start`, `stop`
    # `start` starts the run
    # `stop` stops the run
    # `reconfigure` signals to change the config
    # Each action is performed on Execute of the VME toolchain
```
