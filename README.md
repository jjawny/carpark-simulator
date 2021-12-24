![unit](https://img.shields.io/badge/Unit-Systems%20Programming-ff69b4?style=plastic)
![author](https://img.shields.io/badge/Author-Johnny%20Madigan-yellow?style=plastic)
![year](https://img.shields.io/badge/Year-2021-lightgrey?style=plastic)
![lang](https://img.shields.io/badge/Lang-informational?style=plastic&logo=C)

- [About](#about)
- [Usage](#usage)
- [Notes](#notes)
- [cab403-vm](#cab4O3-vm)

# ***About***
My major systems programming project, demonstrating my skills in multi-threading, inter-process communication, complex data flows, and handling race-conditions/deadlocks.

The project consists of 3 seperate pieces of software, working together to simulate, automate, and protect a car park:

- A simulator that simulates all of the hardware (license plate recognition sensors, boom gates, digital signs, etc), the temperature, and the movement of cars.

- A management system, responsible for decision-making and automation of the sim's hardware to ensure smooth operation and accurate reporting of the car park.

- A fire-alarm (safety critical) system, that uses 2 algorithms for detecing possible fires (rise/spike in temperature).

![Car Park Simulation demonstration](/docs/ezgif-carpark-demo.gif)

# ***Usage***
Please see my report and [video](https://www.youtube.com/watch?v=-4QtDzU25co) for more details on usage. 

A summary: after you configure your car park structure and simulation settings in ***config.h***, clean up the old files, then re-build all of the executables. Finally, run the Sim first, then you can run the Manager and Fire-Alarm System

To clean:
```
$ make clean
```

To build:
```
$ make
```

To run the Sim:
```
$ ./SIMULATOR
```

To run the Manager:
```
$ ./MANAGER
```

To run the Fire-Alarm System:
```
$ ./FIRE-ALARM-SYSTEM
```

# ***Notes***
Please do not modify the project structure, as it's setup so you can easily re-configure, clean, and re-build the car park simulator over and over. Once you run `Make`, feel free to move the executables wherever you like. But the ***SIMULATOR***, ***MANAGER***, and ***plates.txt*** **must** stay in the same folder, as the sim and manager need to ***read plates.txt***.

# ***cab4O3-vm***
I used a Linux VM to complete this project. If you would like to run this project in the VM, you can download the virtual machine image [here](https://drive.google.com/file/d/1TiWPam3fcElTRgOOGlEVmpV6JD4MMoQ9/view?usp=sharing) then unzip. The image is 2.5GB zipped and 7.25GB unzipped. To run the MX Linux VM, download *Oracle*'s [*VirtualBox*](https://www.virtualbox.org) for your OS and the **extension pack**. Launch *VirtualBox*, add the image and you'll be good to go.

- Username: cab403
- Password: cab403
- Root password: cab403

![VM settings](/docs/VM-settings.png)