/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TravelerInformation"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TravelerInformation.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_WorkZone_H_
#define	_WorkZone_H_


#include "asn_application.h"

/* Including external dependencies */
#include "asn_SEQUENCE_OF.h"
#include "ITIScodes.h"
#include "ITIStextPhrase.h"
#include "constr_CHOICE.h"
#include "constr_SEQUENCE.h"
#include "constr_SEQUENCE_OF.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum WorkZone__Member__item_PR {
	WorkZone__Member__item_PR_NOTHING,	/* No components present */
	WorkZone__Member__item_PR_itis,
	WorkZone__Member__item_PR_text
} WorkZone__Member__item_PR;

/* Forward definitions */
typedef struct WorkZone__Member {
	struct WorkZone__Member__item {
		WorkZone__Member__item_PR present;
		union WorkZone__Member__item_u {
			ITIScodes_t	 itis;
			ITIStextPhrase_t	 text;
		} choice;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} item;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} WorkZone__Member;

/* WorkZone */
typedef struct WorkZone {
	A_SEQUENCE_OF(WorkZone__Member) list;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} WorkZone_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_WorkZone;
extern asn_SET_OF_specifics_t asn_SPC_WorkZone_specs_1;
extern asn_TYPE_member_t asn_MBR_WorkZone_1[1];
extern asn_per_constraints_t asn_PER_type_WorkZone_constr_1;

#ifdef __cplusplus
}
#endif

#endif	/* _WorkZone_H_ */
#include "asn_internal.h"
