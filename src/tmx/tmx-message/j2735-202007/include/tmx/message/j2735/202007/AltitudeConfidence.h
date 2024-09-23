/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "AddGrpC"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-AddGrpC.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_AltitudeConfidence_H_
#define	_AltitudeConfidence_H_


#include "asn_application.h"

/* Including external dependencies */
#include "NativeEnumerated.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum AltitudeConfidence {
	AltitudeConfidence_alt_000_01	= 0,
	AltitudeConfidence_alt_000_02	= 1,
	AltitudeConfidence_alt_000_05	= 2,
	AltitudeConfidence_alt_000_10	= 3,
	AltitudeConfidence_alt_000_20	= 4,
	AltitudeConfidence_alt_000_50	= 5,
	AltitudeConfidence_alt_001_00	= 6,
	AltitudeConfidence_alt_002_00	= 7,
	AltitudeConfidence_alt_005_00	= 8,
	AltitudeConfidence_alt_010_00	= 9,
	AltitudeConfidence_alt_020_00	= 10,
	AltitudeConfidence_alt_050_00	= 11,
	AltitudeConfidence_alt_100_00	= 12,
	AltitudeConfidence_alt_200_00	= 13,
	AltitudeConfidence_outOfRange	= 14,
	AltitudeConfidence_unavailable	= 15
} e_AltitudeConfidence;

/* AltitudeConfidence */
typedef long	 AltitudeConfidence_t;

/* Implementation */
extern asn_per_constraints_t asn_PER_type_AltitudeConfidence_constr_1;
extern asn_TYPE_descriptor_t asn_DEF_AltitudeConfidence;
extern const asn_INTEGER_specifics_t asn_SPC_AltitudeConfidence_specs_1;
asn_struct_free_f AltitudeConfidence_free;
asn_struct_print_f AltitudeConfidence_print;
asn_constr_check_f AltitudeConfidence_constraint;
ber_type_decoder_f AltitudeConfidence_decode_ber;
der_type_encoder_f AltitudeConfidence_encode_der;
xer_type_decoder_f AltitudeConfidence_decode_xer;
xer_type_encoder_f AltitudeConfidence_encode_xer;
oer_type_decoder_f AltitudeConfidence_decode_oer;
oer_type_encoder_f AltitudeConfidence_encode_oer;
per_type_decoder_f AltitudeConfidence_decode_uper;
per_type_encoder_f AltitudeConfidence_encode_uper;

#ifdef __cplusplus
}
#endif

#endif	/* _AltitudeConfidence_H_ */
#include "asn_internal.h"
