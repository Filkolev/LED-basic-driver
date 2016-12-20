# Simple LED Driver

This is a basic implementation of a character device driver which maps the
physical address of a GPIO and manipulates the respective bits in order to turn
the LED ON or OFF.

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
the file will return either "0" (if the LED is currently OFF) or "1" (if the LED
is ON). Writing "0" to the file will turn the LED OFF whereas writing anything
else will turn it ON.

The module can be unloaded using this command (as root): `sudo rmmod led`

The test program, called `led-test` will simply open the file descriptor, read
the current LED value and print it, wait for the user to enter a string
(presumably "0" or "1"), write the received string to the character device and
finally print the LED value once again. It should be run as root.

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

There are 6 function select registers, called GPFSEL_n_, where n is in the range
0-5.

Each GPIO occupies 3 bits, therefore each 32-bit register contains 10 triplets
of bits for 10 GPIOs. 

As far as this driver is concerned, the only thing interesting about these
registers is the ability to set a given GPIO as output. Output is defined as 
`001`, the other combinations are not relevant.

For example, we want to set GPIO 6 to output. The respective bits are in
GPFSEL0. The correct register can be found by dividing the GPIO number by 10
(the count of triplets per register), thus GPIO 11 will be controlled by
GPFSEL1, GPIO 20 - by GPFSEL2 and so on.

Within the function select register, the correct triplet starts at position
`(gpio_num % 10) * 3`. For GPIO 6 the starting bit is 18, the triplet occupying
bits 18-20 (also shown on table 6-2,
[page 90-91, BCM ARM Peripherals](https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf#page=90)).

Therefore, in order to set GPIO 6 as output, the driver sets bits 18-20 of
GPFSEL0 to `001`.

On module load, the initial value of the function select register responsible
for the GPIO pin is stored and this value is restored when the module is
unloaded.

### Output Set Registers

There are two registers which set the output for the given GPIO (GPSET0 and 
GPSET1), the first one starts at offset 0x1c from the GPIO base address.

Writing a "1" at a given position sets the given GPIO pin, writing a "0" has no
effect (unsetting is done using the
[output clear registers](#output-clear-registers)).

For example, to turn ON a LED on GPIO 6, we need to write a "1" at position 6
of GPSET0 which is responsible for GPIO pins 0-31. GPSET1 is responsible for
pins 32-53 (bits 0-21 respectively; bits 22-31 are reserved). Refer to tables
6-8 and 6-9 on
[page 95, BCM2835 ARM Peripherals](https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf#page=95).

### Output Clear Registers

There are two registers which clear the output for the given GPIO (GPCLR0 and
GPCLR1), the first one starts at offset 0x28 from the GPIO base address.

The usage of the output clear registers is analogous to the
[output set registers](#output-set-registers). Writing a "1" at a given position
clears the output of the specified pin; writing a "0" has no effect. E.g.,
writing "1" at position 6 of GPCLR0 will turn the LED on pin 6 OFF (if it was ON
to begin with).

### Pin Level Registers

There are two read-only registers which can be used to check the level of the
GPIOs - GPLEV0 and GPLEV1, the first one starting at offset 0x34 from the GPIO
base address.

For example, to read the value of pin 6, we check the value of bit 6 of GPLEV0.
GPLEV0 is responsible for pins 0-31, GPLEV1 is responsible for pins 32-53 (bits
0-21; bits 22-31 are reserved). Refer to tables 6-12 and 6-13 on
[page 96, BCM2835 ARM Peripherals](https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf#page=96).
**NB** The tables are wrong as they show that a value "0" reflects both high and
low levels, which is clearly not possible. A "0" means low, while "1" means the
pin is high.
