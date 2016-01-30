/*
 * Copyright (c) 2010-2012 NVIDIA CORPORATION.  All rights reserved.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "tegra20-common.h"

/* High-level configuration options */
#define CONFIG_TEGRA_BOARD_STRING	"Compal Paz00"

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

#define CONFIG_MACH_TYPE		MACH_TYPE_PAZ00

/* Fastboot and USB OTG */
#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_FLASH_MMC_DEV	0
#define CONFIG_FASTBOOT_BUF_SIZE	(PHYS_SDRAM_1_SIZE - SZ_1M)
#define CONFIG_FASTBOOT_BUF_ADDR	CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_FASTBOOT_GPT_NAME "gpt"

/* Partitioning Support */
#define CONFIG_CMD_GPT
#define CONFIG_CMD_UUID
#define CONFIG_CMD_FS_UUID
#define CONFIG_CMD_FAT
#define CONFIG_EFI_PARTITION

/* Ene EC support */
#define CONFIG_CMD_CROS_EC
#define CONFIG_CROS_EC_KEYB

/* SPI */
#define CONFIG_CMD_SPI

/* Sensor */
#define CONFIG_CMD_DTT

/* I2C */
#define CONFIG_SYS_I2C_TEGRA
#define CONFIG_CMD_I2C
#define CONFIG_I2C_EDID

/* USB host support */
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_TEGRA
#define CONFIG_USB_MAX_CONTROLLER_COUNT 3
#define CONFIG_USB_STORAGE
#define CONFIG_CMD_USB
#define CONFIG_USB_KEYBOARD

/* SD/MMC */
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_TEGRA_MMC
#define CONFIG_CMD_MMC

/* Environment in eMMC, at the end of 2nd "boot sector" */
#define CONFIG_ENV_IS_IN_MMC
#define CONFIG_ENV_OFFSET (-CONFIG_ENV_SIZE)
#define CONFIG_SYS_MMC_ENV_DEV 0
#define CONFIG_SYS_MMC_ENV_PART 2

/* USB networking support */
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_RNDIS

/* General networking support */
#define CONFIG_CMD_DHCP

/* LCD support */
#define CONFIG_LCD
#define CONFIG_PWM_TEGRA
#define CONFIG_VIDEO_TEGRA
#define LCD_BPP				LCD_COLOR16
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_CONSOLE_SCROLL_LINES	5

/*
 * Miscellaneous configurable options
 */

#define BOARD_EXTRA_ENV_SETTINGS \
	"baudrate=115200\0" \
	"bootdelay=3\0" \
	"autoload=no\0" \
	"bootscr=boot.scr\0" \
	"fdtfile=paz00.dtb\0" \
	"kernel=zImage-paz00\0" \
	"ramdisk=ramdisk-paz00.img\0" \
	"console=ttyO3\0" \
	"ramdisksize=16384\0" \
	"mmcdev=0\0" \
	"mmcroot=/dev/mmcblk1p2\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"root=${mmcroot} rw rootwait\0" \
	"ramroot=/dev/ram0\0" \
	"ramargs=setenv bootargs console=${console} " \
		"root=${ramroot} ramdisk_size=${ramdisksize} rw\0" \
	"mmcloadkernel=load mmc ${mmcdev} ${loadaddr} ${kernel}\0" \
	"mmcloadfdt=load mmc ${mmcdev} ${fdtaddr} ${fdtfile}\0" \
	"mmcloadramdisk=load mmc ${mmcdev} ${rdaddr} ${ramdisk}\0" \
	"mmcloadbootscript=load mmc ${mmcdev} ${loadaddr} ${bootsrc}\0" \
	"mmcbootscript=echo Running bootscript from mmc${mmcdev}...; " \
			"source ${loadaddr}\0" \
	"mmcbootlinux=echo Booting from mmc${mmcdev} ...; " \
			"bootz ${loadaddr} ${rdaddr} ${fdtaddr}\0" \
	"mmcboot=if mmc dev ${mmcdev} && mmc rescan; then " \
			"if run mmcloadbootscript; " \
				"then run mmcbootscript; " \
			"fi; " \
			"if run mmcloadkernel; then " \
				"if run mmcloadfdt; then " \
					"if run mmcloadramdisk; then " \
						"run ramargs; " \
						"run mmcbootlinux; " \
					"fi; " \
					"run mmcargs; " \
					"setenv rdaddr - ; " \
					"run mmcbootlinux; " \
				"fi; " \
			"fi; " \
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	"run mmcboot || setenv mmcdev 1; setenv mmcroot /dev/mmcblk0p2; run mmcboot;"


#include "tegra-common-post.h"
#include "tegra-common-usb-gadget.h"

#endif /* __CONFIG_H */
