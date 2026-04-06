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
// Rev.  : 4/20/2026 Mon (clonextop@gmail.com)
//================================================================================
#include <stdbool.h>
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/msi.h"
#include "hw/core/qdev-properties.h"
#include "hw/core/sysbus.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qapi/visitor.h"
#include "testdrive_device.h"
#include "ui/console.h"

#define TESTDRIVE_DEVICE_NAME "testdrive"

typedef struct {
	PCIDevice pdev;
	struct {
		MemoryRegion region;
	} mem[6];
	TESTDRIVE	   *pTestDrive;
	DisplaySurface *pSurface;
	QemuConsole	   *con;
	uint32_t		framebuffer[800 * 600 * 4];
} TESTDRIVE_DEVICE;

DECLARE_INSTANCE_CHECKER(TESTDRIVE_DEVICE, TESTDRIVE_DEV, TESTDRIVE_DEVICE_NAME)

// memory access from device : dma_memory_read() / dma_memory_write()

static const MemoryRegionOps testdrive_mem_ops = {
	.read		= (uint64_t (*)(void *, hwaddr, unsigned))testdrive_bar_read,
	.write		= (void (*)(void *, hwaddr, uint64_t, unsigned))testdrive_bar_write,
	.endianness = DEVICE_LITTLE_ENDIAN,
	.valid =
		{
			.min_access_size = 1,
			.max_access_size = 4,
		},
};

static void testdrive_display_invalidate(void *opaque)
{
	printf("invalidated display\n");
}
static void testdrive_display_text_update(void *opaque, console_ch_t *chardata)
{
	printf("updated text\n");
}

static void testdrive_display_update(TESTDRIVE_DEVICE *dev)
{
	// qemu_console_resize(dev->con, 800, 600);
	DisplaySurface *surface = qemu_console_surface(dev->con);
	memcpy(surface_data(surface), dev->framebuffer, 640 * 480 * 4);
	dpy_gfx_update(dev->con, 0, 0, 640, 480);
}

static const GraphicHwOps ghwops = {
	.invalidate	 = testdrive_display_invalidate,
	.gfx_update	 = (void (*)(void *))testdrive_display_update,
	.text_update = testdrive_display_text_update,
};

static void device_realize(PCIDevice *pdev, Error **errp)
{
	TESTDRIVE_DEVICE *dev = TESTDRIVE_DEV(pdev);

	if (!(dev->pTestDrive = testdrive_create())) {
		LOGE("TestDrive device is not ready.");
		exit(1);
	}

	// setup BAR#
	for (int i = 0; i < 6; i++) {
		TESTDRIVE_PCI_BAR *bar = testdrive_param.pci.bar[i];
		if (bar) {
			uint8_t attr = bar->option.bar_type ? PCI_BASE_ADDRESS_SPACE_MEMORY : PCI_BASE_ADDRESS_SPACE_IO;
			if (bar->option.b64bit)
				attr |= PCI_BASE_ADDRESS_MEM_TYPE_64;
			if (bar->option.bPrefetchable)
				attr |= PCI_BASE_ADDRESS_MEM_PREFETCH;
			memory_region_init_io(&dev->mem[bar->option.bar_id].region, OBJECT(dev), &testdrive_mem_ops, bar, bar->name, bar->byte_size);
			pci_register_bar(pdev, bar->option.bar_id, attr, &dev->mem[bar->option.bar_id].region);
		}
	}

	// setup MSI
	if (testdrive_param.pci.msi.vector_count) {
		msi_init(pdev, 0x50, testdrive_param.pci.msi.vector_count, true, testdrive_param.pci.msi.per_vector_mask, NULL);
	}

	// enable bus master : but some guest OS(linux) will turn off this.
	// pci_default_write_config(pdev, PCI_COMMAND, pci_get_word(pdev->config + PCI_COMMAND) | PCI_COMMAND_MASTER, 2);
	/*
		dev->con = graphic_console_init(DEVICE(pdev), 0, &ghwops, dev);
		qemu_console_resize(dev->con, 640, 480);
		DisplaySurface *surface = qemu_console_surface(dev->con);
		for (int i = 0; i < 640 * 480; i++) {
			dev->framebuffer[i] = i; // XRGB
		}*/
	/*
	dev->pSurface = qemu_create_displaysurface_from(640, 480, PIXMAN_x8r8g8b8, 640 * 4, (uint8_t *)dev->framebuffer);
	dpy_gfx_replace_surface(dev->con, dev->pSurface);
	*/
}

static void device_exit(PCIDevice *pdev)
{
	TESTDRIVE_DEVICE *dev = TESTDRIVE_DEV(pdev);
	if (testdrive_param.pci.msi.vector_count) {
		msi_uninit(pdev);
	}
	if (dev->pTestDrive) {
		testdrive_destroy(dev->pTestDrive);
		dev->pTestDrive = NULL;
	}
}

static void device_class_init(ObjectClass *class, const void *data)
{
	PCIDeviceClass *pc		= PCI_DEVICE_CLASS(class);
	pc->parent_class.desc	= "TestDrive Device";
	pc->realize				= device_realize;
	pc->exit				= device_exit;
	pc->vendor_id			= testdrive_param.pci.main.vendor_id;
	pc->device_id			= testdrive_param.pci.main.device_id;
	pc->class_id			= testdrive_param.pci.class_id;
	pc->revision			= testdrive_param.pci.revision;
	pc->subsystem_vendor_id = testdrive_param.pci.sub.vendor_id;
	pc->subsystem_id		= testdrive_param.pci.sub.device_id;
	pc->romfile				= testdrive_param.pci.rom_file;
}

static void testdrive_device_register(void)
{
	if (testdrive_init()) {
		static InterfaceInfo interfaces[] = {
			{INTERFACE_PCIE_DEVICE},
			{NULL},
		};

		static const TypeInfo device_info = {
			.name		   = TESTDRIVE_DEVICE_NAME,
			.parent		   = TYPE_PCI_DEVICE,
			.instance_size = sizeof(TESTDRIVE_DEVICE),
			.class_init	   = device_class_init,
			.interfaces	   = interfaces,
		};

		type_register_static(&device_info);
	} else {
		LOGW("TestDrive device is need to run from TestDrive Profiling Master environment.");
	}
}
// constructor
type_init(testdrive_device_register)
