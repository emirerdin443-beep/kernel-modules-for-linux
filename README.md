# Simple Kernel Module

Hello. As a Turkish developer, this is my first repository and my first kernel modules. I will add more modules and other features in the future. Enjoy using them.

## About the Project

This repository contains basic examples for loading new modules into the Linux kernel. You can get an in-depth introduction to kernel programming by using different types of kernel modules.

## Modules

The following kernel modules are included in this repository:

### 1. Hello Module

- **Description:** A simple "Hello World" kernel module
- **Directory:** `hello/`
- **Files:** `hello.c`, `Makefile`
- **Functionality:** Prints load and unload messages to the kernel

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

## Requirements

- Linux operating system
- Kernel headers (kernel development files)
- GCC compiler
- Make

### For Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential linux-headers-$(uname -r)
````

### For RHEL/CentOS:

```bash
sudo yum groupinstall "Development Tools"
sudo yum install kernel-devel-$(uname -r)
```

## General Build Instructions

To build each module, navigate to the corresponding directory and run the `make` command:

```bash
cd <module_directory>
make
```

This command will create the `.ko` file.

## Module Usage

### Hello Module

```bash
# Navigate to the directory
cd hello

# Build
make

# Load the module
sudo insmod hello.ko

# Check the output
dmesg | tail -n 1

# Remove the module
sudo rmmod hello
```

**Expected Output:**

```
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
ifconfig -a

# Remove the module
sudo rmmod wifi_driver
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
```

## Cleaning

To remove compiled files from each module's directory:

```bash
cd <module_directory>
make clean
```

To clean all modules:

```bash
cd hello && make clean && cd ..
cd char_device && make clean && cd ..
cd simple_driver && make clean && cd ..
cd wifi_driver && make clean && cd ..
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

```
simple-kernel-module/
├── hello/
│   ├── hello.c
│   └── Makefile
├── char_device/
│   ├── char_device.c
│   └── Makefile
├── simple_driver/
│   ├── simple_driver.c
│   └── Makefile
├── wifi_driver/
│   ├── wifi_driver.c
│   └── Makefile
└── README.md
```

---

**Note:** These modules are intended for educational purposes. Test them thoroughly before using them in a production environment, and make sure you have a deep understanding of kernel programming!

**Author:** emirerdin443-beep
**License:** GPL

```
```
