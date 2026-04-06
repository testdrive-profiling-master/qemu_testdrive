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
#ifndef __QEMU_TESTDRIVE_DEVICE_H__
#define __QEMU_TESTDRIVE_DEVICE_H__
#ifdef __cplusplus
#	include "STDInterface.h"
#	include "cstring.h"
extern "C" {
#endif

// testdrive parameters
typedef struct {
	struct {
		const char *interface_type;
		const char *rom_file;
		struct {
			uint16_t vendor_id;
			uint16_t device_id;
		} main, sub;
		uint8_t class_id;
		uint8_t revision;
		struct {
			uint8_t vector_count;
			bool	per_vector_mask;
		} msi;
	} pci;
} TESTDRIVE_PARAM;
extern TESTDRIVE_PARAM testdrive_param;
//
bool testdrive_param_init(void);

void TestDrive_test(void);

#ifdef __cplusplus
}
#endif
#endif //__QEMU_TESTDRIVE_DEVICE_H__
