/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "SPAT"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-SPAT.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_AdvisorySpeedList_H_
#define	_AdvisorySpeedList_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct AdvisorySpeed;

/* AdvisorySpeedList */
typedef struct AdvisorySpeedList {
	A_SEQUENCE_OF(struct AdvisorySpeed) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} AdvisorySpeedList_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_AdvisorySpeedList;
extern asn_SET_OF_specifics_t asn_SPC_AdvisorySpeedList_specs_1;
extern asn_TYPE_member_t asn_MBR_AdvisorySpeedList_1[1];
extern asn_per_constraints_t asn_PER_type_AdvisorySpeedList_constr_1;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "AdvisorySpeed.h"

#endif	/* _AdvisorySpeedList_H_ */
#include "asn_internal.h"
