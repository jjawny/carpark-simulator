![unit](https://img.shields.io/badge/CAB4O3-Systems%20Programming-ff69b4?style=plastic)
![author](https://img.shields.io/badge/Author-Johnny%20Madigan-yellow?style=plastic)
![year](https://img.shields.io/badge/Year-2021-lightgrey?style=plastic)
![lang](https://img.shields.io/badge/Lang-informational?style=plastic&logo=C)

- [About](#about)
- [Usage](#usage)
- [Notes](#notes)

# **About**
My major systems programming project, demonstrating my skills in multi-threading, inter-process communication, complex data flows, and handling race-conditions/deadlocks.

The project consists of 3 seperate pieces of software, working together to simulate, automate, and protect a car park:

- A simulator that simulates all of the hardware (license plate recognition sensors, boom gates, digital signs, etc), the temperature, and the movement of cars.

- A management system, responsible for decision-making and automation of the sim's hardware to ensure smooth operation and accurate reporting of the car park.

- A fire-alarm (safety critical) system, that uses 2 algorithms for detecing possible fires (rise/spike in temperature).

![Car Park Simulation demonstration](/docs/ezgif-carpark-demo.gif)

# **Usage**
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

# **Notes**
I used a Linux VM to complete this project. You can download the virtual machine image in my VM repo. Next, download *Oracle*'s [*VirtualBox*](https://www.virtualbox.org) for your OS and the **extension pack**. Launch *VirtualBox*, add the image and you'll be good to go.

Please do not modify the project structure, as it's setup so you can easily re-configure, clean, and re-build the car park simulator over and over. Once you run `Make`, feel free to move the executables wherever you like. But the ***SIMULATOR***, ***MANAGER***, and ***plates.txt*** **must** stay in the same folder, as the sim and manager need to ***read plates.txt***.