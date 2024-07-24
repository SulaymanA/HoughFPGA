## Hough\_OneAPI

# Emulation

To run the FPGA Emulation use command:

```
bash emulation.sh 
```

The output (.exe) file will be placed in the 'build' directory, which can be found and run with:

```
cd build
./fpga_emulation
```

# Report Generation


Similarly, one may run the FPGA report via the script:

```
bash report.sh 
``` 

You will be prompted to give an -Xstarget (FPGA Device) to run the report on. You can either provide the device name such as 'Agilex7', device number or device Board. The report will be output to the report.prj folder. 

# FPGA Hardware Image
To create the Hardware Image, run the script:

```
hardware.sh
``` 

Again you will prompted to provide an Xstarget, as in the report, whoch will be of the same form. The output will be in the the folder fpga\_compile.fpga
