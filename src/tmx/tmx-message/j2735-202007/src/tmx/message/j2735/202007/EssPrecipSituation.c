/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "NTCIP"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-NTCIP.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "EssPrecipSituation.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_EssPrecipSituation_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_EssPrecipSituation_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 4,  4,  0,  14 }	/* (0..14) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_EssPrecipSituation_value2enum_1[] = {
	{ 1,	5,	"other" },
	{ 2,	7,	"unknown" },
	{ 3,	15,	"noPrecipitation" },
	{ 4,	18,	"unidentifiedSlight" },
	{ 5,	20,	"unidentifiedModerate" },
	{ 6,	17,	"unidentifiedHeavy" },
	{ 7,	10,	"snowSlight" },
	{ 8,	12,	"snowModerate" },
	{ 9,	9,	"snowHeavy" },
	{ 10,	10,	"rainSlight" },
	{ 11,	12,	"rainModerate" },
	{ 12,	9,	"rainHeavy" },
	{ 13,	25,	"frozenPrecipitationSlight" },
	{ 14,	27,	"frozenPrecipitationModerate" },
	{ 15,	24,	"frozenPrecipitationHeavy" }
};
static const unsigned int asn_MAP_EssPrecipSituation_enum2value_1[] = {
	14,	/* frozenPrecipitationHeavy(15) */
	13,	/* frozenPrecipitationModerate(14) */
	12,	/* frozenPrecipitationSlight(13) */
	2,	/* noPrecipitation(3) */
	0,	/* other(1) */
	11,	/* rainHeavy(12) */
	10,	/* rainModerate(11) */
	9,	/* rainSlight(10) */
	8,	/* snowHeavy(9) */
	7,	/* snowModerate(8) */
	6,	/* snowSlight(7) */
	5,	/* unidentifiedHeavy(6) */
	4,	/* unidentifiedModerate(5) */
	3,	/* unidentifiedSlight(4) */
	1	/* unknown(2) */
};
const asn_INTEGER_specifics_t asn_SPC_EssPrecipSituation_specs_1 = {
	asn_MAP_EssPrecipSituation_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_EssPrecipSituation_enum2value_1,	/* N => "tag"; sorted by N */
	15,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_EssPrecipSituation_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_EssPrecipSituation = {
	"EssPrecipSituation",
	"EssPrecipSituation",
	&asn_OP_NativeEnumerated,
	asn_DEF_EssPrecipSituation_tags_1,
	sizeof(asn_DEF_EssPrecipSituation_tags_1)
		/sizeof(asn_DEF_EssPrecipSituation_tags_1[0]), /* 1 */
	asn_DEF_EssPrecipSituation_tags_1,	/* Same as above */
	sizeof(asn_DEF_EssPrecipSituation_tags_1)
		/sizeof(asn_DEF_EssPrecipSituation_tags_1[0]), /* 1 */
	{ &asn_OER_type_EssPrecipSituation_constr_1, &asn_PER_type_EssPrecipSituation_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_EssPrecipSituation_specs_1	/* Additional specs */
};

