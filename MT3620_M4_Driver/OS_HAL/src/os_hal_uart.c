/*
 * (C) 2005-2019 MediaTek Inc. All rights reserved.
 *
 * Copyright Statement:
 *
 * This MT3620 driver software/firmware and related documentation
 * ("MediaTek Software") are protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. ("MediaTek").
 * You may only use, reproduce, modify, or distribute (as applicable)
 * MediaTek Software if you have agreed to and been bound by this
 * Statement and the applicable license agreement with MediaTek
 * ("License Agreement") and been granted explicit permission to do so within
 * the License Agreement ("Permitted User").  If you are not a Permitted User,
 * please cease any access or use of MediaTek Software immediately.

 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT MEDIATEK SOFTWARE RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES
 * ARE PROVIDED TO RECEIVER ON AN "AS-IS" BASIS ONLY.
 * MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE.
 * MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES
 * MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR
 * OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK DURING THE PRECEDING TWELVE (12) MONTHS FOR SUCH MEDIATEK
 * SOFTWARE AT ISSUE.
 */

#include "os_hal_uart.h"
#include "os_hal_gpio.h"
#include "os_hal_dma.h"

#define MTK_UART_MAX_PORT_NUMBER 6

#define CM4_UART_BASE			0x21040000
#define ISU0_UART_BASE			0x38070500
#define ISU1_UART_BASE			0x38080500
#define ISU2_UART_BASE			0x38090500
#define ISU3_UART_BASE			0x380a0500
#define ISU4_UART_BASE			0x380b0500

static unsigned long uart_base_addr[MTK_UART_MAX_PORT_NUMBER] = {
	CM4_UART_BASE,
	ISU0_UART_BASE,
	ISU1_UART_BASE,
	ISU2_UART_BASE,
	ISU3_UART_BASE,
	ISU4_UART_BASE,
};

#define CM4_UART_CG_BASE	0x30030120
#define ISU0_UART_CG_BASE	0x38070000
#define ISU1_UART_CG_BASE	0x38080000
#define ISU2_UART_CG_BASE	0x38090000
#define ISU3_UART_CG_BASE	0x380a0000
#define ISU4_UART_CG_BASE	0x380b0000

static unsigned long uart_cg_base_addr[MTK_UART_MAX_PORT_NUMBER] = {
	CM4_UART_CG_BASE,
	ISU0_UART_CG_BASE,
	ISU1_UART_CG_BASE,
	ISU2_UART_CG_BASE,
	ISU3_UART_CG_BASE,
	ISU4_UART_CG_BASE,
};

static u8 uart_half_dma_chan[MTK_UART_MAX_PORT_NUMBER-1][2] = {
	/* [0]:tx, [1]:rx */
	{DMA_ISU0_TX_CH0, DMA_ISU0_RX_CH1},
	{DMA_ISU1_TX_CH2, DMA_ISU1_RX_CH3},
	{DMA_ISU2_TX_CH4, DMA_ISU2_RX_CH5},
	{DMA_ISU3_TX_CH6, DMA_ISU3_RX_CH7},
	{DMA_ISU4_TX_CH8, DMA_ISU4_RX_CH9},
};

static u8 uart_vff_dma_chan[MTK_UART_MAX_PORT_NUMBER-1][2] = {
	/* [0]:tx, [1]:rx */
	{VDMA_ISU0_TX_CH13, VDMA_ISU0_RX_CH14},
	{VDMA_ISU1_TX_CH15, VDMA_ISU1_RX_CH16},
	{VDMA_ISU2_TX_CH17, VDMA_ISU2_RX_CH18},
	{VDMA_ISU3_TX_CH19, VDMA_ISU3_RX_CH20},
	{VDMA_ISU4_TX_CH21, VDMA_ISU4_RX_CH22},
};


/**
 * this os special UART structure, need mapping it to mtk_uart_controller
 */
struct mtk_uart_controller_rtos {
	struct mtk_uart_controller *ctlr;

	/* the type based on OS */
	volatile u8 xTX_Queue;
	volatile u8 xRX_Queue;
};

static struct mtk_uart_private
	g_uart_mdata[MTK_UART_MAX_PORT_NUMBER];
static struct mtk_uart_controller
	g_uart_ctlr[MTK_UART_MAX_PORT_NUMBER];
static struct mtk_uart_controller_rtos
	g_uart_ctlr_rtos[MTK_UART_MAX_PORT_NUMBER];

static struct mtk_uart_controller_rtos
	*_mtk_os_hal_uart_get_ctlr(UART_PORT port_num)
{
	return &g_uart_ctlr_rtos[port_num];
}

static void _mtk_os_hal_uart_config_gpio(UART_PORT port_num)
{
	switch (port_num) {
	case OS_HAL_UART_PORT0:
		break;
	case OS_HAL_UART_ISU0:
		mtk_os_hal_gpio_request(OS_HAL_GPIO_26);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_26, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_27);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_27, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_28);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_28, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_29);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_29, OS_HAL_MODE_1);
		break;
	case OS_HAL_UART_ISU1:
		mtk_os_hal_gpio_request(OS_HAL_GPIO_31);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_31, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_32);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_32, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_33);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_33, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_34);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_34, OS_HAL_MODE_1);
		break;
	case OS_HAL_UART_ISU2:
		mtk_os_hal_gpio_request(OS_HAL_GPIO_36);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_36, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_37);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_37, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_38);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_38, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_39);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_39, OS_HAL_MODE_1);
		break;
	case OS_HAL_UART_ISU3:
		mtk_os_hal_gpio_request(OS_HAL_GPIO_66);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_66, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_67);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_67, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_68);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_68, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_69);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_69, OS_HAL_MODE_1);
		break;
	case OS_HAL_UART_ISU4:
		mtk_os_hal_gpio_request(OS_HAL_GPIO_71);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_71, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_72);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_72, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_73);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_73, OS_HAL_MODE_1);
		mtk_os_hal_gpio_request(OS_HAL_GPIO_74);
		mtk_os_hal_gpio_pmx_set_mode(OS_HAL_GPIO_74, OS_HAL_MODE_1);
		break;
	case OS_HAL_UART_MAX_PORT:
		break;
	}
}

static void _mtk_os_hal_uart_free_gpio(UART_PORT port_num)
{
	switch (port_num) {
	case OS_HAL_UART_PORT0:
		break;
	case OS_HAL_UART_ISU0:
		mtk_os_hal_gpio_free(OS_HAL_GPIO_26);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_27);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_28);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_29);
		break;
	case OS_HAL_UART_ISU1:
		mtk_os_hal_gpio_free(OS_HAL_GPIO_31);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_32);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_33);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_34);
		break;
	case OS_HAL_UART_ISU2:
		mtk_os_hal_gpio_free(OS_HAL_GPIO_36);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_37);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_38);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_39);
		break;
	case OS_HAL_UART_ISU3:
		mtk_os_hal_gpio_free(OS_HAL_GPIO_66);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_67);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_68);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_69);
		break;
	case OS_HAL_UART_ISU4:
		mtk_os_hal_gpio_free(OS_HAL_GPIO_71);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_72);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_73);
		mtk_os_hal_gpio_free(OS_HAL_GPIO_74);
		break;
	case OS_HAL_UART_MAX_PORT:
		break;
	}
}

int mtk_os_hal_uart_ctlr_init(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);
	struct mtk_uart_controller *ctlr;

	if (!ctlr_rtos)
		return -UART_EPTR;

	ctlr_rtos->ctlr = &g_uart_ctlr[port_num];

	ctlr = ctlr_rtos->ctlr;
	ctlr->mdata = &g_uart_mdata[port_num];

	ctlr->port_num = port_num;
	ctlr->base = (void __iomem *)uart_base_addr[port_num];
	ctlr->cg_base = (void __iomem *)uart_cg_base_addr[port_num];
	ctlr->baudrate = 115200;
	ctlr->data_bit = UART_DATA_8_BITS;
	ctlr->parity = UART_NONE_PARITY;
	ctlr->stop_bit = UART_STOP_1_BIT;

	_mtk_os_hal_uart_config_gpio(port_num);

	mtk_mhal_uart_sw_reset(ctlr_rtos->ctlr);
	mtk_mhal_uart_enable_clk(ctlr_rtos->ctlr);

	mtk_mhal_uart_hw_init(ctlr_rtos->ctlr);

	return 0;
}

int mtk_os_hal_uart_ctlr_deinit(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	if (!ctlr_rtos)
		return -UART_EPTR;

	_mtk_os_hal_uart_free_gpio(port_num);

	return 0;
}

void mtk_os_hal_uart_cg_gating(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_disable_clk(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_cg_release(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_enable_clk(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_sw_reset(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_sw_reset(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_dumpreg(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_dumpreg(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_set_baudrate(UART_PORT port_num, u32 baudrate)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	ctlr_rtos->ctlr->baudrate = baudrate;
	mtk_mhal_uart_set_baudrate(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_set_format(UART_PORT port_num,
		mhal_uart_data_len data_bit,
		mhal_uart_parity parity,
		mhal_uart_stop_bit stop_bit)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	ctlr_rtos->ctlr->data_bit = data_bit;
	ctlr_rtos->ctlr->parity = parity;
	ctlr_rtos->ctlr->stop_bit = stop_bit;
	mtk_mhal_uart_set_format(ctlr_rtos->ctlr);
}

u8 mtk_os_hal_uart_get_char(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);
	u8 data;

	data = mtk_mhal_uart_getc(ctlr_rtos->ctlr);

	return data;
}

u8 mtk_os_hal_uart_get_char_nowait(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);
	u8 data;

	data = mtk_mhal_uart_getc_nowait(ctlr_rtos->ctlr);

	return data;
}

void mtk_os_hal_uart_put_char(UART_PORT port_num, u8 data)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_putc(ctlr_rtos->ctlr, data);
}

void mtk_os_hal_uart_put_str(UART_PORT port_num, const char *msg)
{
	while (*msg) {
		mtk_os_hal_uart_put_char(port_num, *msg++);
	}
}

int mtk_os_hal_uart_clear_irq_status(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	return mtk_mhal_uart_clear_irq_status(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_set_irq(UART_PORT port_num, u8 irq_flag)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_set_irq(ctlr_rtos->ctlr, irq_flag);
}

void mtk_os_hal_uart_set_hw_fc(UART_PORT port_num, u8 hw_fc)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_set_hw_fc(ctlr_rtos->ctlr, hw_fc);
}

void mtk_os_hal_uart_disable_sw_fc(UART_PORT port_num)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_disable_sw_fc(ctlr_rtos->ctlr);
}

void mtk_os_hal_uart_set_sw_fc(UART_PORT port_num,
	u8 xon1, u8 xoff1, u8 xon2, u8 xoff2, u8 escape_data)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);

	mtk_mhal_uart_set_sw_fc(ctlr_rtos->ctlr, xon1, xoff1,
		xon2, xoff2, escape_data);
}

static int _mtk_os_hal_uart_dma_tx_callback(void *data)
{
	struct mtk_uart_controller_rtos *ctlr_rtos = data;

	ctlr_rtos->xTX_Queue++;
	printf("_mtk_os_hal_uart_dma_tx_callback\r\n");
	return 0;
}

static int _mtk_os_hal_uart_dma_rx_callback(void *data)
{
	struct mtk_uart_controller_rtos *ctlr_rtos = data;

	ctlr_rtos->xRX_Queue++;
	printf("_mtk_os_hal_uart_dma_rx_callback\r\n");
	return 0;
}

static int _mtk_os_hal_uart_wait_for_tx_done(
				struct mtk_uart_controller_rtos
				*ctlr_rtos, int time_ms)
{
	while(ctlr_rtos->xTX_Queue==0){}
	ctlr_rtos->xTX_Queue--;
	return 0;
}

static int _mtk_os_hal_uart_wait_for_rx_done(
				struct mtk_uart_controller_rtos
				*ctlr_rtos, int time_ms)
{
	while(ctlr_rtos->xRX_Queue==0){}
	ctlr_rtos->xRX_Queue--;
	return 0;
}

u32 mtk_os_hal_uart_dma_send_data(UART_PORT port_num,
	u8 *data, u32 len, bool vff_mode)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);
	struct mtk_uart_controller *ctlr;
	int ret, cnt;

	ctlr = ctlr_rtos->ctlr;

	if (!ctlr)
		return -UART_EPTR;

	if (len >= 0x4000) {
		printf("DMA max transfter size is 0x4000\n");
		return -UART_EINVAL;
	}

	ctlr_rtos->xTX_Queue = 0;

	mtk_mhal_uart_dma_tx_callback_register(ctlr,
				_mtk_os_hal_uart_dma_tx_callback,
				(void *)ctlr_rtos);

	mtk_mhal_uart_set_dma(ctlr, true);

	ctlr->vff_dma_mode = vff_mode;
	if (vff_mode)
		ctlr->mdata->dma_tx_ch = uart_vff_dma_chan[port_num-1][0];
	else
		ctlr->mdata->dma_tx_ch = uart_half_dma_chan[port_num-1][0];

	ctlr->mdata->tx_len = len;
	ctlr->mdata->tx_buf = data;
	ctlr->mdata->tx_size = 0;

	mtk_mhal_uart_allocate_dma_tx_ch(ctlr);
	mtk_mhal_uart_dma_tx_config(ctlr);
	mtk_mhal_uart_start_dma_tx(ctlr);

	cnt = len / (ctlr->baudrate / 10) + 1000;
	printf("UART TX DMA Len:%d, timeout:%dms\n", len, cnt);

	ret = _mtk_os_hal_uart_wait_for_tx_done(ctlr_rtos, cnt);
	if (ret) {
		printf("Take UART TX Semaphore timeout!\n");
		mtk_mhal_uart_stop_dma_tx(ctlr);
	}

	mtk_mhal_uart_update_dma_tx_info(ctlr);
	mtk_mhal_uart_release_dma_tx_ch(ctlr);

	mtk_mhal_uart_set_dma(ctlr, false);

	printf("tx_size: %d\n", ctlr->mdata->tx_size);

	return ctlr->mdata->tx_size;
}

u32 mtk_os_hal_uart_dma_get_data(UART_PORT port_num,
	u8 *data, u32 len, bool vff_mode)
{
	struct mtk_uart_controller_rtos *ctlr_rtos =
		_mtk_os_hal_uart_get_ctlr(port_num);
	struct mtk_uart_controller *ctlr;
	int ret, cnt;

	ctlr = ctlr_rtos->ctlr;

	if (!ctlr)
		return -UART_EPTR;

	if (len >= 0x4000) {
		printf("DMA max transfter size is 0x4000\n");
		return -UART_EINVAL;
	}

	ctlr_rtos->xRX_Queue = 0;

	mtk_mhal_uart_dma_rx_callback_register(ctlr,
				_mtk_os_hal_uart_dma_rx_callback,
				(void *)ctlr_rtos);

	mtk_mhal_uart_set_dma(ctlr, true);

	ctlr->vff_dma_mode = vff_mode;
	if (vff_mode)
		ctlr->mdata->dma_rx_ch = uart_vff_dma_chan[port_num-1][1];
	else
		ctlr->mdata->dma_rx_ch = uart_half_dma_chan[port_num-1][1];

	ctlr->mdata->rx_len = len;
	ctlr->mdata->rx_buf = data;
	ctlr->mdata->rx_size = 0;

	mtk_mhal_uart_allocate_dma_rx_ch(ctlr);
	mtk_mhal_uart_dma_rx_config(ctlr);
	mtk_mhal_uart_start_dma_rx(ctlr);

	cnt = len / (ctlr->baudrate / 10) + 5000;
	printf("UART RX DMA Len:%d, timeout:%dms\n", len, cnt);

	ret = _mtk_os_hal_uart_wait_for_rx_done(ctlr_rtos, cnt);
	if (ret) {
		printf("Take UART RX Semaphore timeout!\n");
		mtk_mhal_uart_stop_dma_rx(ctlr);
	}

	mtk_mhal_uart_update_dma_rx_info(ctlr);
	mtk_mhal_uart_release_dma_rx_ch(ctlr);

	mtk_mhal_uart_set_dma(ctlr, false);

	printf("rx_size: %d!\n", ctlr->mdata->rx_size);

	return ctlr->mdata->rx_size;
}

