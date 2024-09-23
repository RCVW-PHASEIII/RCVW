/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BasicSafetyMessage"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-BasicSafetyMessage.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_SpecialVehicleExtensions_H_
#define	_SpecialVehicleExtensions_H_


#include "asn_application.h"

/* Including external dependencies */
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct EmergencyDetails;
struct EventDescription;
struct TrailerData;

/* SpecialVehicleExtensions */
typedef struct SpecialVehicleExtensions {
	struct EmergencyDetails	*vehicleAlerts	/* OPTIONAL */;
	struct EventDescription	*description	/* OPTIONAL */;
	struct TrailerData	*trailers	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} SpecialVehicleExtensions_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SpecialVehicleExtensions;
extern asn_SEQUENCE_specifics_t asn_SPC_SpecialVehicleExtensions_specs_1;
extern asn_TYPE_member_t asn_MBR_SpecialVehicleExtensions_1[3];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "EmergencyDetails.h"
#include "EventDescription.h"
#include "TrailerData.h"

#endif	/* _SpecialVehicleExtensions_H_ */
#include "asn_internal.h"
