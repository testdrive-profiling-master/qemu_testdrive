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
// Rev.  : 4/15/2026 Wed (clonextop@gmail.com)
//================================================================================
#ifndef __QEMU_TESTDRIVE_COMMON_H__
#define __QEMU_TESTDRIVE_COMMON_H__
#ifdef __cplusplus
#	define _USE_MATH_DEFINES
#	define USE_TESTDRIVE
#	define SYSTEM_EXPORTS
#	include "STDInterface.h"
#	include <string.h>
#	include "cstring.h"
#	include "TextFile.h"
#	include "ddk/SystemHAL.h"
#	include <map>
#	include <list>
#	include "lua.hpp"
#	include "LuaBridge/LuaBridge.h"

using namespace luabridge;
using namespace std;
extern "C" {
#else
#	include <stdbool.h>
#endif
#include <stdint.h>

typedef enum {
	LOG_MODE_INFO,
	LOG_MODE_WARNING,
	LOG_MODE_ERROR
} LOG_MODE;

void LOG(LOG_MODE id, const char *sFormat, ...);
#define LOGI(...) LOG(LOG_MODE_INFO, __VA_ARGS__)
#define LOGW(...) LOG(LOG_MODE_WARNING, __VA_ARGS__)
#define LOGE(...) LOG(LOG_MODE_ERROR, __VA_ARGS__)
extern int g_log_warning_count;
extern int g_log_error_count;
bool	   IsFileExist(const char *sFileName);

#ifdef __cplusplus
void LOG_Suppress(bool bSuppress = true);

// ini section
string	 GetConfiguration(const char *sName, const char *sSection = "TESTDRIVE_DEVICE");
bool	 GetConfigurationBoolean(const char *sName, bool bDefault = false);
uint64_t GetConfigurationValue(const char *sName, uint64_t uDefault = 0);
}
#endif
#endif //__QEMU_TESTDRIVE_COMMON_H__
