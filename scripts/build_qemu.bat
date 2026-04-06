@echo off
if not exist ".qemu\Makefile" (
	echo "*** Now installing QEMU build environment...(It runs only once.) ***"
	rm -rf .qemu
	if exist ".qemu" (
		echo *E: Can't delete .qemu folder, you should this folder manually...
		exit /b 1
	)
	git clone https://gitlab.com/qemu-project/qemu.git .qemu
	codegen scripts/touch_project.lua
	cd .qemu
	touch .TestDrive.nosearch
	git checkout tags/v11.0.0-rc2 -b testdrive_qemu
	git submodule update --init --recursive
	git apply --ignore-whitespace --whitespace=nowarn ../scripts/build_qemu.diff
	cd include
	mklink /D /J testdrive_common %TESTDRIVE_PROFILE:/=\%Common\include
	mklink /D /J testdrive_system %TESTDRIVE_DIR:/=\%include
	mklink /D /J LuaBridge %TESTDRIVE_PROFILE:/=\%Common\utils\lib_src\LuaBridge
	mklink /D /J testdrive ..\..\include
	cd ../hw/misc
	mklink /D /J testdrive ..\..\..\src
	cd ../../
	mklink /D /J testdrive_framework %TESTDRIVE_PROFILE:/=\%Common\utils\framework
	%TESTDRIVE_DIR%bin\msys64\msys2_shell.cmd -defterm -here -no-start -ucrt64 -c "./configure --target-list=x86_64-softmmu,aarch64-softmmu,riscv64-softmmu,ppc64-softmmu --prefix=%TESTDRIVE_PROFILE%Common/bin/"
	cd ..
)

codegen scripts/touch_project.lua

echo *** Build QEMU... ***
cd .qemu
%TESTDRIVE_DIR%bin\msys64\msys2_shell.cmd -defterm -here -no-start -ucrt64 -c "make %*"
cd ..
