/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeVehicleData"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-ProbeVehicleData.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "BrakeAppliedPressure.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_BrakeAppliedPressure_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_BrakeAppliedPressure_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED,	 4,  4,  0,  15 }	/* (0..15) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_BrakeAppliedPressure_value2enum_1[] = {
	{ 0,	11,	"unavailable" },
	{ 1,	11,	"minPressure" },
	{ 2,	7,	"bkLvl-2" },
	{ 3,	7,	"bkLvl-3" },
	{ 4,	7,	"bkLvl-4" },
	{ 5,	7,	"bkLvl-5" },
	{ 6,	7,	"bkLvl-6" },
	{ 7,	7,	"bkLvl-7" },
	{ 8,	7,	"bkLvl-8" },
	{ 9,	7,	"bkLvl-9" },
	{ 10,	8,	"bkLvl-10" },
	{ 11,	8,	"bkLvl-11" },
	{ 12,	8,	"bkLvl-12" },
	{ 13,	8,	"bkLvl-13" },
	{ 14,	8,	"bkLvl-14" },
	{ 15,	11,	"maxPressure" }
};
static const unsigned int asn_MAP_BrakeAppliedPressure_enum2value_1[] = {
	10,	/* bkLvl-10(10) */
	11,	/* bkLvl-11(11) */
	12,	/* bkLvl-12(12) */
	13,	/* bkLvl-13(13) */
	14,	/* bkLvl-14(14) */
	2,	/* bkLvl-2(2) */
	3,	/* bkLvl-3(3) */
	4,	/* bkLvl-4(4) */
	5,	/* bkLvl-5(5) */
	6,	/* bkLvl-6(6) */
	7,	/* bkLvl-7(7) */
	8,	/* bkLvl-8(8) */
	9,	/* bkLvl-9(9) */
	15,	/* maxPressure(15) */
	1,	/* minPressure(1) */
	0	/* unavailable(0) */
};
const asn_INTEGER_specifics_t asn_SPC_BrakeAppliedPressure_specs_1 = {
	asn_MAP_BrakeAppliedPressure_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_BrakeAppliedPressure_enum2value_1,	/* N => "tag"; sorted by N */
	16,	/* Number of elements in the maps */
	0,	/* Enumeration is not extensible */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_BrakeAppliedPressure_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_BrakeAppliedPressure = {
	"BrakeAppliedPressure",
	"BrakeAppliedPressure",
	&asn_OP_NativeEnumerated,
	asn_DEF_BrakeAppliedPressure_tags_1,
	sizeof(asn_DEF_BrakeAppliedPressure_tags_1)
		/sizeof(asn_DEF_BrakeAppliedPressure_tags_1[0]), /* 1 */
	asn_DEF_BrakeAppliedPressure_tags_1,	/* Same as above */
	sizeof(asn_DEF_BrakeAppliedPressure_tags_1)
		/sizeof(asn_DEF_BrakeAppliedPressure_tags_1[0]), /* 1 */
	{ &asn_OER_type_BrakeAppliedPressure_constr_1, &asn_PER_type_BrakeAppliedPressure_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_BrakeAppliedPressure_specs_1	/* Additional specs */
};

