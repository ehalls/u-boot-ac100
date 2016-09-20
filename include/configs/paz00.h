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

/* LCD support */
#define CONFIG_LCD
#define CONFIG_LCD_BMP_RLE8
#define CONFIG_LCD_CONSOLE_ANSI
#define CONFIG_PWM_TEGRA
#define CONFIG_VIDEO_TEGRA
#define LCD_BPP LCD_COLOR16
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_VIDEO_SW_CURSOR

/* Boot menu options */
#define CONFIG_CMDLINE_EDITING		/* add command line history */
#define CONFIG_AUTO_COMPLETE		/* add autocompletion support */

#define CONFIG_CMD_BOOTMENU		/* ANSI terminal Boot Menu */
#define CONFIG_CMD_CLEAR		/* ANSI terminal clear screen command */

#define CONFIG_MENU

/*
 * Miscellaneous configurable options
 */

#define BOARD_EXTRA_ENV_SETTINGS \
    "bootmenu_0=Nand (ext2)=ext2load mmc 0:1 0x1000000 /boot/zImage; ext2load mmc 0:1 0x2200000 /boot/initrd; bootz 0x1000000 0x2200000\0" \
	"bootmenu_1=SD (ext2)=ext2load mmc 1:1 0x1000000 /boot/zImage; ext2load mmc 1:1 0x2200000 /boot/initrd; bootz 0x1000000 0x2200000\0" \
	"bootmenu_2=USBfat (fat)=fatload usb 0:1 0x1000000 /boot/zImage; fatload usb 0:1 0x2200000 /boot/initrd; bootz 0x1000000 0x2200000\0" \
	"bootmenu_3=USBext (ext2)=ext2load usb 0:1 0x1000000 /boot/zImage; ext2load usb 0:1 0x2200000 /boot/initrd; bootz 0x1000000 0x2200000\0" \
    "bootmenu_4=NandFallback (ext2)=ext2load mmc 0:1 0x1000000 /boot/zBackup; bootz 0x1000000 -;\0" 

#define CONFIG_BOOTCOMMAND "bootmenu"

#include "tegra-common-post.h"
#include "tegra-common-usb-gadget.h"

#endif /* __CONFIG_H */
