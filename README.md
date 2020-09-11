# Lab 2: Mycat

## Team

 - [Denys Maletskyi](https://github.com/maletsden)

## Prerequisites

 - **C++ compiler** - needs to support **C++20** standard
 - **CMake** 3.15+
 
The rest prerequisites (such as development libraries) can be found in the [packages file](./apt_packages.txt) in the form of the apt package manager package names.

## Installing

1. Clone the project.
    ```bash
    git clone git@github.com:maletsden/mycat.git
    ```
2. Install required libraries. On Ubuntu:
   ```bash
   sed 's/\r$//' apt_packages.txt | sed 's/#.*//' | xargs sudo apt-get install -y
   ```
3. Build.
    ```bash
    cmake -G"Unix Makefiles" -Bbuild
    cmake --build build
    ```

## Usage

```bash
mycat [-h|--help] [-A] <file1> <file2> ... <fileN>
```

If no arguments provided, the program will finish with code 0, but nothing will be printed.
Option "-A" will print invisible characters as their hex value.

Help flags `-h`/`--help` support is available.
