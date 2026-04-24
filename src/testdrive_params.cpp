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
// Rev.  : 4/24/2026 Fri (clonextop@gmail.com)
//================================================================================
#include "testdrive_device.h"

TESTDRIVE_PARAM testdrive_param = {0};

bool			testdrive_init(void)
{
	// PCI settings
	testdrive_param.pci.main.vendor_id = GetConfigurationValue("MAIN_VENDOR_ID", 0x1234);
	if (!testdrive_param.pci.main.vendor_id) {
		printf("*E: 'MAIN_VENDOR_ID' must be specific number.\n");
		return false;
	}

	testdrive_param.pci.main.device_id = GetConfigurationValue("MAIN_DEVICE_ID");
	testdrive_param.pci.sub.vendor_id  = GetConfigurationValue("SUB_VENDOR_ID");
	testdrive_param.pci.sub.device_id  = GetConfigurationValue("SUB_DEVICE_ID");
	testdrive_param.pci.class_id	   = GetConfigurationValue("CLASS_ID");
	testdrive_param.pci.revision	   = GetConfigurationValue("REVISION");
	static cstring rom_path			   = GetConfiguration("ROM_FILE");

	if (!rom_path.IsEmpty()) {
		testdrive_param.pci.rom_file = rom_path.c_str();
	}

	return true;
}

TESTDRIVE_DISPLAY_FORMAT testdrive_display_format(const char *sFormat)
{
	TESTDRIVE_DISPLAY_FORMAT format			 = (TESTDRIVE_DISPLAY_FORMAT)-1;
	static const char		*__format_list[] = {
		"A8R8G8B8", "X8R8G8B8", "A8B8G8R8", "X8B8G8R8", "B8G8R8A8", "B8G8R8X8", "R8G8B8A8",
		"R8G8B8X8", "R8G8B8",	"B8G8R8",	"R5G6B5",	"A1R5G5B5", "X1R5G5B5",
	};
	cstring s(sFormat);
	return (TESTDRIVE_DISPLAY_FORMAT)s.RetrieveTag(__format_list, ARRAY_SIZE(__format_list));
}
