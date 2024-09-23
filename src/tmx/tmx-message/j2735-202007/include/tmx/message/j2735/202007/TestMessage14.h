/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TestMessage14"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TestMessage14.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_TestMessage14_H_
#define	_TestMessage14_H_


#include "asn_application.h"

/* Including external dependencies */
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct Header;
struct RegionalExtension;

/* TestMessage14 */
typedef struct TestMessage14 {
	struct Header	*header	/* OPTIONAL */;
	struct RegionalExtension	*regional	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TestMessage14_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TestMessage14;
extern asn_SEQUENCE_specifics_t asn_SPC_TestMessage14_specs_1;
extern asn_TYPE_member_t asn_MBR_TestMessage14_1[2];

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "Header.h"
#include "RegionalExtension.h"

#endif	/* _TestMessage14_H_ */
#include "asn_internal.h"
