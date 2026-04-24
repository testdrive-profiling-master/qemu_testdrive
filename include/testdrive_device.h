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
#include "testdrive_common.h"
#ifndef __QEMU_TESTDRIVE_DEVICE_H__
#	define __QEMU_TESTDRIVE_DEVICE_H__

// PCI BAR
typedef struct {
	void	*pTestDrive; // TestDrive
	uint64_t byte_size;
	uint64_t bind_address;
	char	 name[32];
	union {
		uint32_t m;
		uint8_t	 bar_id;
		struct {
			uint32_t /*bar_id*/	   : 8;
			uint32_t bar_type	   : 1; // 0:I/O, 1:Memory
			uint32_t b64bit		   : 1;
			uint32_t bPrefetchable : 1;
		};
	} option;
} TESTDRIVE_PCI_BAR;

typedef enum {
	TESTDRIVE_DISPLAY_FORMAT_NONE = -1,
	// 32bit format
	TESTDRIVE_DISPLAY_FORMAT_A8R8G8B8 = 0,
	TESTDRIVE_DISPLAY_FORMAT_X8R8G8B8,
	TESTDRIVE_DISPLAY_FORMAT_A8B8G8R8,
	TESTDRIVE_DISPLAY_FORMAT_X8B8G8R8,
	TESTDRIVE_DISPLAY_FORMAT_B8G8R8A8,
	TESTDRIVE_DISPLAY_FORMAT_B8G8R8X8,
	TESTDRIVE_DISPLAY_FORMAT_R8G8B8A8,
	TESTDRIVE_DISPLAY_FORMAT_R8G8B8X8,
	// 24bit format
	TESTDRIVE_DISPLAY_FORMAT_R8G8B8,
	TESTDRIVE_DISPLAY_FORMAT_B8G8R8,
	TESTDRIVE_DISPLAY_FORMAT_R5G6B5,
	// 16bit format
	TESTDRIVE_DISPLAY_FORMAT_A1R5G5B5,
	TESTDRIVE_DISPLAY_FORMAT_X1R5G5B5,
} TESTDRIVE_DISPLAY_FORMAT;
extern const uint32_t g_TESTDRIVE_DISPLAY_FORMAT2PIXMAN[];

typedef struct {
	void *pBuffer;
	union {
		uint64_t settings;
		struct {
			uint16_t width, height, byte_stride;
			uint8_t	 format;
		};
	};
} TESTDRIVE_DISPLAY;

// PCI parameters
typedef struct {
	struct {
		const char *rom_file;
		struct {
			uint16_t vendor_id;
			uint16_t device_id;
		} main, sub;
		uint16_t		   class_id;
		uint8_t			   revision;
		TESTDRIVE_PCI_BAR *bar[6]; // BAR#0~5
		struct {
			uint8_t vector_count;
			bool	per_vector_mask;
		} msi;
	} pci;
} TESTDRIVE_PARAM;
extern TESTDRIVE_PARAM testdrive_param;

typedef struct {
	TESTDRIVE_DISPLAY display;
} TESTDRIVE;

typedef struct {
	TESTDRIVE *pOwner;
	struct {
		uint32_t bar_id		   : 3;
		uint32_t prefetchable  : 1;
		uint32_t address_space : 1; // 0:I/O, 1:Memory
	};
	uint64_t memory_size;
} TESTDRIVE_BAR;

#	ifdef __cplusplus
class TestDrive : public TESTDRIVE, public IVMHost
{
public:
	TestDrive(void *pdev);
	virtual ~TestDrive(void);

	void		 MemIO(bool bWrite, uint64_t addr, uint32_t &val);
	virtual bool dma_master(uint64_t addr, void *pBuff, uint64_t byte_size, bool bWrite);

private:
	bool RunScript(const char *sFileName);
	bool CreateBAR(const char *address_space, uint64_t byte_size, bool b64bit, bool bPrefetchable, uint64_t bind_address);
	bool EnableMSI(int iVectorCount, bool bMaskPerVector);
	bool EnableDisplay(int width, int height, LuaRef disp_address, LuaRef disp_format);
	bool LoadSystemModule(const char *sFileName);

protected:
	void					   *m_pdev; // PCIDevice
	HMODULE						m_hSystemImpModule;
	ISystemSim				   *m_pSystem;
	lua_State				   *m_pLua;
	map<int, TESTDRIVE_PCI_BAR> m_BARs;
};

extern "C" {
#	endif

bool					 testdrive_init(void);
TESTDRIVE				*testdrive_create(void *pdev);
void					 testdrive_destroy(TESTDRIVE *pTestDrive);
uint64_t				 testdrive_bar_read(TESTDRIVE_PCI_BAR *bar, uint64_t offset, unsigned byte_size);
void					 testdrive_bar_write(TESTDRIVE_PCI_BAR *bar, uint64_t offset, uint64_t val, unsigned byte_size);
bool					 testdrive_display(TESTDRIVE *pTestDrive);
bool					 testdrive_dma_master(void *pdev, uint64_t addr, void *pBuff, uint64_t byte_size, bool bWrite);
TESTDRIVE_DISPLAY_FORMAT testdrive_display_format(const char *sFormat);

#	ifdef __cplusplus
}
#	endif
#endif //__QEMU_TESTDRIVE_DEVICE_H__
