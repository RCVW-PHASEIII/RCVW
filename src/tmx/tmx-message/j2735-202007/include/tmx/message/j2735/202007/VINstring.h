/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeVehicleData"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-ProbeVehicleData.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_VINstring_H_
#define	_VINstring_H_


#include "asn_application.h"

/* Including external dependencies */
#include "OCTET_STRING.h"

#ifdef __cplusplus
extern "C" {
#endif

/* VINstring */
typedef OCTET_STRING_t	 VINstring_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_VINstring_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_VINstring;
asn_struct_free_f VINstring_free;
asn_struct_print_f VINstring_print;
asn_constr_check_f VINstring_constraint;
ber_type_decoder_f VINstring_decode_ber;
der_type_encoder_f VINstring_encode_der;
xer_type_decoder_f VINstring_decode_xer;
xer_type_encoder_f VINstring_encode_xer;
oer_type_decoder_f VINstring_decode_oer;
oer_type_encoder_f VINstring_encode_oer;
per_type_decoder_f VINstring_decode_uper;
per_type_encoder_f VINstring_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _VINstring_H_ */
#include "asn_internal.h"
