/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: hwpf/fapi2/include/plat/hw_access.H $                         */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* EKB Project                                                            */
/*                                                                        */
/* COPYRIGHT 2012,2019                                                    */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/// @file hw_access.H
///
/// @brief Hardware access functions that needs to be specialized for
/// platform implementation.
///

#ifndef _FAPI2_HWACCESS_H_
#define _FAPI2_HWACCESS_H_

// variable_buffer isn't supported on PPE
#ifndef __PPE__
#include <variable_buffer.H>
#endif

#include <fapi2_hw_access.H>
#include <plat_hw_access.H>

extern "C" {
#include <libpdbg.h>
}

#define pdbg_parent(tgt, cls) pdbg_target_parent_virtual(cls, tgt)

namespace fapi2
{
//--------------------------------------------------------------------------
// PIB Error Functions
//--------------------------------------------------------------------------

/// @brief Sets the PIB error mask - platform dependant
/// @param[in]  i_mask     The new error mask
// note: this can be moved  to a C file if desired
inline void setPIBErrorMask(uint8_t i_mask)
{
	// Keeps the compiler from complaining about the unused i_mask
	static_cast<void>(i_mask);

	return;
}

/// @brief Gets the PIB error mask - platform dependant
/// @return uint8_t The current PIB error mask
// note: this can be moved  to a C file if desired
inline uint8_t getPIBErrorMask(void)
{
	return 0;
}

//--------------------------------------------------------------------------
// Operational Mode Error Functions
//--------------------------------------------------------------------------

/// @brief Sets the operational mode
/// @param[in]  i_mode     The new mode
// note: this can be moved  to a C file if desired
inline void setOpMode(const OpModes i_mode)
{
	// Keeps the compiler from complaining about the unused i_mode
	static_cast<void>(i_mode);

	// No-op for now. Should set thread-local operational mode
	return;
}

/// @brief Gets the operational mode
/// @return the operational mode
// note: this can be moved  to a C file if desired
inline OpModes getOpMode(void)
{
	// No-op for now. Should read thread-local operational mode
	return NORMAL;
}

//------------------------------------------------------------------------------
// HW Communication Functions to be implemented at the platform layer.
//------------------------------------------------------------------------------

///
/// @brief Platform-level implementation of getScom()
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  SCOM register address to read from.
/// @param[out] o_data     Buffer that holds data read from HW target.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template <TargetType K, MulticastType M, typename V>
inline ReturnCode getScom(const Target<K, M, V>& i_target,
			  const uint64_t i_address, buffer<uint64_t>& o_data)
{
	struct pdbg_target* target = i_target;
	uint64_t data;
	int rc;

	rc = pib_read(target, i_address, &data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	o_data = data;
	return FAPI2_RC_SUCCESS;
}

/* Returns the pib target for the given proc chip or NULL in case of
 * error. */
inline struct pdbg_target* get_pib_target(struct pdbg_target* proc)
{
	struct pdbg_target* pib = NULL;

	/* We assume each processor chip contains one and only one pib */
	pdbg_for_each_target("pib", proc, pib)
	{
		break;
	}

	if (!pib) {
		FAPI_ERR("No pib target associated with processor %s\n",
			 pdbg_target_path(proc));
		return NULL;
	}

	return pib;
}

template <MulticastType M, typename V>
inline ReturnCode getScom(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_target,
			  const uint64_t i_address, buffer<uint64_t>& o_data)
{
	struct pdbg_target *pib, *proc = i_target;
	uint64_t data;
	int rc;

	if (!(pib = get_pib_target(proc)))
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	rc = pib_read(pib, i_address, &data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	o_data = data;
	return FAPI2_RC_SUCCESS;
}

/// @brief Platform-level implementation of putScom()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     HW target to operate on.
/// @param[in] i_address    SCOM register address to write to.
/// @param[in] i_data       Buffer that holds data to write into address.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template <TargetType K, MulticastType M, typename V>
inline ReturnCode putScom(const Target<K, M, V>& i_target,
			  const uint64_t i_address,
			  const buffer<uint64_t> i_data)
{
	struct pdbg_target* target = i_target;
	int rc;

	rc = pib_write(target, i_address, i_data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	return FAPI2_RC_SUCCESS;
}

template <MulticastType M, typename V>
inline ReturnCode putScom(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_target,
			  const uint64_t i_address,
			  const buffer<uint64_t> i_data)
{
	struct pdbg_target *pib, *proc = i_target;
	int rc;

	if (!(pib = get_pib_target(proc)))
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	rc = pib_write(pib, i_address, i_data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	return FAPI2_RC_SUCCESS;
}

/// @brief Platform-level implementation of putScomUnderMask()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     HW target to operate on.
/// @param[in] i_address    SCOM register address to write to.
/// @param[in] i_data       Buffer that holds data to write into address.
/// @param[in] i_mask       Buffer that holds the mask value.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template <TargetType K, MulticastType M, typename V>
inline ReturnCode putScomUnderMask(const Target<K, M, V>& i_target,
				   const uint64_t i_address,
				   const buffer<uint64_t> i_data,
				   const buffer<uint64_t> i_mask)
{
	struct pdbg_target* target = i_target;
	uint64_t data;
	int rc;

	rc = pib_write_mask(target, i_address, i_data, i_mask);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	return FAPI2_RC_SUCCESS;
}

template <MulticastType M, typename V>
inline ReturnCode
    putScomUnderMask(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_target,
		     const uint64_t i_address, const buffer<uint64_t> i_data,
		     const buffer<uint64_t> i_mask)
{
	struct pdbg_target *pib, *proc = i_target;
	uint64_t data;
	int rc;

	if (!(pib = get_pib_target(proc)))
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	rc = pib_write_mask(pib, i_address, i_data, i_mask);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	return FAPI2_RC_SUCCESS;
}

/* Returns the pib target for the given proc chip or NULL in case of
 * error. */
inline struct pdbg_target* get_fsi_target(struct pdbg_target* proc)
{
	struct pdbg_target* fsi = NULL;

	/* We assume each processor chip contains one and only one pib */
	pdbg_for_each_target("fsi", proc, fsi)
	{
		break;
	}

	if (!fsi) {
		FAPI_ERR("No fsi target associated with processor %s\n",
			 pdbg_target_path(proc));
		return NULL;
	}

	return fsi;
}

///
/// @brief Platform-level implementation called by getCfamRegister()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  CFAM address to read from.
/// @param[out] o_data     32-bit buffer that holds data read from HW target.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template <MulticastType M, typename V>
inline ReturnCode
    getCfamRegister(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_target,
		    const uint32_t i_address, buffer<uint32_t>& o_data)
{
	struct pdbg_target *fsi, *proc = i_target;
	uint32_t data;
	int rc;

	if (!(fsi = get_fsi_target(proc)))
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	rc = fsi_read(fsi, i_address, &data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	o_data = data;
	return FAPI2_RC_SUCCESS;
}

template <MulticastType M, typename V>
inline ReturnCode
    getCfamRegister(const Target<TARGET_TYPE_MEMBUF_CHIP, M, V>& i_target,
		    const uint32_t i_address, buffer<uint32_t>& o_data)
{
	FAPI_ERR("CFAM access for memory buffer chips not implemented on this "
		 "platform\n");

	return FAPI2_RC_FALSE;
}

///
/// @brief Platform-level implementation of putCfamRegister()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  CFAM address to write to.
/// @param[out] i_data     32-bit buffer that holds data to write into address.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template <MulticastType M, typename V>
inline ReturnCode
    putCfamRegister(const Target<TARGET_TYPE_PROC_CHIP, M, V>& i_target,
		    const uint32_t i_address, const buffer<uint32_t> i_data)
{
	struct pdbg_target *fsi, *proc = i_target;
	int rc;

	if (!(fsi = get_fsi_target(proc)))
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	rc = fsi_write(fsi, i_address, i_data);
	if (rc)
		return FAPI2_RC_PLAT_ERR_SEE_DATA;

	return FAPI2_RC_SUCCESS;
}

/// @brief Platform-level implementation  of putRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in] i_target   Target to operate on.
/// @param[in] i_ringID   Ring ID that will identify the Ring in the image.
/// @param[in] i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template <TargetType K, MulticastType M, typename V>
ReturnCode putRing(const Target<K, M, V>& i_target, const RingId_t i_ringID,
		   const RingMode i_ringMode)
{
	return FAPI2_RC_SUCCESS;
}

// variable_buffer isn't supported on PPE
#ifndef __PPE__
///
/// @brief Platform-level implementation of getRing()
/// Hardware procedures writers will not call this function.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   HW target to operate on.
/// @param[in]  i_address  Ring address to read from.
/// @param[out] o_data     Buffer that holds ring data read from HW target.
/// @param[in]  i_ringMode Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
template <TargetType K, MulticastType M, typename V>
inline ReturnCode getRing(const Target<K, M, V>& i_target,
			  const scanRingId_t i_address, variable_buffer& o_data,
			  const RingMode i_ringMode)
{
	return FAPI2_RC_SUCCESS;
}

/// @brief Platform-level implementation of modifyRing()
/// @tparam K template parameter, passed in target.
/// @param[in] i_target     Target to operate on.
/// @param[in] i_address    Ring address to modify.
/// @param[in] i_data       Buffer that contains RS4 compressed ring data
///                         to be modified.
/// @param[in] i_modifyMode The modify mode (or/and/xor)
/// @param[in] i_ringMode   Ring operation mode.
/// @return fapi::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
template <TargetType K, MulticastType M, typename V>
inline ReturnCode
    modifyRing(const Target<K, M, V>& i_target, const scanRingId_t i_address,
	       const variable_buffer& i_data,
	       const ChipOpModifyMode i_modifyMode, const RingMode i_ringMode)
{
	return FAPI2_RC_SUCCESS;
}
#endif

// --------------------------------------------------------------------------
// NOTE:
// Implement platform Spy access functions if platform supports them.
// --------------------------------------------------------------------------

// variable_buffer isn't supported on PPE
#ifndef __PPE__
/// @brief Reads a spy from a chip.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   Target to operate on.
/// @param[in]  i_spyId    Id of the spy whose data to be read.
/// @param[out] o_data     Buffer that holds data read from HW target.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependant on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         1. if the spy name contains a # procedure writers should replace
///         it with an __P__ for example -
///
///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///         becomes
///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
///
///         2. if the spy name has a number following a "." it must have an
///         underscore prepended to the number.
///
///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
///         becomes
///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
///
///         Example SPY name:
///         The hardware procedure should call the function like:
///
///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///
///          fapi2::ReturnCode rc = fapiGetSpy( targ,
///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
///
/// @note The ID is not in quotes the fapi code will handle adding
///       the quotes for the cronus environment
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template <TargetType K, MulticastType M, typename V>
inline ReturnCode getSpy(const Target<K, M, V>& i_target, const spyId_t i_spyId,
			 variable_buffer& o_data)
{
	static_assert(K == 0, "implement getSpy (enum)");
	return ~FAPI2_RC_SUCCESS;
}

template <TargetType K>
inline ReturnCode getSpy(const Target<K>& i_target, const spyId_t i_spyId,
			 variable_buffer& o_data, const RingMode i_ringMode)
{
	static_assert(K == 0, "implement getSpy (enum)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template <TargetType K, MulticastType M, typename V>
inline ReturnCode getSpy(const Target<K, M, V>& i_target,
			 const char* const i_spyId, variable_buffer& o_data)
{
	static_assert(K == 0, "implement getSpy (string)");
	return ~FAPI2_RC_SUCCESS;
}

template <TargetType K>
inline ReturnCode getSpy(const Target<K>& i_target, const char* const i_spyId,
			 variable_buffer& o_data, const RingMode i_ringMode)
{
	static_assert(K == 0, "implement getSpy (string)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
/// @brief Writes a spy on a chip.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target   Target to operate on.
/// @param[in]  i_spyId    Id of the spy to write data to.
/// @param[out] i_data     Buffer that holds data to write into spy.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         1. if the spy name contains a # procedure writers should replace
///         is with an __P__ for example -
///
///         ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///         becomes
///         ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS
///
///         2. if the spy name has a number following a "." it must have an
///         underscore prepended to the number.
///
///         EH.TPCHIP.2KX100_ARY_CLK_EDGES_DLY
///         becomes
///         EH.TPCHIP._2KX100_ARY_CLK_EDGES_DLY
///
///         Example SPY name:
///         The hardware procedure should call the function like:
///
///          ABUS.RX0.RXPACKS#0.RXPACK.RD.LC.LC.ACT_DIS
///
///          fapi2::ReturnCode rc = fapiPutSpy( targ,
///                  ABUS.RX0.RXPACKS__P__0.RXPACK.RD.LC.LC.ACT_DIS, data );
///
/// @note The ID is not in quotes the fapi code will handle adding
///         the quotes for the cronus environment
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template <TargetType K, MulticastType M, typename V>
inline ReturnCode putSpy(const Target<K, M, V>& i_target, const spyId_t i_spyId,
			 const variable_buffer& i_data)
{
	static_assert(K == 0, "implement putSpy (enum)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template <TargetType K, MulticastType M, typename V>
inline ReturnCode putSpy(const Target<K, M, V>& i_target,
			 const char* const i_spyId,
			 const variable_buffer& i_data)
{
	static_assert(K == 0, "implement putSpy (string)");
	return ~FAPI2_RC_SUCCESS;
}

template <TargetType K, MulticastType M, typename V>
inline ReturnCode putSpy(const Target<K, M, V>& i_target,
			 const char* const i_spyId, const char* i_enum)
{
	static_assert(K == 0, "implement putSpy (string)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
/// @brief Writes spy data into a buffer holding ring data image
///        This API is used by L2/L3 repair to put column repair data
///        into a ring buffer image.
/// @tparam K template parameter, passed in target.
/// @param[in]  i_target      Target to operate on.
/// @param[in]  i_spyId       Id of the spy.
/// @param[in]  i_data        Buffer that holds spy data to write into ring
///                           image.
/// @param[out] o_imageData   Buffer that holds updated ring image.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         See fapiPutSpy for details on spy id specifics.
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template <TargetType K, MulticastType M, typename V>
inline ReturnCode
    putSpyImage(const Target<K, M, V>& i_target, const spyId_t i_spyId,
		const variable_buffer& i_data, variable_buffer& o_imageData)
{
	static_assert(K == 0, "implement putSpyImage (enum)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template <TargetType K, MulticastType M, typename V>
inline ReturnCode
    putSpyImage(const Target<K, M, V>& i_target, const char* const i_spyId,
		const variable_buffer& i_data, variable_buffer& o_imageData)
{
	static_assert(K == 0, "implement putSpyImage (string)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
/// @brief Reads spy data from a ring image buffer
/// @param[in]   i_target      Target to operate on
/// @param[in]   i_spyId       The spy's id
/// @param[out]  o_data        Buffer that holds data read from ring image.
/// @param[in]   i_imageData   Buffer that holds ring image to read data
///                            from.
/// @return fapi2::ReturnCode. FAPI2_RC_SUCCESS if success, else error code.
///
/// @note:  The string version is only supported for cronus.
///
///         The fapi design to support both FSP and cronus use of get and
///         put spy functions is dependent on the SPY names being expanded
///         to resemble a valid C identifier. This design places some
///         restrictions on the SPY names which can be used.
///
///         See fapiPutSpy for details on spy id specifics.
///
#ifdef FAPI_SUPPORT_SPY_AS_ENUM
template <TargetType K, MulticastType M, typename V>
inline ReturnCode getSpyImage(const Target<K, M, V>& i_target,
			      const spyId_t i_spyId, variable_buffer& o_data,
			      const variable_buffer& i_imageData)
{
	static_assert(K == 0, "implement getSpyImage (enum)");
	return ~FAPI2_RC_SUCCESS;
}
#endif
#ifdef FAPI_SUPPORT_SPY_AS_STRING
template <TargetType K, MulticastType M, typename V>
inline ReturnCode
    getSpyImage(const Target<K, M, V>& i_target, const char* const i_spyId,
		variable_buffer& o_data, const variable_buffer& i_imageData)
{
	static_assert(K == 0, "implement getSpyImage (string)");
	return ~FAPI2_RC_SUCCESS;
}
#endif

#endif // PPE

}; // namespace fapi2

#endif // _FAPI2_HWACCESS_H_
