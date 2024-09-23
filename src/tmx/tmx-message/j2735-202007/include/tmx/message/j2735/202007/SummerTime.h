/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "AddGrpB"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-AddGrpB.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_SummerTime_H_
#define	_SummerTime_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum SummerTime {
	SummerTime_notInSummerTime	= 0,
	SummerTime_inSummerTime	= 1
} e_SummerTime;

/* SummerTime */
typedef long	 SummerTime_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_SummerTime;
asn_struct_free_f SummerTime_free;
asn_struct_print_f SummerTime_print;
asn_constr_check_f SummerTime_constraint;
ber_type_decoder_f SummerTime_decode_ber;
der_type_encoder_f SummerTime_encode_der;
xer_type_decoder_f SummerTime_decode_xer;
xer_type_encoder_f SummerTime_encode_xer;
oer_type_decoder_f SummerTime_decode_oer;
oer_type_encoder_f SummerTime_encode_oer;
per_type_decoder_f SummerTime_decode_uper;
per_type_encoder_f SummerTime_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _SummerTime_H_ */
#include "asn_internal.h"
