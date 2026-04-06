# QEMU for TestDrive project

This project requires a `TestDrive Profiling master` CI environment.


## Build
Since the resulting QEMU binary is very large, we go through the build process from the QEMU git source.  
Run with Eclipse, and press build command `CTRL+B`. (Repeat 2 times for the first run.)

```bash
> git clone https://github.com/testdrive-profiling-master/qemu_testdrive.git
> cd qemu_testdrive
> .eclipse
```

or Run this command. (Repeat 2 times for the first run.)

```bash
> run_as_admin .build_qemu.bat
```

The build results are copied to `$(TESTDRIVE)Common/bin`.

## Run qemu

To use `-accel whpx` for acceleration, you must enable `Windows Hypervisor Platform` and `Hyper-V Platform` on Windows.

```bash
> qemu [argument...]
```
