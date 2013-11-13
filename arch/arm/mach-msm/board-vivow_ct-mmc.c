/* linux/arch/arm/mach-msm/board-vivow-mmc.c
 *
 * Copyright (C) 2008 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/gpio.h>
#include <linux/mfd/pmic8058.h>

#include <linux/gpio.h>
#include <linux/io.h>

#include <mach/vreg.h>
#include <mach/htc_pwrsink.h>

#include <asm/mach/mmc.h>

#include "devices.h"
#include "board-vivow_ct.h"
#include "proc_comm.h"
#include "pm.h"
#include "pm-boot.h"

#define VIVOW_CT_SDMC_CD_N_TO_SYS PM8058_GPIO_PM_TO_SYS(VIVOW_CT_GPIO_SDMC_CD_N)

extern int msm_add_sdcc(unsigned int controller, struct mmc_platform_data *plat);

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
	[MSM_PM_SLEEP_MODE_APPS_SLEEP] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 8594,
		.residency = 23740,
	},
/*
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 4594,
		.residency = 23740,
	},
*/
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_STANDALONE] = {
#ifdef CONFIG_MSM_STANDALONE_POWER_COLLAPSE
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 0,
#else /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.idle_supported = 0,
		.suspend_supported = 0,
		.idle_enabled = 0,
		.suspend_enabled = 0,
#endif /*CONFIG_MSM_STANDALONE_POWER_COLLAPSE*/
		.latency = 500,
		.residency = 6000,
	},
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 0,
		.suspend_enabled = 1,
		.latency = 443,
		.residency = 1098,
	},
	[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT] = {
		.idle_supported = 1,
		.suspend_supported = 1,
		.idle_enabled = 1,
		.suspend_enabled = 1,
		.latency = 2,
		.residency = 0,
	},
};


/* ---- SDCARD ---- */
#if 0
static uint32_t sdcard_on_gpio_table[] = {
	PCOM_GPIO_CFG(58, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_16MA), /* CLK */
	PCOM_GPIO_CFG(59, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_10MA), /* CMD */
	PCOM_GPIO_CFG(60, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT3 */
	PCOM_GPIO_CFG(61, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT2 */
	PCOM_GPIO_CFG(62, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT1 */
	PCOM_GPIO_CFG(63, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT0 */
};

static uint32_t sdcard_off_gpio_table[] = {
	PCOM_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), /* CLK */
	PCOM_GPIO_CFG(59, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_10MA), /* CMD */
	PCOM_GPIO_CFG(60, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_10MA), /* DAT3 */
	PCOM_GPIO_CFG(61, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_10MA), /* DAT2 */
	PCOM_GPIO_CFG(62, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_10MA), /* DAT1 */
	PCOM_GPIO_CFG(63, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_10MA), /* DAT0 */
};

static uint opt_disable_sdcard;

static uint32_t movinand_on_gpio_table[] = {
	PCOM_GPIO_CFG(64, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), /* CLK */
	PCOM_GPIO_CFG(65, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* CMD */
	PCOM_GPIO_CFG(66, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT3 */
	PCOM_GPIO_CFG(67, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT2 */
	PCOM_GPIO_CFG(68, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT1 */
	PCOM_GPIO_CFG(69, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT0 */
	PCOM_GPIO_CFG(115, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT4 */
	PCOM_GPIO_CFG(114, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT5 */
	PCOM_GPIO_CFG(113, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT6 */
	PCOM_GPIO_CFG(112, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_10MA), /* DAT7 */
};
static int __init vivow_ct_disablesdcard_setup(char *str)
{
	int cal = simple_strtol(str, NULL, 0);

	opt_disable_sdcard = cal;
	return 1;
}

__setup("board_vivo_w_ct.disable_sdcard=", vivow_ct_disablesdcard_setup);

static struct vreg *vreg_sdslot;	/* SD slot power */

struct mmc_vdd_xlat {
	int mask;
	int level;
};

static struct mmc_vdd_xlat mmc_vdd_table[] = {
	{ MMC_VDD_28_29,	2850 },
	{ MMC_VDD_29_30,	2900 },
};

static unsigned int sdslot_vdd = 0xffffffff;
static unsigned int sdslot_vreg_enabled;

static uint32_t vivow_ct_sdslot_switchvdd(struct device *dev, unsigned int vdd)
{
	int i;

	BUG_ON(!vreg_sdslot);

	if (vdd == sdslot_vdd)
		return 0;

	sdslot_vdd = vdd;

	if (vdd == 0) {
		printk(KERN_INFO "%s: Disabling SD slot power\n", __func__);
		config_gpio_table(sdcard_off_gpio_table,
				  ARRAY_SIZE(sdcard_off_gpio_table));
		vreg_disable(vreg_sdslot);
		sdslot_vreg_enabled = 0;
		return 0;
	}

	if (!sdslot_vreg_enabled) {
		mdelay(5);
		vreg_enable(vreg_sdslot);
		udelay(500);
		config_gpio_table(sdcard_on_gpio_table,
				  ARRAY_SIZE(sdcard_on_gpio_table));
		sdslot_vreg_enabled = 1;
	}

	for (i = 0; i < ARRAY_SIZE(mmc_vdd_table); i++) {
		if (mmc_vdd_table[i].mask == (1 << vdd)) {
			printk(KERN_INFO "%s: Setting level to %u\n",
				__func__, mmc_vdd_table[i].level);
			vreg_set_level(vreg_sdslot, mmc_vdd_table[i].level);
			return 0;
		}
	}

	printk(KERN_ERR "%s: Invalid VDD %d specified\n", __func__, vdd);
	return 0;
}

static unsigned int vivow_ct_sdslot_status(struct device *dev)
{
	unsigned int status;

	status = (unsigned int) gpio_get_value(VIVOW_SDMC_CD_N_TO_SYS);

	return (!status);
}
#endif

#define VIVOW_CT_MMC_VDD		(MMC_VDD_28_29 | MMC_VDD_29_30)

#if 0
static unsigned int vivow_ct_sdslot_type = MMC_TYPE_SD;

static struct mmc_platform_data vivow_ct_sdslot_data = {
	.ocr_mask	= VIVOW_MMC_VDD,
	.status_irq	= MSM_GPIO_TO_INT(VIVOW_CT_SDMC_CD_N_TO_SYS),
	.status		= vivow_ct_sdslot_status,
	.translate_vdd	= vivow_ct_sdslot_switchvdd,
	.slot_type	= &vivow_ct_sdslot_type,
	.dat0_gpio	= 63,
};

static unsigned int vivow_ct_emmcslot_type = MMC_TYPE_MMC;
static struct mmc_platform_data vivow_ct_movinand_data = {
	.ocr_mask	=  VIVOW_CT_MMC_VDD,
	.slot_type	= &vivow_ct_emmcslot_type,
	.mmc_bus_width  = MMC_CAP_8_BIT_DATA,
};
#endif
/* ---- WIFI ---- */

static uint32_t wifi_on_gpio_table[] = {
	PCOM_GPIO_CFG(116, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(117, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(118, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT1 */
	PCOM_GPIO_CFG(119, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT0 */
	PCOM_GPIO_CFG(111, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), /* CMD */
	PCOM_GPIO_CFG(110, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), /* CLK */
	PCOM_GPIO_CFG(147, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_4MA), /* WLAN IRQ */
};

static uint32_t wifi_off_gpio_table[] = {
	PCOM_GPIO_CFG(116, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT3 */
	PCOM_GPIO_CFG(117, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT2 */
	PCOM_GPIO_CFG(118, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT1 */
	PCOM_GPIO_CFG(119, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA), /* DAT0 */
	PCOM_GPIO_CFG(111, 0, GPIO_INPUT, GPIO_PULL_UP, GPIO_4MA), /* CMD */
	PCOM_GPIO_CFG(110, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA), /* CLK */
	PCOM_GPIO_CFG(147, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_4MA), /* WLAN IRQ */
};

static void config_gpio_table(uint32_t *table, int len)
{
		int n, rc;
		for (n = 0; n < len; n++) {
				rc = gpio_tlmm_config(table[n], GPIO_CFG_ENABLE);
				if (rc) {
						pr_err("%s: gpio_tlmm_config(%#x)=%d\n",
								__func__, table[n], rc);
						break;
				}
		}
}

/* BCM4329 returns wrong sdio_vsn(1) when we read cccr,
 * we use predefined value (sdio_vsn=2) here to initial sdio driver well
 */
static struct embedded_sdio_data vivow_ct_wifi_emb_data = {
	.cccr	= {
		.sdio_vsn	= 2,
		.multi_block	= 1,
		.low_speed	= 0,
		.wide_bus	= 0,
		.high_power	= 1,
		.high_speed	= 1,
	},
};

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int
vivow_ct_wifi_status_register(void (*callback)(int card_present, void *dev_id),
				void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int vivow_ct_wifi_cd;	/* WiFi virtual 'card detect' status */

static unsigned int vivow_ct_wifi_status(struct device *dev)
{
	return vivow_ct_wifi_cd;
}

static unsigned int vivow_ct_wifislot_type = MMC_TYPE_SDIO_WIFI;
static struct mmc_platform_data vivow_ct_wifi_data = {
	.ocr_mask		= MMC_VDD_28_29,
	.status			= vivow_ct_wifi_status,
	.register_status_notify	= vivow_ct_wifi_status_register,
	.embedded_sdio		= &vivow_ct_wifi_emb_data,
	.slot_type          = &vivow_ct_wifislot_type,
		.mmc_bus_width  = MMC_CAP_4_BIT_DATA,
		.msmsdcc_fmin   = 144000,
		.msmsdcc_fmid   = 24576000,
		.msmsdcc_fmax   = 49152000,
		.nonremovable   = 0,
};

int vivow_ct_wifi_set_carddetect(int val)
{
	printk(KERN_INFO "%s: %d\n", __func__, val);
	vivow_ct_wifi_cd = val;
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		printk(KERN_WARNING "%s: Nobody to notify\n", __func__);
	return 0;
}
EXPORT_SYMBOL(vivow_ct_wifi_set_carddetect);

#if 0
static struct pm8058_gpio pmic_gpio_sleep_clk_output = {
	.direction      = PM_GPIO_DIR_OUT,
	.output_buffer  = PM_GPIO_OUT_BUF_CMOS,
	.output_value   = 0,
	.pull           = PM_GPIO_PULL_NO,
	.vin_sel        = PM_GPIO_VIN_S3,      /* S3 1.8 V */
	.out_strength   = PM_GPIO_STRENGTH_HIGH,
	.function       = PM_GPIO_FUNC_2,
};

#define ID_WIFI	0
#define ID_BT	1
#define CLK_OFF	0
#define CLK_ON	1
static DEFINE_SPINLOCK(vivow_ct_w_b_slock);
int vivow_ct_sleep_clk_state_wifi = CLK_OFF;
int vivow_ct_sleep_clk_state_bt = CLK_OFF;

int vivow_ct_wifi_bt_sleep_clk_ctl(int on, int id)
{
	int err = 0;
	unsigned long flags;

	printk(KERN_DEBUG "%s ON=%d, ID=%d\n", __func__, on, id);

	spin_lock_irqsave(&vivow_ct_w_b_slock, flags);
	if (on) {
		if ((CLK_OFF == vivow_ct_sleep_clk_state_wifi)
			&& (CLK_OFF == vivow_ct_sleep_clk_state_bt)) {
			printk(KERN_DEBUG "EN SLEEP CLK\n");
			pmic_gpio_sleep_clk_output.function = PM_GPIO_FUNC_2;
			err = pm8058_gpio_config(
					VIVOW_CT_GPIO_WIFI_BT_SLEEP_CLK_EN,
					&pmic_gpio_sleep_clk_output);
			if (err) {
				spin_unlock_irqrestore(&vivow_ct_w_b_slock,
							flags);
				printk(KERN_DEBUG "ERR EN SLEEP CLK, ERR=%d\n",
					err);
				return err;
			}
		}

		if (id == ID_BT)
			vivow_ct_sleep_clk_state_bt = CLK_ON;
		else
			vivow_ct_sleep_clk_state_wifi = CLK_ON;
	} else {
		if (((id == ID_BT) && (CLK_OFF == vivow_ct_sleep_clk_state_wifi))
			|| ((id == ID_WIFI)
			&& (CLK_OFF == vivow_ct_sleep_clk_state_bt))) {
			printk(KERN_DEBUG "DIS SLEEP CLK\n");
			pmic_gpio_sleep_clk_output.function
					= PM_GPIO_FUNC_NORMAL;
			err = pm8058_gpio_config(
					VIVOW_CT_GPIO_WIFI_BT_SLEEP_CLK_EN,
					&pmic_gpio_sleep_clk_output);
			if (err) {
				spin_unlock_irqrestore(&vivow_ct_w_b_slock,
							flags);
				printk(KERN_DEBUG "ERR DIS SLEEP CLK, ERR=%d\n",
					err);
				return err;
			}
		} else {
			printk(KERN_DEBUG "KEEP SLEEP CLK ALIVE\n");
		}

		if (id)
			vivow_ct_sleep_clk_state_bt = CLK_OFF;
		else
			vivow_ct_sleep_clk_state_wifi = CLK_OFF;
	}
	spin_unlock_irqrestore(&vivow_ct_w_b_slock, flags);

	return 0;
}
EXPORT_SYMBOL(vivow_ct_wifi_bt_sleep_clk_ctl);
#endif

int vivow_ct_wifi_power(int on)
{
	printk(KERN_INFO "%s: %d\n", __func__, on);

	if (on) {
		config_gpio_table(wifi_on_gpio_table,
				ARRAY_SIZE(wifi_on_gpio_table));
	} else {
		config_gpio_table(wifi_off_gpio_table,
				ARRAY_SIZE(wifi_off_gpio_table));
	}

	/*vivow_wifi_bt_sleep_clk_ctl(on, ID_WIFI);*/
	gpio_set_value(VIVOW_CT_GPIO_WIFI_SHUTDOWN_N, on); /* WIFI_SHUTDOWN */
	mdelay(120);
	return 0;
}
EXPORT_SYMBOL(vivow_ct_wifi_power);

int vivow_ct_wifi_reset(int on)
{
	printk(KERN_INFO "%s: do nothing\n", __func__);
	return 0;
}

int __init vivow_ct_init_mmc(unsigned int sys_rev)
{
	uint32_t id;
	wifi_status_cb = NULL;
	/*sdslot_vreg_enabled = 0;*/

	printk(KERN_INFO "vivow_ct: %s\n", __func__);
	/* SDC2: MoviNAND */
#if 0
	register_msm_irq_mask(INT_SDC2_0);
	register_msm_irq_mask(INT_SDC2_1);
	config_gpio_table(movinand_on_gpio_table,
			  ARRAY_SIZE(movinand_on_gpio_table));
	msm_add_sdcc(2, &vivow_ct_movinand_data, 0, 0);
#endif

	/* initial WIFI_SHUTDOWN# */
	id = PCOM_GPIO_CFG(VIVOW_CT_GPIO_WIFI_SHUTDOWN_N, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
	msm_proc_comm(PCOM_RPC_GPIO_TLMM_CONFIG_EX, &id, 0);
	gpio_set_value(VIVOW_CT_GPIO_WIFI_SHUTDOWN_N, 0);

	vivow_ct_wifi_data.swfi_latency =
		msm_pm_data[MSM_PM_SLEEP_MODE_WAIT_FOR_INTERRUPT].latency;

	msm_add_sdcc(3, &vivow_ct_wifi_data);

#if 0
	register_msm_irq_mask(INT_SDC4_0);
	register_msm_irq_mask(INT_SDC4_1);

	if (opt_disable_sdcard) {
		printk(KERN_INFO "vivow_ct: SD-Card interface disabled\n");
		goto done;
	}

	vreg_sdslot = vreg_get(0, "gp10");
	if (IS_ERR(vreg_sdslot))
		return PTR_ERR(vreg_sdslot);

	set_irq_wake(MSM_GPIO_TO_INT(VIVOW_CT_SDMC_CD_N_TO_SYS), 1);

	msm_add_sdcc(4, &vivow_ct_sdslot_data,
			MSM_GPIO_TO_INT(VIVOW_CT_SDMC_CD_N_TO_SYS),
			IORESOURCE_IRQ_LOWEDGE | IORESOURCE_IRQ_HIGHEDGE);
done:
#endif
	/* reset eMMC for write protection test */
	gpio_set_value(VIVOW_CT_GPIO_EMMC_RST, 0);	/* this should not work!!! */
	udelay(100);
	gpio_set_value(VIVOW_CT_GPIO_EMMC_RST, 1);

	return 0;
}
