# sample-c-vcp
Dracal // SDK code sample for C (posix) on VCP

## Assumptions

Running this repository requires you to have installed:
- GCC (version >= 9.4.0)
- Make (version >= 4.2.1)

This repository makes use of library [**'libcrc'**](https://github.com/lammertb/libcrc/) - which is included as a built object (`libcrc.a`) in this repository. It is compiled based on the latest commit available at the time of writing ([2021-04-21](https://github.com/lammertb/libcrc/commit/7719e2112a9a960b1bba130d02bebdf58e8701f1)).


## Simple usage

Make sure that in `main.c`, variable `dev` corresponds to your local setup (on Linux/Ubuntu, your serial path might be e.g. `/dev/ttyACM0`)

Build
```
make
```

Run
```
./main
```

## Sample output


```
Ξ dracal/sample-c-vcp git:(main) ▶ ./main

Poll interval set to 1000 ms

VCP-PTH450-CAL E24638 @ 2024-03-07 12:54:29
P = 100426 Pa
T = 25.27 C
H = 46.55 %

VCP-PTH450-CAL E24638 @ 2024-03-07 12:54:30
P = 100429 Pa
T = 25.27 C
H = 46.62 %

VCP-PTH450-CAL E24638 @ 2024-03-07 12:54:32
P = 100429 Pa
T = 25.27 C
H = 46.62 %

VCP-PTH450-CAL E24638 @ 2024-03-07 12:54:33
P = 100429 Pa
T = 25.27 C
H = 46.58 %
^C
↑130 dracal/sample-c-vcp git:(main) ▶
```
