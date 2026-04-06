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
#include <stdbool.h>
#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/pci/msi.h"
#include "hw/core/qdev-properties.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qapi/visitor.h"
#include "testdrive_device.h"

#define TESTDRIVE_DEVICE_NAME "testdrive"

typedef struct {
	PCIDevice	  pdev;
	MemoryRegion  mem;
	unsigned char data[1024 * 1024 * 1];
} TESTDRIVE_DEVICE;

DECLARE_INSTANCE_CHECKER(TESTDRIVE_DEVICE, TESTDRIVE_DEV, TESTDRIVE_DEVICE_NAME)

// memory access from device : dma_memory_read() / dma_memory_write()

static uint64_t gpu_mem_read(TESTDRIVE_DEVICE *dev, hwaddr offset, unsigned byte_size)
{
	uint64_t bitcount = ((uint64_t)byte_size) << 3;
	uint64_t mask	  = (1ULL << bitcount) - 1;
	uint64_t got	  = dev->data[offset] & mask;
	printf("Virtual TestDrive[PCIe Read] : 0x%x bytes at 0x%llX = 0x%llX\n", byte_size, offset, got);
	return got;
}
static void gpu_mem_write(TESTDRIVE_DEVICE *dev, hwaddr offset, uint64_t val, unsigned byte_size)
{
	uint64_t bitcount = ((uint64_t)byte_size) << 3;
	uint64_t mask	  = (1ULL << bitcount) - 1;
	uint64_t sizedval = val & mask;
	dev->data[offset] = sizedval;
	printf("Virtual TestDrive[PCIe Write] : 0x%llX [0x%llX] (0x%x bytes) at 0x%llX\n", val, sizedval, byte_size, offset);
}

static const MemoryRegionOps gpu_mem_ops = {
	.read  = (uint64_t (*)(void *, hwaddr, unsigned))gpu_mem_read,
	.write = (void (*)(void *, hwaddr, uint64_t, unsigned))gpu_mem_write,
};

static void device_realize(PCIDevice *pdev, Error **errp)
{
	TESTDRIVE_DEVICE *dev = TESTDRIVE_DEV(pdev);
	memory_region_init_io(&dev->mem, OBJECT(dev), &gpu_mem_ops, dev, "testdrive-bar0", 1 * MiB);
	pci_register_bar(pdev, 0, PCI_BASE_ADDRESS_SPACE_MEMORY, &dev->mem);

	// enable bus master : but some guest OS(linux) will turn off this.
	pci_default_write_config(pdev, PCI_COMMAND, pci_get_word(pdev->config + PCI_COMMAND) | PCI_COMMAND_MASTER, 2);

	// setup MSI
	if (testdrive_param.pci.msi.vector_count) {
		msi_init(pdev, 0x50, testdrive_param.pci.msi.vector_count, true, testdrive_param.pci.msi.per_vector_mask, NULL);
	}
}

static void device_exit(PCIDevice *pdev)
{
	if (testdrive_param.pci.msi.vector_count) {
		msi_uninit(pdev);
	}
}

static void device_instance_init(Object *obj) {}

/*
// not useful for this time.
static const Property testdrive_properties[] = {
	DEFINE_PROP_UINT16("vendor_id", TESTDRIVE_DEVICE, prop.main.vendor_id, PCI_VENDOR_ID_QEMU),
	DEFINE_PROP_UINT16("device_id", TESTDRIVE_DEVICE, prop.main.device_id, 0x3000),
	// DEFINE_PROP_UINT16("sub_vender_id", TESTDRIVE_DEVICE, prop.sub.vender_id, 0),
	// DEFINE_PROP_UINT16("sub_device_id", TESTDRIVE_DEVICE, prop.sub.device_id, 0),
	// DEFINE_PROP_UINT8("class_id", TESTDRIVE_DEVICE, prop.class_id, PCI_CLASS_OTHERS),
};
*/

static void device_class_init(ObjectClass *class, const void *data)
{
	PCIDeviceClass *pc = PCI_DEVICE_CLASS(class);

	// device_class_set_props(&pc->parent_class, testdrive_properties);
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
	if (testdrive_param_init()) {
		static InterfaceInfo interfaces[] = {
			{INTERFACE_PCIE_DEVICE},
			{NULL},
		};
		if (testdrive_param.pci.interface_type) {
			interfaces[0].type = testdrive_param.pci.interface_type;
		}

		static const TypeInfo device_info = {
			.name		   = TESTDRIVE_DEVICE_NAME,
			.parent		   = TYPE_PCI_DEVICE,
			.instance_size = sizeof(TESTDRIVE_DEVICE),
			.instance_init = device_instance_init,
			.class_init	   = device_class_init,
			.interfaces	   = interfaces,
		};

		type_register_static(&device_info);
	} else {
		printf("*E: No TestDrive device description. Device will not attached.\n");
	}
}
// constructor
type_init(testdrive_device_register)
