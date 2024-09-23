/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "StabilityControlStatus.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_StabilityControlStatus_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_StabilityControlStatus_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 2,  2,  0,  3 }	/* (0..3) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_StabilityControlStatus_value2enum_1[] = {
	{ 0,	11,	"unavailable" },
	{ 1,	3,	"off" },
	{ 2,	2,	"on" },
	{ 3,	7,	"engaged" }
};
static const unsigned int asn_MAP_StabilityControlStatus_enum2value_1[] = {
	3,	/* engaged(3) */
	1,	/* off(1) */
	2,	/* on(2) */
	0	/* unavailable(0) */
};
const asn_INTEGER_specifics_t asn_SPC_StabilityControlStatus_specs_1 = {
	asn_MAP_StabilityControlStatus_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_StabilityControlStatus_enum2value_1,	/* N => "tag"; sorted by N */
	4,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_StabilityControlStatus_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_StabilityControlStatus = {
	"StabilityControlStatus",
	"StabilityControlStatus",
	&asn_OP_NativeEnumerated,
	asn_DEF_StabilityControlStatus_tags_1,
	sizeof(asn_DEF_StabilityControlStatus_tags_1)
		/sizeof(asn_DEF_StabilityControlStatus_tags_1[0]), /* 1 */
	asn_DEF_StabilityControlStatus_tags_1,	/* Same as above */
	sizeof(asn_DEF_StabilityControlStatus_tags_1)
		/sizeof(asn_DEF_StabilityControlStatus_tags_1[0]), /* 1 */
	{ &asn_OER_type_StabilityControlStatus_constr_1, &asn_PER_type_StabilityControlStatus_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_StabilityControlStatus_specs_1	/* Additional specs */
};

