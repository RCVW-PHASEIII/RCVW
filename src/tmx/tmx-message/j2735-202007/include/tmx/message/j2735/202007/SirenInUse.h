/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_SirenInUse_H_
#define	_SirenInUse_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SirenInUse {
	SirenInUse_unavailable	= 0,
	SirenInUse_notInUse	= 1,
	SirenInUse_inUse	= 2,
	SirenInUse_reserved	= 3
} e_SirenInUse;

/* SirenInUse */
typedef long	 SirenInUse_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_SirenInUse_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_SirenInUse;
extern const asn_INTEGER_specifics_t asn_SPC_SirenInUse_specs_1;
asn_struct_free_f SirenInUse_free;
asn_struct_print_f SirenInUse_print;
asn_constr_check_f SirenInUse_constraint;
ber_type_decoder_f SirenInUse_decode_ber;
der_type_encoder_f SirenInUse_encode_der;
xer_type_decoder_f SirenInUse_decode_xer;
xer_type_encoder_f SirenInUse_encode_xer;
oer_type_decoder_f SirenInUse_decode_oer;
oer_type_encoder_f SirenInUse_encode_oer;
per_type_decoder_f SirenInUse_decode_uper;
per_type_encoder_f SirenInUse_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _SirenInUse_H_ */
#include "asn_internal.h"
