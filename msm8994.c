// SPDX-License-Identifier: BSD-3-Clause
/* Copyright (c) 2022, Linaro Limited */

#include <sys/mman.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "debugcc.h"

static struct gcc_mux gcc = {
	.mux = {
		.phys = 0xfc400000,
		.size = 0x2000,

		.measure = measure_gcc,

		.enable_reg = 0x1880,
		.enable_mask = BIT(16),

		.mux_reg = 0x1880,
		.mux_mask = 0x3ff,

		.div_reg = 0x1880,
		.div_shift = 12,
		.div_mask = 0xf << 12,
		.div_val = 4,
	},

	.xo_div4_reg = 0x10c8,
	.debug_ctl_reg = 0x1884,
	.debug_status_reg = 0x1888,
};

static struct debug_mux mmcc = {
	.phys = 0xfd8c0000,
	.size = 0x5200,
	.block_name = "mm",

	.measure = measure_leaf,
	.parent = &gcc.mux,
	.parent_mux_val = 0x2b,

	.enable_reg = 0x900,
	.enable_mask = BIT(16),

	.mux_reg = 0x900,
	.mux_mask = 0x3ff,
};

static struct measure_clk msm8994_clocks[] = {
	// { "debug_cpu_clk", &gcc.mux, 0x16a },

	{ "gcc_sys_noc_usb3_axi_clk", &gcc.mux, 0x6 },
	{ "gcc_mss_q6_bimc_axi_clk", &gcc.mux, 0x31 },
	{ "gcc_usb30_master_clk", &gcc.mux, 0x50 },
	{ "gcc_usb30_sleep_clk", &gcc.mux, 0x51 },
	{ "gcc_usb30_mock_utmi_clk", &gcc.mux, 0x52 },
	{ "gcc_usb3_phy_aux_clk", &gcc.mux, 0x53 },
	{ "gcc_usb3_phy_pipe_clk", &gcc.mux, 0x54 },
	{ "gcc_sys_noc_ufs_axi_clk", &gcc.mux, 0x58 },
	{ "gcc_usb_hs_system_clk", &gcc.mux, 0x60 },
	{ "gcc_usb_hs_ahb_clk", &gcc.mux, 0x61 },
	{ "gcc_usb2_hs_phy_sleep_clk", &gcc.mux, 0x63 },
	{ "gcc_usb_phy_cfg_ahb2phy_clk", &gcc.mux, 0x64 },
	{ "gcc_sdcc1_apps_clk", &gcc.mux, 0x68 },
	{ "gcc_sdcc1_ahb_clk", &gcc.mux, 0x69 },
	{ "gcc_sdcc2_apps_clk", &gcc.mux, 0x70 },
	{ "gcc_sdcc2_ahb_clk", &gcc.mux, 0x71 },
	{ "gcc_sdcc3_apps_clk", &gcc.mux, 0x78 },
	{ "gcc_sdcc3_ahb_clk", &gcc.mux, 0x79 },
	{ "gcc_sdcc4_apps_clk", &gcc.mux, 0x80 },
	{ "gcc_sdcc4_ahb_clk", &gcc.mux, 0x81 },
	{ "gcc_blsp1_ahb_clk", &gcc.mux, 0x88 },
	{ "gcc_blsp1_qup1_spi_apps_clk", &gcc.mux, 0x8a },
	{ "gcc_blsp1_qup1_i2c_apps_clk", &gcc.mux, 0x8b },
	{ "gcc_blsp1_uart1_apps_clk", &gcc.mux, 0x8c },
	{ "gcc_blsp1_qup2_spi_apps_clk", &gcc.mux, 0x8e },
	{ "gcc_blsp1_qup2_i2c_apps_clk", &gcc.mux, 0x90 },
	{ "gcc_blsp1_uart2_apps_clk", &gcc.mux, 0x91 },
	{ "gcc_blsp1_qup3_spi_apps_clk", &gcc.mux, 0x93 },
	{ "gcc_blsp1_qup3_i2c_apps_clk", &gcc.mux, 0x94 },
	{ "gcc_blsp1_uart3_apps_clk", &gcc.mux, 0x95 },
	{ "gcc_blsp1_qup4_spi_apps_clk", &gcc.mux, 0x98 },
	{ "gcc_blsp1_qup4_i2c_apps_clk", &gcc.mux, 0x99 },
	{ "gcc_blsp1_uart4_apps_clk", &gcc.mux, 0x9a },
	{ "gcc_blsp1_qup5_spi_apps_clk", &gcc.mux, 0x9c },
	{ "gcc_blsp1_qup5_i2c_apps_clk", &gcc.mux, 0x9d },
	{ "gcc_blsp1_uart5_apps_clk", &gcc.mux, 0x9e },
	{ "gcc_blsp1_qup6_spi_apps_clk", &gcc.mux, 0xa1 },
	{ "gcc_blsp1_qup6_i2c_apps_clk", &gcc.mux, 0xa2 },
	{ "gcc_blsp1_uart6_apps_clk", &gcc.mux, 0xa3 },
	{ "gcc_blsp2_ahb_clk", &gcc.mux, 0xa8 },
	{ "gcc_blsp2_qup1_spi_apps_clk", &gcc.mux, 0xaa },
	{ "gcc_blsp2_qup1_i2c_apps_clk", &gcc.mux, 0xab },
	{ "gcc_blsp2_uart1_apps_clk", &gcc.mux, 0xac },
	{ "gcc_blsp2_qup2_spi_apps_clk", &gcc.mux, 0xae },
	{ "gcc_blsp2_qup2_i2c_apps_clk", &gcc.mux, 0xb0 },
	{ "gcc_blsp2_uart2_apps_clk", &gcc.mux, 0xb1 },
	{ "gcc_blsp2_qup3_spi_apps_clk", &gcc.mux, 0xb3 },
	{ "gcc_blsp2_qup3_i2c_apps_clk", &gcc.mux, 0xb4 },
	{ "gcc_blsp2_uart3_apps_clk", &gcc.mux, 0xb5 },
	{ "gcc_blsp2_qup4_spi_apps_clk", &gcc.mux, 0xb8 },
	{ "gcc_blsp2_qup4_i2c_apps_clk", &gcc.mux, 0xb9 },
	{ "gcc_blsp2_uart4_apps_clk", &gcc.mux, 0xba },
	{ "gcc_blsp2_qup5_spi_apps_clk", &gcc.mux, 0xbc },
	{ "gcc_blsp2_qup5_i2c_apps_clk", &gcc.mux, 0xbd },
	{ "gcc_blsp2_uart5_apps_clk", &gcc.mux, 0xbe },
	{ "gcc_blsp2_qup6_spi_apps_clk", &gcc.mux, 0xc1 },
	{ "gcc_blsp2_qup6_i2c_apps_clk", &gcc.mux, 0xc2 },
	{ "gcc_blsp2_uart6_apps_clk", &gcc.mux, 0xc3 },
	{ "gcc_pdm_ahb_clk", &gcc.mux, 0xd0 },
	{ "gcc_pdm2_clk", &gcc.mux, 0xd2 },
	{ "gcc_prng_ahb_clk", &gcc.mux, 0xd8 },
	{ "gcc_bam_dma_ahb_clk", &gcc.mux, 0xe0 },
	{ "gcc_tsif_ahb_clk", &gcc.mux, 0xe8 },
	{ "gcc_tsif_ref_clk", &gcc.mux, 0xe9 },
	{ "gcc_boot_rom_ahb_clk", &gcc.mux, 0xf8 },
	{ "gcc_lpass_q6_axi_clk", &gcc.mux, 0x160 },
	{ "gcc_pcie_0_slv_axi_clk", &gcc.mux, 0x1e8 },
	{ "gcc_pcie_0_mstr_axi_clk", &gcc.mux, 0x1e9 },
	{ "gcc_pcie_0_cfg_ahb_clk", &gcc.mux, 0x1ea },
	{ "gcc_pcie_0_aux_clk", &gcc.mux, 0x1eb },
	{ "gcc_pcie_0_pipe_clk", &gcc.mux, 0x1ec },
	{ "gcc_pcie_1_slv_axi_clk", &gcc.mux, 0x1f0 },
	{ "gcc_pcie_1_mstr_axi_clk", &gcc.mux, 0x1f1 },
	{ "gcc_pcie_1_cfg_ahb_clk", &gcc.mux, 0x1f2 },
	{ "gcc_pcie_1_aux_clk", &gcc.mux, 0x1f3 },
	{ "gcc_pcie_1_pipe_clk", &gcc.mux, 0x1f4 },
	{ "gcc_ufs_axi_clk", &gcc.mux, 0x230 },
	{ "gcc_ufs_ahb_clk", &gcc.mux, 0x231 },
	{ "gcc_ufs_tx_cfg_clk", &gcc.mux, 0x232 },
	{ "gcc_ufs_rx_cfg_clk", &gcc.mux, 0x233 },
	{ "gcc_ufs_tx_symbol_0_clk", &gcc.mux, 0x234 },
	{ "gcc_ufs_tx_symbol_1_clk", &gcc.mux, 0x235 },
	{ "gcc_ufs_rx_symbol_0_clk", &gcc.mux, 0x236 },
	{ "gcc_ufs_rx_symbol_1_clk", &gcc.mux, 0x237 },

	{ "mmsscc_mmssnoc_ahb", &mmcc, 0x1 },
	{ "oxili_gfx3d_clk", &mmcc, 0xd },
	{ "mmss_misc_ahb_clk", &mmcc, 0x3 },
	{ "mmss_mmssnoc_axi_clk", &mmcc, 0x4 },
	{ "mmss_s0_axi_clk", &mmcc, 0x5 },
	{ "ocmemcx_ocmemnoc_clk", &mmcc, 0x9 },
	{ "oxilicx_ahb_clk", &mmcc, 0xc },
	{ "venus0_vcodec0_clk", &mmcc, 0xe },
	{ "venus0_axi_clk", &mmcc, 0xf },
	{ "venus0_ocmemnoc_clk", &mmcc, 0x10 },
	{ "venus0_ahb_clk", &mmcc, 0x11 },
	{ "mdss_mdp_clk", &mmcc, 0x14 },
	{ "mdss_pclk0_clk", &mmcc, 0x16 },
	{ "mdss_pclk1_clk", &mmcc, 0x17 },
	{ "mdss_extpclk_clk", &mmcc, 0x18 },
	{ "venus0_core0_vcodec_clk", &mmcc, 0x1a },
	{ "venus0_core1_vcodec_clk", &mmcc, 0x1b },
	{ "mdss_vsync_clk", &mmcc, 0x1c },
	{ "mdss_hdmi_clk", &mmcc, 0x1d },
	{ "mdss_byte0_clk", &mmcc, 0x1e },
	{ "mdss_byte1_clk", &mmcc, 0x1f },
	{ "mdss_esc0_clk", &mmcc, 0x20 },
	{ "mdss_esc1_clk", &mmcc, 0x21 },
	{ "mdss_ahb_clk", &mmcc, 0x22 },
	{ "mdss_hdmi_ahb_clk", &mmcc, 0x23 },
	{ "mdss_axi_clk", &mmcc, 0x24 },
	{ "camss_top_ahb_clk", &mmcc, 0x25 },
	{ "camss_micro_ahb_clk", &mmcc, 0x26 },
	{ "camss_gp0_clk", &mmcc, 0x27 },
	{ "camss_gp1_clk", &mmcc, 0x28 },
	{ "camss_mclk0_clk", &mmcc, 0x29 },
	{ "camss_mclk1_clk", &mmcc, 0x2a },
	{ "camss_mclk2_clk", &mmcc, 0x2b },
	{ "camss_mclk3_clk", &mmcc, 0x2c },
	{ "camss_cci_cci_clk", &mmcc, 0x2d },
	{ "camss_cci_cci_ahb_clk", &mmcc, 0x2e },
	{ "camss_phy0_csi0phytimer_clk", &mmcc, 0x2f },
	{ "camss_phy1_csi1phytimer_clk", &mmcc, 0x30 },
	{ "camss_phy2_csi2phytimer_clk", &mmcc, 0x31 },
	{ "camss_jpeg_jpeg0_clk", &mmcc, 0x32 },
	{ "camss_jpeg_jpeg1_clk", &mmcc, 0x33 },
	{ "camss_jpeg_jpeg2_clk", &mmcc, 0x34 },
	{ "camss_jpeg_jpeg_ahb_clk", &mmcc, 0x35 },
	{ "camss_jpeg_jpeg_axi_clk", &mmcc, 0x36 },
	{ "camss_ahb_clk", &mmcc, 0x37 },
	{ "camss_vfe_vfe0_clk", &mmcc, 0x38 },
	{ "camss_vfe_vfe1_clk", &mmcc, 0x39 },
	{ "camss_vfe_cpp_clk", &mmcc, 0x3a },
	{ "camss_vfe_cpp_ahb_clk", &mmcc, 0x3b },
	{ "camss_vfe_vfe_ahb_clk", &mmcc, 0x3c },
	{ "camss_vfe_vfe_axi_clk", &mmcc, 0x3d },
	{ "oxili_rbbmtimer_clk", &mmcc, 0x3e },
	{ "camss_csi_vfe0_clk", &mmcc, 0x3f },
	{ "camss_csi_vfe1_clk", &mmcc, 0x40 },
	{ "camss_csi0_clk", &mmcc, 0x41 },
	{ "camss_csi0_ahb_clk", &mmcc, 0x42 },
	{ "camss_csi0phy_clk", &mmcc, 0x43 },
	{ "camss_csi0rdi_clk", &mmcc, 0x44 },
	{ "camss_csi0pix_clk", &mmcc, 0x45 },
	{ "camss_csi1_clk", &mmcc, 0x46 },
	{ "camss_csi1_ahb_clk", &mmcc, 0x47 },
	{ "camss_csi1phy_clk", &mmcc, 0x48 },
	{ "camss_csi1rdi_clk", &mmcc, 0x49 },
	{ "camss_csi1pix_clk", &mmcc, 0x4a },
	{ "camss_csi2_clk", &mmcc, 0x4b },
	{ "camss_csi2_ahb_clk", &mmcc, 0x4c },
	{ "camss_csi2phy_clk", &mmcc, 0x4d },
	{ "camss_csi2rdi_clk", &mmcc, 0x4e },
	{ "camss_csi2pix_clk", &mmcc, 0x4f },
	{ "camss_csi3_clk", &mmcc, 0x50 },
	{ "camss_csi3_ahb_clk", &mmcc, 0x51 },
	{ "camss_csi3phy_clk", &mmcc, 0x52 },
	{ "camss_csi3rdi_clk", &mmcc, 0x53 },
	{ "camss_csi3pix_clk", &mmcc, 0x54 },
	{ "camss_ispif_ahb_clk", &mmcc, 0x55 },
	{ "venus0_core2_vcodec_clk", &mmcc, 0x79 },
	{ "camss_vfe_cpp_axi_clk", &mmcc, 0x7a },
	{ "camss_jpeg_dma_clk", &mmcc, 0x7b },
	{ "fd_core_clk", &mmcc, 0x89 },
	{ "fd_core_uar_clk", &mmcc, 0x8a },
	{ "fd_axi_clk", &mmcc, 0x8b },
	{ "fd_ahb_clk", &mmcc, 0x8c },
	{}
};

struct debugcc_platform msm8994_debugcc = {
	"msm8994",
	msm8994_clocks,
};
