#!/bin/bash

echo Xstarget \(Device Name or Explicit Board\):

read target

icpx -fsycl -fintelfpga -fsycl-link -Xshardware -Xstarget=$target *.cpp -o report.a

echo Producing Optimisation report in report.prj ... 
