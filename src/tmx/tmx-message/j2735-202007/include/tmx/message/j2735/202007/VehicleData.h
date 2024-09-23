/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BasicSafetyMessage"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-BasicSafetyMessage.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_VehicleData_H_
#define	_VehicleData_H_


#include "asn_application.h"

/* Including external dependencies */
#include "VehicleHeight.h"
#include "VehicleMass.h"
#include "TrailerWeight.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct BumperHeights;

/* VehicleData */
typedef struct VehicleData {
	VehicleHeight_t	*height	/* OPTIONAL */;
	struct BumperHeights	*bumpers	/* OPTIONAL */;
	VehicleMass_t	*mass	/* OPTIONAL */;
	TrailerWeight_t	*trailerWeight	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} VehicleData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_VehicleData;
extern asn_SEQUENCE_specifics_t asn_SPC_VehicleData_specs_1;
extern asn_TYPE_member_t asn_MBR_VehicleData_1[4];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "BumperHeights.h"

#endif	/* _VehicleData_H_ */
#include "asn_internal.h"
