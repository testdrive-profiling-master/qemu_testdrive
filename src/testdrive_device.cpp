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
// Rev.  : 4/23/2026 Thu (clonextop@gmail.com)
//================================================================================
#include "testdrive_device.h"

TESTDRIVE *testdrive_create(void *pdev)
{
	return new TestDrive(pdev);
}

void testdrive_destroy(TESTDRIVE *pTestDrive)
{
	if (pTestDrive) {
		delete (TestDrive *)pTestDrive;
	}
}

TestDrive::TestDrive(void *pdev)
{
	m_pdev			   = pdev;
	m_hSystemImpModule = NULL;
	m_pSystem		   = NULL;
	m_pLua			   = NULL;
	memset(&display, 0, sizeof(TESTDRIVE_DISPLAY));
	cstring sScriptFile = GetConfiguration("SETUP_SCRIPT");

	// run device setup script
	if (!sScriptFile.IsEmpty() && !RunScript(sScriptFile)) {
		exit(1);
	}
}

TestDrive::~TestDrive(void)
{
	if (m_pLua) {
		lua_close(m_pLua);
		m_pLua = NULL;
	}
	SAFE_RELEASE(m_pSystem);

	if (m_hSystemImpModule) {
		FreeLibrary(m_hSystemImpModule);
		m_hSystemImpModule = NULL;
	}
}

bool TestDrive::LoadSystemModule(const char *sFileName)
{
	SAFE_RELEASE(m_pSystem);
	if (m_hSystemImpModule) {
		FreeLibrary(m_hSystemImpModule);
		m_hSystemImpModule = NULL;
	}

	// open system module
	m_hSystemImpModule = LoadLibrary(sFileName);
	if (!m_hSystemImpModule) {
		LOGE("Can't open simulation module : %s", sFileName);
		return false;
	}

	// Create sub-system
	CREATE_SIMULATION CreateSimulationImplementation = (CREATE_SIMULATION)GetProcAddress(m_hSystemImpModule, "CreateSimulationImplementation");
	if (CreateSimulationImplementation) {
		m_pSystem = CreateSimulationImplementation();
		if (m_pSystem) {
			// Initialize sub-system
			if (!m_pSystem->InitializeSimulation(this)) {
				SAFE_RELEASE(m_pSystem);
				LOGE("Can't initialize TestDrive simulation system.");
				return false;
			}
		} else {
			LOGE("Can't create simulation sub-system.");
			return false;
		}
	} else {
		LOGE("Can't find TestDrive simulation creation implementation.");
		return false;
	}
	return true;
}

void TestDrive::MemIO(bool bWrite, uint64_t addr, uint32_t &val)
{
	if (m_pSystem) {
		m_pSystem->io_slave(bWrite, addr, val);
	}
}

uint64_t testdrive_bar_read(TESTDRIVE_PCI_BAR *bar, uint64_t offset, unsigned byte_size)
{
	uint32_t val = 0xFFFFFFFF;
	if (byte_size == 4) {
		((TestDrive *)bar->pTestDrive)->MemIO(false, offset + bar->bind_address, val);
	} else {
		LOGE("Invalid BAR[%d] read : offset(0x%llX) size(%d)", bar->option.bar_id, offset, byte_size);
	}
	return val;
}

void testdrive_bar_write(TESTDRIVE_PCI_BAR *bar, uint64_t offset, uint64_t val, unsigned byte_size)
{
	if (byte_size == 4) {
		((TestDrive *)bar->pTestDrive)->MemIO(true, offset + bar->bind_address, *(uint32_t *)&val);
	} else {
		LOGE("Invalid BAR[%d] write : offset(0x%llX) size(%d)", bar->option.bar_id, offset, byte_size);
	}
}

bool TestDrive::dma_master(uint64_t addr, void *pBuff, uint64_t byte_size, bool bWrite)
{
	return testdrive_dma_master(m_pdev, addr, pBuff, byte_size, bWrite);
}

bool testdrive_display(TESTDRIVE_DISPLAY *pDisplay)
{
	pDisplay->pBuffer = NULL;

	return false;
}
