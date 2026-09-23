# dining-philosophers
Simulation of the dining philosophers problem.
## Usage
Make sure you have installed 'ncurses.h' library. 
```bash
sudo apt-get install libncurses-dev
```
Then compile the program using a compiler of choice:
```bash
g++ -o philosophers philosophers.cpp -lncurses
```
Execute the program by passing one argument corresponding to the integer number of philosophers:
```bash
./philosophers 5
```
## Features
- Handles deadlock and starvation cases,
- Uses condition variable and mutex for managing shared resources,
- Utilizing 'ncurses.h' library helps with real-time visualization of executed program.


