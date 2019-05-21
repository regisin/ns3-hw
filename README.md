# Introduction

This project enables the [ns-3](https://www.nsnam.org/) network simulator to have access to realtime power supply information. Researchers and developers might rely on battery/energy models while simulating various scenarios. However, to fully test an energy-aware protocol in production, hardware integration is an essential step. Thus, this projkect aims to enable a low-cost energy-aware testbed for networking protocol development.

We developed an interface between the low-level API of the [INA219](http://www.ti.com/lit/ds/symlink/ina219.pdf) current sensor that can be used by researchers in their protocols. The model allows access to raw information directly from the sensor, such as current and power readings. The model also allows the emulation of state of charge by calibrating an initial charge value (Coulombs).

# Installation and usage

This project was developed using version ns-3.29 of the simulator. This repository contains the entire simulator codebase (essentially a fork of the ns-3).

## Step 1: the hardware and operating system

We tested our project on a Raspberry Pi 3 Model B. The first step is to purchase and configure the hardware and operating system. There are plenty of tutorials of how to connect the wires and configure the OS to enable I2C communication with the device (INA219 in our case), one such tutorial can be found [HERE](https://www.youtube.com/watch?v=BgShCD7xT_A).

As for the operating system we used the official Raspbian OS with no GUI found [HERE](https://www.raspberrypi.org/downloads/raspbian/).

## Step 2: download/compile

After configuring the device, now we simply need to clone the repository:

    git clone https://github.com/regisin/ns3-hw

At this point you can compile the code.

    ./waf configure --enable-sudo
    ./waf build

## Step 3: customize each node

To run the examples, you should choose the role of each node and configure it accordingly. There are four example scripts in the `scratch` folder: `client.cc`, `relay.cc`, `server.cc`, and `test-ina.cc`. The last one (`test-ina.cc`) doesn't require any extra configuration and can be run on a single node, it simply prints out the readings from the sensor to the log function.

If you want the node to be a client in the network (i.e. generate traffic) it should execute the `client.cc` script. If you want it to be a router only, pick `relay.cc`, and if it's a server (i.e. `PacketSink`), use `server.cc`.

For each node, you should edit the desired script and change the IP and MAC addresses to a unique address. You can search for (ctrl+f) `XX`, as it is a placeholder string that must be changed. The `server.cc` doesn't require to be changed, it is set to be `XX = 01`: `10.1.1.1` and `00:00:00:00:00:01` (which should not be used by the others).

## Step 3.5: helper script

Configuring/reconfiguring each node can be annoying. To facilitate we created a helper script that does that for you. Assuming you configured each RPi with a different hostname (ex.: `pi-02`, `pi-03`, ..., `pi-99`), you can run the `reconfigure.sh` script.

This script gets the last 2 digits of the hostname, downloads this GitHub repository (in case something was modified like the type of application each node runs), replaces the placeholder address for the hostname digits, copies the command that this node should execute (more on that in a second), and compiles ns-3.

The `config.txt` is a file that contains all the commands that must be executed for each respective node. When `reconfigure.sh` parses this file, it will copy the corresponding command to the `run.sh` file (which can be executed right after boot).

For exemple, if this is your `config.txt`:

    10:./waf --run="server --time=630 --charge=100.0 --hist=10000 --routing=aodv" > server.log 2>&1
    13:./waf --run="relay  --time=600 --charge=95.0  --hist=10000 --routing=aodv" > relay13.log 2>&1
    14:./waf --run="relay  --time=600 --charge=95.0  --hist=10000 --routing=aodv" > relay14.log 2>&1
    44:./waf --run="relay  --time=600 --charge=95.0  --hist=10000 --routing=aodv" > relay44.log 2>&1
    07:./waf --run="client --time=500 --charge=80.0  --hist=10000 --routing=aodv" > client.log 2>&1

And your `etc/hostname` is `rpi-10`, hen your `run.sh` will containg a single command: `./waf --run="server --time=630 --charge=100.0 --hist=10000 --routing=aodv" > server.log 2>&1`.

## Step 4: run the experiment

Just like any other ns-3 simulation, you can run an experiment by simply calling the waf script (`./waf --run="server/relay/client --time=630 --charge=100.0 --hist=10000 --routing=aodv" > server.log 2>&1`). The challenge is that you might have to do this step at the same time in all the devices.

## Optional: execute experiment after booting the device

As mentioned before, the `reconfigure.sh` script will place the command to be executed based on the device hostname into the `run.sh` bash script. Then, you can simply execute the `run.sh` script after booting up the Raspberry Pi. This comes in handy when you don't have a keyboard and monitor to manually type the commands (i.e. outdoors experiment).

## Repeat

Nothing really interesting happens if you have a single node in your network. In orther to run a simple experiment at least 2 nodes (devices) are necessary. Repeat the same process with the second (third, fourth, etc.) device, but **don't forget to choose a different node number, rename the RPi hostname**.

# Contribute

If you would like to contribute to this project, simply send a pull request.

Currently, we use the FdNetDevice that is included with the main distributed versions of ns-3. This device is limited to a single packet I/O, which can be detrimental to the network performance. We encourage you to integrate the multiple packet I/O (burst mode), a module that is under development by the community. Which will hopefully provide more realistic performance measurements.

# Related

If you need to integrate the INA219 into your C/C++ project you can look at our independent library [HERE](https://github.com/regisin/ina219), which does not require ns-3 at all.