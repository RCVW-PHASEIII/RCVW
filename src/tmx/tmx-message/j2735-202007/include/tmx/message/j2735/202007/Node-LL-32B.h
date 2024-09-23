/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TravelerInformation"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TravelerInformation.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_Node_LL_32B_H_
#define	_Node_LL_32B_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OffsetLL-B16.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Node-LL-32B */
typedef struct Node_LL_32B {
	OffsetLL_B16_t	 lon;
	OffsetLL_B16_t	 lat;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} Node_LL_32B_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_Node_LL_32B;
extern asn_SEQUENCE_specifics_t asn_SPC_Node_LL_32B_specs_1;
extern asn_TYPE_member_t asn_MBR_Node_LL_32B_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _Node_LL_32B_H_ */
#include "asn_internal.h"
