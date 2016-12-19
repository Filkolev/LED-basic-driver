# Simple LED Driver

This is a basic implementation of a character device driver which maps the
physical address of a GPIO and manipulates the respective bits in order to turn
the LED on or off.

This driver is compiled and tested on Raspberry Pi 3 Model B running Raspbian
(Linux raspberrypi 4.4.34-v7+ #930). The LED is connected to the Pi via a
breadboard.

## Using the Driver

The driver's binary (along with a simple test program) is compiled natively on
the Raspberry. Both binary files can be recompiled by issuing `make` in the
source directory.

The driver should be loaded using the following command (as root):  
`insmod led.ko [gpio_num=<gpio>]`

Here `gpio_num` is a parameter (defaults to 6) and can only be set at module
load time.

This creates the character device which is located at `/dev/simple-led`. Reading
the file will return either "0" (if the LED is currently off) or "1" (if the LED
is on). Writing "0" to the file will turn the LED off whereas writing anything
else will turn it on.

The module can be unloaded using this command (as root): `sudo rmmod led`

The test program, called `led-test` will simply open the file descriptor, read
the current LED value and print it, wait for the user to enter a string
(presumably "0" or "1"), writes the received string to the character device and
finally prints the LED value once again. It should be run as root.

## Working with Physical Addresses

The GPIO register information is taken from Broadcom's
[BCM2835 ARM Peripherals](https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf)
document. It provides the layout of the registers on pages 90 and onwards. Note
that the addresses shown in the tables are virtual ones; we're only interested
in the physical addresses.

The document is not up-to-date with respect to Raspberry Pi 3 which uses
BCM2837. The only significant error is the wrong range of physical addresses
quoted for peripherals - from 0x20000000 to 0x20FFFFFF. The actual beginning is
0x3f000000.

The offset for GPIO peripherals is unchanged (0x200000), thus GPIO physical
addresses start at 0x3f200000.

All registers are 32 bits wide; a more detailed description follows.

### Function Select Registers

There are 6 function select registers, called GPFSEL<n>, where n is [0..5].
Each GPIO occupies 3 bits, therefore each 32-bit register contains 10 triplets
of bits for 10 GPIOs. 

As far as this driver is concerned, the only thing interesting about these
registers is the ability to set a given GPIO as either input or output. Input is
defined as the combination `000` and output is defined as `001`.

For example, we want to set GPIO 6 to output. The respective bits are in
GPFSEL0. The correct register can be found by dividing the GPIO number by 10
(the count of triplets per register), thus GPIO 11 will be controlled by
GPFSEL1, GPIO 20 - by GPFSEL2 and so on.

Within the function select register, the correct triplet starts at position
`(gpio_num % 10) * 3`. For GPIO 6 the starting bit is 18, the triplet occupying
bits 18-20 (also confirmed on page 91 of the BCM document).

Therefore, in order to set GPIO 6 as output, the driver sets bits 18-20 of
GPFSEL0 to `001`.

### Output Set Registers

TODO

### Output Clear Registers

TODO

### Pin Level Registers

TODO
