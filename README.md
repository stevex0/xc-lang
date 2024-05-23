# The XC Programming Language

**XC** is a statically typed, compiled, procedural programming language designed for general-purpose use.

## Getting Started
Compiling and running the XC compiler is quite simple and straightforward.

### Prerequisites
Ensure that you have the following dependencies installed on your system:
 * `make`
 * `g++`, `clang++`, or any other C++ compiler that supports `C++17`
    > [!NOTE]
	> By default it will compile with `g++`, so if you decide to not use `g++`, please assign the compiler of your choice when using make.

### Building
Once you have those dependencies install, all you need to do is go to the `build` directory and run:
```bash
make COMPILER="g++"
```
> [!NOTE]
> The `COMPILER` variable can be omitted if you want to compile the project with `g++`. If you decide to use another C++ compiler, replace `g++` with your desired c++ compiler.

> [!CAUTION]
> Any C++ compiler other than `g++` has not been tested. You have been warn.

If all goes well, there will be a new executable file called `xc` located in the `build/bin` directory.

### Running
Now all that is left to do it is to run it by:
```bash
./xc
```

## Project Organization
The XC project is organized as follows:
```
.
├── build/
│   ├── bin/
│   ├── obj/
│   └── Makefile
├── docs/
├── src/
│   └── include/
├── .gitignore
├── LICENSE
└── README.md
```

| Directory | Description | 
| - | - |
| `build/` | Contains the `Makefile` used to build the project. During the build process, object files will be generated in the `build/obj` directory. Once the build is complete, the executable will be located in the `build/bin` directory. |
| `docs/` | Contains additional documentation, including the language's *grammar* and *specifications*. |
| `src/` | Contains the C++ source code files. Header files are located in the `src/include` directory. |

## License
[GNU GPL-3.0](LICENSE)