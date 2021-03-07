# spd

## ddr4

### xmp 2.0

#### header

byte | value
-----|-------
384  | XMP magic number byte 1 - 0x0c
385  | XMP magic number byte 2 - 0x4a
386  | bit zero - profile 1 enable <br> bit one - profile 2 enable <br> bits 2-3 - profile 1 dimms per channel - 1 <br> bits 3-4 - profile 2 dimms per channel - 1
387  | bits 0-3 - XMP minor version number <br> bits 4-7 - XMP major version number

#### profiles

profile 1 starts @ byte 393 <br>
profile 2 starts @ byte 440 <br>

byte offset given are offset from starting byte of profile

mtb represents a multiple of 125 picoseconds
ftb is signed twos complement of picoseconds

time in picoseconds = mtb * 125 + ftb 

rfc values are unsigned short

byte offset | value
------------|-------
0           | bit 7 - 0 or 1 volts <br> bits 0-6 centivolts <br> voltage in mV = volts * 1000 + centivolts * 10
3           | memory clock mtb
8           | tCL mtb
9           | tRCD mtb
10          | tRP mtb
11          | bits 0-3 - tRAS upper nibble <br> bits 4-7 - tRC upper nibble
12          | tRAS mtb
13          | tRC mtb
14          | tRFC byte 1
15          | tRFC byte 2
16          | tRFC2 byte 1
17          | tRFC2 byte 2
18          | tRFC4 byte 1
19          | tRFC4 byte 2
20          | bite 0-3 - tFAW upper nibble
21          | tFAW mtb
22          | RRD_S mtb
23          | RRD_L mtb
32          | RRD_L ftb
33          | RRD_S ftb
34          | tRC ftb
35          | tRP ftb
36          | tRCD ftb
37          | tCL ftb
38          | memory clock ftb
