#!/bin/bash 

echo Xstarget \(Device Name or Explicit Board\):

read target

icpx -fsycl -fintelfpga -Xshardware ‐Xstarget=$target *.cpp ‐o fpga_compile.fpga

echo Producing Hardware image in fpga_compile.prj ... 
