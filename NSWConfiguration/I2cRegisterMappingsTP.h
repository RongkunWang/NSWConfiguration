#ifndef SCAX_H
#define SCAX_H

/*
 * ----------------------------------------------------------------------------------
 * -- Company:  The Weizmann Institute of Science
 * -- Engineer: Noam Inbari (noam.inbari@weizmann.ac.il)
 * --
 * -- Copyright Notice/Copying Permission:
 * --    Copyright 2019 Noam Inbari
 * --
 * --    This file is part of the SCA_eXtension_firmware (SCX).
 * --
 * --    SCX is free software: you can redistribute it and/or modify
 * --    it under the terms of the GNU General Public License as published by
 * --    the Free Software Foundation, either version 3 of the License, or
 * --    (at your option) any later version.
 * --
 * --    SCX is distributed in the hope that it will be useful,
 * --    but WITHOUT ANY WARRANTY; without even the implied warranty of
 * --    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * --    GNU General Public License for more details.
 * --
 * --    You should have received a copy of the GNU General Public License
 * --    along with SCX.  If not, see <http://www.gnu.org/licenses/>.
 * --
 * -- Create Date: 10.07.2019 18:08:12
 * -- Design Name: Register File 0
 * -- Module Name: registerFile_package_0 - RTL
 * -- Project Name: SCX
 * -- Description: Package for TP Registers I2C
 * --
 * -- Dependencies:
 * --
 * -- Changelog:
 * --
 * ----------------------------------------------------------------------------------
 */

 	// -------------	sTGC_alg	-------------
	static const char TP_NAME_sTGC_alg[] ="sTGC_alg";
 	static const i2c::AddressRegisterMap TP_REGISTERS_sTGC_alg = 
	{
		{ "radial_offset_00", { {"register", 12 } } }, 
		{ "plane_offset_01", { {"register", 16 } } }, 
		{ "max_cluster_size_02", { {"register", 4 } } }, 
		{ "min_cluster_size_03", { {"register", 4 } } }, 
		{ "charge_thresh_cluster_04", { {"register", 6 } } }, 
		{ "charge_thresh_centroid_05", { {"register", 6 } } }, 
		{ "strip_dir_06", { {"register", 1 } } }, 
		{ "R_offset_07", { {"register", 16 } } }, 
	};  

 	// -------------	MM_addc	-------------
	static const char TP_NAME_MM_addc[] ="MM_addc";
 	static const i2c::AddressRegisterMap TP_REGISTERS_MM_addc = 
	{
		{ "addc_packet_parity_error_00", { {"register", 32 } } }, 
		{ "addc_packet_bc_error_01", { {"register", 32 } } }, 
		{ "addc_packet_format_error_02", { {"register", 32 } } }, 
		{ "addc_packet_BC_offset_03", { {"register", 8 } } }, 
	};  

 	// -------------	MM_lnk	-------------
	static const char TP_NAME_MM_lnk[] ="MM_lnk";
 	static const i2c::AddressRegisterMap TP_REGISTERS_MM_lnk = 
	{
		{ "gbt_link_alligned_00", { {"register", 32 } } }, 
		{ "gbt_pll_locked_01", { {"register", 9 } } }, 
		{ "gbt_link_error_04", { {"register", 32 } } }, 
		{ "gbt_channel_mask_05", { {"register", 32 } } }, 
	};  

 	// -------------	MM	-------------
	static const char TP_NAME_MM[] ="MM";
 	static const i2c::AddressRegisterMap TP_REGISTERS_MM = 
	{
		{ "tp_status_00", { {"register", 32 } } }, 
	};  

 	// -------------	sTGC_V6	-------------
	static const char TP_NAME_sTGC_V6[] ="sTGC_V6";
 	static const i2c::AddressRegisterMap TP_REGISTERS_sTGC_V6 = 
	{
		{ "V6_global_RST_00", { {"register", 1 } } }, 
		{ "V6_UDP_GTX_RST_01", { {"register", 1 } } }, 
		{ "V6_ANLINK_RX_WR_EN_02", { {"register", 1 } } }, 
		{ "V6_ANLINK_RX_RD_EN_03", { {"register", 1 } } }, 
		{ "V6_ANLINK_TX_WR_EN_04", { {"register", 1 } } }, 
		{ "V6_ANLINK_TX_RD_EN_05", { {"register", 1 } } }, 
	};  

 	// -------------	sTGC_lnk	-------------
	static const char TP_NAME_sTGC_lnk[] ="sTGC_lnk";
 	static const i2c::AddressRegisterMap TP_REGISTERS_sTGC_lnk = 
	{
		{ "GTH_MMC_RX_lock_00", { {"register", 3 } } }, 
		{ "GTH_TX_FSM_RESET_DONE_01", { {"register", 36 } } }, 
		{ "GTH_RX_FSM_RESET_DONE_02", { {"register", 36 } } }, 
		{ "GTH_cpllfbclklost_03", { {"register", 36 } } }, 
		{ "GTH_cplllock_04", { {"register", 36 } } }, 
		{ "GTH_rxresetdone_05", { {"register", 36 } } }, 
		{ "GTH_txresetdone_06", { {"register", 36 } } }, 
		{ "GTH_QPLLLOCK_07", { {"register", 12 } } }, 
		{ "GTH_QPLLREFCLKLOST_08", { {"register", 12 } } }, 
		{ "GTH_QPLLRESET_09", { {"register", 12 } } }, 
		{ "GlobalReset_0a", { {"register", 1 } } }, 
		{ "GTH_rxuserrdy_0b", { {"register", 36 } } }, 
		{ "GTH_txuserrdy_0c", { {"register", 36 } } }, 
		{ "GTH_cpllreset_0d", { {"register", 36 } } }, 
		{ "GTH_gtrxreset_0e", { {"register", 36 } } }, 
		{ "GTH_gttxreset_0f", { {"register", 36 } } }, 
		{ "V7_ANLINK_RX_WR_EN_10", { {"register", 1 } } }, 
		{ "V7_ANLINK_RX_RD_EN_11", { {"register", 1 } } }, 
		{ "V7_ANLINK_TX_WR_EN_12", { {"register", 1 } } }, 
		{ "V7_ANLINK_TX_RD_EN_13", { {"register", 1 } } }, 
	};  

 	// -------------	NSW	-------------
	static const char TP_NAME_NSW[] ="NSW";
 	static const i2c::AddressRegisterMap TP_REGISTERS_NSW = 
	{
		{ "Firmware_version__time_stamp_00", { {"register", 32 } } }, 
		{ "EndcapSector_ID_01", { {"register", 5 } } }, 
		{ "XL1ID_02", { {"register", 8 } } }, 
		{ "Run_number_03", { {"register", 32 } } }, 
		{ "orbit_count_04", { {"register", 32 } } }, 
		{ "delta_theta_cut_05", { {"register", 5 } } }, 
		{ "XVC_IP_low_byte_07", { {"register", 8 } } }, 
		{ "BC_offset_08", { {"register", 12 } } }, 
		{ "BC_L1A_window_09", { {"register", 3 } } }, 
		{ "BC_monitoring_window_0a", { {"register", 3 } } }, 
		{ "enable_LIA_data_output_0d", { {"register", 1 } } }, 
	};  

 	// -------------	SL_out	-------------
	static const char TP_NAME_SL_out[] ="SL_out";
 	static const i2c::AddressRegisterMap TP_REGISTERS_SL_out = 
	{
		{ "enable_MM_segments_00", { {"register", 1 } } }, 
		{ "enable_SL_output_01", { {"register", 1 } } }, 
		{ "SL_out_offset_02", { {"register", 3 } } }, 
	};  

 	// -------------	MM_diag	-------------
	static const char TP_NAME_MM_diag[] ="MM_diag";
 	static const i2c::AddressRegisterMap TP_REGISTERS_MM_diag = 
	{
		{ "diag_fifo_ef_00", { {"register", 32 } } }, 
		{ "diag_fifo_of_01", { {"register", 32 } } }, 
		{ "diag_gbt_loopback_data_05", { {"register", 32 } } }, 
	};  

 	// -------------	MM_alg	-------------
	static const char TP_NAME_MM_alg[] ="MM_alg";
 	static const i2c::AddressRegisterMap TP_REGISTERS_MM_alg = 
	{
		{ "algor_strip0_offset_00", { {"register", 16 } } }, 
		{ "algor_plane_offset_01", { {"register", 16 } } }, 
		{ "algor_plane_offset_recip_02", { {"register", 16 } } }, 
		{ "algor_mmfe8_invert_03", { {"register", 32 } } }, 
		{ "algor_roi_map_addr_07", { {"register", 16 } } }, 
		{ "algor_mxlocal_parameters_addr_0b", { {"register", 16 } } }, 
		{ "algor_dtheta_parameters_addr_0f", { {"register", 16 } } }, 
		{ "algor_region_mask_10", { {"register", 16 } } }, 
	};  

	/*
	 * Map-Pairing Register Files Arrays
	 */
	#define NUM_REGISTER_FILES 10

	i2c::AddressRegisterMap registerFilesArr[NUM_REGISTER_FILES] = 
	{ 	
		TP_REGISTERS_sTGC_alg,	
		TP_REGISTERS_MM_addc,	
		TP_REGISTERS_MM_lnk,	
		TP_REGISTERS_MM,	
		TP_REGISTERS_sTGC_V6,	
		TP_REGISTERS_sTGC_lnk,	
		TP_REGISTERS_NSW,	
		TP_REGISTERS_SL_out,	
		TP_REGISTERS_MM_diag,	
		TP_REGISTERS_MM_alg,
	};
	
	std::string registerFilesNamesArr[NUM_REGISTER_FILES] = 
	{ 	
		TP_NAME_sTGC_alg,	
		TP_NAME_MM_addc,	
		TP_NAME_MM_lnk,	
		TP_NAME_MM,	
		TP_NAME_sTGC_V6,	
		TP_NAME_sTGC_lnk,	
		TP_NAME_NSW,	
		TP_NAME_SL_out,	
		TP_NAME_MM_diag,	
		TP_NAME_MM_alg,
	};
#endif