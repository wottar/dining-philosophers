# dining-philosophers
Simulation of the dining philosophers problem.

## Requirements
Make sure you have installed the `ncurses` library:
```bash
sudo apt install libncurses-dev
```
## Build and Usage
Build the project using CMake:
```bash
cmake -B build
cmake --build build
```
Execute the program by passing one argument corresponding to the integer number of philosophers:
```bash
./build/philosophers 5
```
## Features
- Handles deadlock and starvation cases
- Condition variable, mutexes and unique locks for managing shared resources
- Utilizing 'ncurses' library helps with real-time visualization of simulation


