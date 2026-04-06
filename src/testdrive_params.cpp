//================================================================================
// Copyright (c) 2013 ~ 2026. HyungKi Jeong(clonextop@gmail.com)
// Freely available under the terms of the 3-Clause BSD License
// (https://opensource.org/licenses/BSD-3-Clause)
//
// Redistribution and use in source and binary forms,
// with or without modification, are permitted provided
// that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
// OF SUCH DAMAGE.
//
// Title : QEMU for TestDrive
// Rev.  : 4/9/2026 Thu (clonextop@gmail.com)
//================================================================================
#include "testdrive_device.h"

TESTDRIVE_PARAM testdrive_param = {0};

static string	__GetParam(const char *sName, const char *sSection = "TESTDRIVE_DEVICE")
{
	cstring sEnv, sVal;
	sEnv.Format("%s@%s@qemu_testdrive.ini", sName, sSection);
	if (sVal.GetEnvironment(sEnv)) {
		return sVal.c_string();
	}
	return "";
}

static bool __GetPCIParam(const char *sName, uint32_t &val)
{
	cstring sVal = __GetParam(sName);
	if (!sVal.IsEmpty()) {
		if (sVal.CompareFront("0x")) {
			val = stoi(sVal, NULL, 16);
		} else {
			val = atoi(sVal);
		}
		return true;
	}
	return false;
}

static bool __GetPCIParamBOOL(const char *sName, bool bDefault = false)
{
	cstring s = __GetParam(sName);
	s.MakeLower();
	if (s == "true")
		bDefault = true;
	else if (s == "false")
		bDefault = false;
	return bDefault;
}

bool testdrive_param_init(void)
{
	// clang-format off
	uint32_t temp;
#define GET_PARAM(target, name)	if(__GetPCIParam(name, temp)) {testdrive_param.target = temp;}
	// clang-format on
	GET_PARAM(pci.main.vendor_id, "MAIN_VENDOR_ID");
	if (!testdrive_param.pci.main.vendor_id) {
		printf("*E: Must set specific MAIN_VENDOR_ID.\n");
		return false;
	}
	GET_PARAM(pci.main.device_id, "MAIN_DEVICE_ID");
	GET_PARAM(pci.sub.vendor_id, "SUB_VENDOR_ID");
	GET_PARAM(pci.sub.device_id, "SUB_DEVICE_ID");
	GET_PARAM(pci.class_id, "CLASS_ID");
	GET_PARAM(pci.revision, "REVISION");
	static cstring rom_path = __GetParam("ROM_FILE");
	if (!rom_path.IsEmpty()) {
		testdrive_param.pci.rom_file = rom_path.c_str();
	}
	static cstring interface_type	   = __GetParam("INTERFACE");
	testdrive_param.pci.interface_type = interface_type.IsEmpty() ? "pci-express-device" // = INTERFACE_PCIE_DEVICE
																  : interface_type.c_str();
	GET_PARAM(pci.msi.vector_count, "MSI_VECTOR_COUNT");
	if (testdrive_param.pci.msi.vector_count > 32) {
		printf("*E: MSI count is out of range (0~32)\n");
		return false;
	}
	testdrive_param.pci.msi.per_vector_mask = __GetPCIParamBOOL("MSI_PER_VECTOR_MASK");
	return true;
}
