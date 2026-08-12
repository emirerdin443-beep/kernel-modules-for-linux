# Simple Kernel Module

Hello. As a Turkish developer, this is my first repository and my first modules. Enjoy using them. 

## About the Project

This repository contains basic examples for loading new modules into the Linux kernel. You can get an in-depth introduction to kernel programming by using different types of kernel modules.

## Modules

The following kernel modules are included in this repository:

### 1. Hello Module

- **Description:** A simple "Hello World" kernel module
- **Directory:** repository root (`./`)
- **Files:** `hello.c`, `Makefile`
- **Functionality:** Prints load and unload messages to the kernel log

### 2. Character Device Module

- **Description:** Character device driver
- **Directory:** `char_device/`
- **Files:** `char_device.c`, `Makefile`
- **Functionality:** Creates the `/dev/simple_char` device and performs read/write operations

### 3. Simple Driver Module

- **Description:** Platform device driver
- **Directory:** `simple_driver/`
- **Files:** `simple_driver.c`, `Makefile`
- **Functionality:** Performs probe and remove operations using the platform device framework

### 4. Wi-Fi Driver Module

- **Description:** Network device driver
- **Directory:** `wifi_driver/`
- **Files:** `wifi_driver.c`, `Makefile`
- **Functionality:** Creates a `wlan%d` network interface and can perform packet transmission

### 5. Hardware Manager Module

- **Description:** Hardware monitoring and management kernel module
- **Directory:** `hardware-manager/`
- **Files:** `hwmgr.c`, `Makefile`
- **Functionality:** Exposes `/proc/hwmgr` and `/sys/kernel/hwmgr` interfaces for CPU, memory, uptime, and monitoring controls

## Requirements

- Linux operating system
- Kernel headers (kernel development files)
- GCC compiler
- Make

### For Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)
```

### For RHEL/CentOS:

```bash
sudo yum groupinstall "Development Tools"
sudo yum install kernel-devel-$(uname -r)
```

## General Build Instructions

Build the Hello module from the repository root:

```bash
make
```

Build modules in subdirectories by navigating to the corresponding directory and running `make`:

```bash
cd <module_directory>
make
```

These commands create the module's `.ko` file in the current build directory.

## Module Usage

### Hello Module

```bash
# From the repository root
make

# Load the module
sudo insmod hello.ko

# Check the output
dmesg | tail -n 1

# Remove the module
sudo rmmod hello
```

**Expected Output:**

```text
[timestamp] Hello, Kernel!
[timestamp] Goodbye, Kernel!
```

### Character Device Module

```bash
cd char_device
make
sudo insmod char_device.ko

# Check the device
ls -la /dev/simple_char

# Write operation
echo "Hello Kernel" | sudo tee /dev/simple_char

# Read operation
sudo cat /dev/simple_char

# Remove the module
sudo rmmod char_device
```

### Simple Driver Module

```bash
cd simple_driver
make
sudo insmod simple_driver.ko

# Check whether the module is running
dmesg | tail -n 5

# Remove the module
sudo rmmod simple_driver
```

### Wi-Fi Driver Module

```bash
cd wifi_driver
make
sudo insmod wifi_driver.ko

# Check the network interface
ip link show

# Remove the module
sudo rmmod wifi_driver
```

### Hardware Manager Module

```bash
cd hardware-manager
make
sudo insmod hwmgr.ko

# Check the proc interface
cat /proc/hwmgr

# Check the sysfs interface
ls /sys/kernel/hwmgr

# Remove the module
sudo rmmod hwmgr
```

## Verifying the Module

To list loaded modules:

```bash
lsmod | grep <module_name>
```

Example:

```bash
lsmod | grep hello
lsmod | grep char_device
lsmod | grep simple_driver
lsmod | grep wifi_driver
lsmod | grep hwmgr
```

## Cleaning

To remove compiled files from the current module's build directory:

```bash
make clean
```

To clean all modules from the repository root:

```bash
make clean
cd char_device && make clean && cd ..
cd simple_driver && make clean && cd ..
cd wifi_driver && make clean && cd ..
cd hardware-manager && make clean && cd ..
```

## Troubleshooting

### "Permission denied" error

* Run all commands with `sudo`.

### "Module not found" error

* Make sure the module was compiled correctly: `ls -la *.ko`
* Specify the full path to the module: `sudo insmod ./module.ko`

### "Unknown symbol" error

* Make sure the correct kernel headers are installed
* Check your kernel version: `uname -r`
* Install the appropriate kernel headers

### Kernel headers not found

* Install the correct kernel headers for your system
* Check your kernel version with `uname -r`
* Reinstall the kernel headers package

### Device cannot be created

* Check whether you have permission to write to the `/dev/` directory
* Run the commands with `sudo`

## Resources

* [Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/html/)
* [Kernel.org Documentation](https://www.kernel.org/doc/)
* [Linux Device Drivers Book](https://lwn.net/Kernel/LDD3/)
* [Linux Networking Documentation](https://www.kernel.org/doc/html/latest/networking/)

## Repository Structure

```text
simple-kernel-module/
├── Makefile
├── README.md
├── hello.c
├── char_device/
│   ├── Makefile
│   └── char_device.c
├── hardware-manager/
│   ├── Makefile
│   └── hwmgr.c
├── simple_driver/
│   ├── Makefile
│   └── simple_driver.c
└── wifi_driver/
    ├── Makefile
    └── wifi_driver.c
```

---

**Note:** These modules are intended for educational purposes. Test them thoroughly before using them in a production environment, and make sure you have a deep understanding of kernel programming!

**Author:** emirerdin443-beep
**License:** GPL
