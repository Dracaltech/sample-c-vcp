# sample-c-vcp
Dracal // SDK code sample for C (posix) on VCP

## Assumptions

Running this repository requires you to have installed:
- GCC (version >= 9.4.0)
- Make (version >= 4.2.1)

This repository makes use of library [**'libcrc'**](https://github.com/lammertb/libcrc/) - which is included as a built object (`libcrc.a`) in this repository. It is compiled (_Ubuntu 20.04 LTS_) based on the latest commit available at the time of writing ([2021-04-21](https://github.com/lammertb/libcrc/commit/7719e2112a9a960b1bba130d02bebdf58e8701f1)).


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
<img src="https://github.com/Dracaltech/sample-c-vcp/assets/1357711/f97ecc9a-5e57-4e7e-9756-88717bce3e84" width=400 />


```
Ξ dracal/sample-c-vcp git:(main) ▶ ./main

Poll interval set to 1000 ms

VCP-PTH450-CAL E24638 @ 2024-03-08 11:22:09
P = 99688 Pa
T = 21.77 C
H = 49.98 %

VCP-PTH450-CAL E24638 @ 2024-03-08 11:22:10
P = 99686 Pa
T = 21.77 C
H = 49.95 %

VCP-PTH450-CAL E24638 @ 2024-03-08 11:22:12
P = 99685 Pa
T = 21.77 C
H = 49.95 %

VCP-PTH450-CAL E24638 @ 2024-03-08 11:22:13
P = 99685 Pa
T = 21.78 C
H = 49.98 %
^C
↑130 dracal/sample-c-vcp git:(main) ▶ 
```
