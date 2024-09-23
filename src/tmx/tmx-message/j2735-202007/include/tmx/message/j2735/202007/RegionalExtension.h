/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_RegionalExtension_H_
#define	_RegionalExtension_H_


#include "asn_application.h"

/* Including external dependencies */
#include "RegionId.h"
#include "ANY.h"
#include "asn_ioc.h"
#include "OPEN_TYPE.h"
#include "constr_CHOICE.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum RegionalExtension_120P0__regExtValue_PR {
	RegionalExtension_120P0__regExtValue_PR_NOTHING,	/* No components present */
	
} RegionalExtension_120P0__regExtValue_PR;

/* RegionalExtension */
typedef struct RegionalExtension_120P0 {
	RegionId_t	 regionId;
	struct RegionalExtension_120P0__regExtValue {
		RegionalExtension_120P0__regExtValue_PR present;
		union RegionalExtension_120P0__regExtValue_u {
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} regExtValue;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} RegionalExtension_120P0_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_RegionalExtension_120P0;
extern asn_SEQUENCE_specifics_t asn_SPC_RegionalExtension_120P0_specs_1;
extern asn_TYPE_member_t asn_MBR_RegionalExtension_120P0_1[2];

#ifdef __cplusplus
}
#endif

#endif	/* _RegionalExtension_H_ */
#include "asn_internal.h"
