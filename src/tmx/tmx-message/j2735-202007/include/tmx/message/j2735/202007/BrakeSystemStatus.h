/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_BrakeSystemStatus_H_
#define	_BrakeSystemStatus_H_


#include "asn_application.h"

/* Including external dependencies */
#include "BrakeAppliedStatus.h"
#include "TractionControlStatus.h"
#include "AntiLockBrakeStatus.h"
#include "StabilityControlStatus.h"
#include "BrakeBoostApplied.h"
#include "AuxiliaryBrakeStatus.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* BrakeSystemStatus */
typedef struct BrakeSystemStatus {
	BrakeAppliedStatus_t	 wheelBrakes;
	TractionControlStatus_t	 traction;
	AntiLockBrakeStatus_t	 abs;
	StabilityControlStatus_t	 scs;
	BrakeBoostApplied_t	 brakeBoost;
	AuxiliaryBrakeStatus_t	 auxBrakes;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} BrakeSystemStatus_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_BrakeSystemStatus;
extern asn_SEQUENCE_specifics_t asn_SPC_BrakeSystemStatus_specs_1;
extern asn_TYPE_member_t asn_MBR_BrakeSystemStatus_1[6];

#ifdef __cplusplus
}
#endif

#endif	/* _BrakeSystemStatus_H_ */
#include "asn_internal.h"
