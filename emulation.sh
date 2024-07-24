#!/bin/bash

icpx -fsycl -fintelfpga -DFPGA_EMULATOR *.cpp -o build/fpga_emulation

echo Producing Emulation in \'build\' folder ... 

echo Run with ./fpga_emulation 
