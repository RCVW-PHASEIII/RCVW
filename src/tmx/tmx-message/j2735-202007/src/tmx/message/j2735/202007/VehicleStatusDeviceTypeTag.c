/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataManagement"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-ProbeDataManagement.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "VehicleStatusDeviceTypeTag.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_VehicleStatusDeviceTypeTag_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_VehicleStatusDeviceTypeTag_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  5,  5,  0,  28 }	/* (0..28,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_VehicleStatusDeviceTypeTag_value2enum_1[] = {
	{ 0,	7,	"unknown" },
	{ 1,	6,	"lights" },
	{ 2,	6,	"wipers" },
	{ 3,	6,	"brakes" },
	{ 4,	4,	"stab" },
	{ 5,	4,	"trac" },
	{ 6,	3,	"abs" },
	{ 7,	4,	"sunS" },
	{ 8,	5,	"rainS" },
	{ 9,	7,	"airTemp" },
	{ 10,	8,	"steering" },
	{ 11,	14,	"vertAccelThres" },
	{ 12,	9,	"vertAccel" },
	{ 13,	12,	"hozAccelLong" },
	{ 14,	11,	"hozAccelLat" },
	{ 15,	11,	"hozAccelCon" },
	{ 16,	9,	"accel4way" },
	{ 17,	13,	"confidenceSet" },
	{ 18,	6,	"obDist" },
	{ 19,	8,	"obDirect" },
	{ 20,	3,	"yaw" },
	{ 21,	10,	"yawRateCon" },
	{ 22,	8,	"dateTime" },
	{ 23,	7,	"fullPos" },
	{ 24,	10,	"position2D" },
	{ 25,	10,	"position3D" },
	{ 26,	7,	"vehicle" },
	{ 27,	10,	"speedHeadC" },
	{ 28,	6,	"speedC" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_VehicleStatusDeviceTypeTag_enum2value_1[] = {
	6,	/* abs(6) */
	16,	/* accel4way(16) */
	9,	/* airTemp(9) */
	3,	/* brakes(3) */
	17,	/* confidenceSet(17) */
	22,	/* dateTime(22) */
	23,	/* fullPos(23) */
	15,	/* hozAccelCon(15) */
	14,	/* hozAccelLat(14) */
	13,	/* hozAccelLong(13) */
	1,	/* lights(1) */
	19,	/* obDirect(19) */
	18,	/* obDist(18) */
	24,	/* position2D(24) */
	25,	/* position3D(25) */
	8,	/* rainS(8) */
	28,	/* speedC(28) */
	27,	/* speedHeadC(27) */
	4,	/* stab(4) */
	10,	/* steering(10) */
	7,	/* sunS(7) */
	5,	/* trac(5) */
	0,	/* unknown(0) */
	26,	/* vehicle(26) */
	12,	/* vertAccel(12) */
	11,	/* vertAccelThres(11) */
	2,	/* wipers(2) */
	20,	/* yaw(20) */
	21	/* yawRateCon(21) */
	/* This list is extensible */
};
const asn_INTEGER_specifics_t asn_SPC_VehicleStatusDeviceTypeTag_specs_1 = {
	asn_MAP_VehicleStatusDeviceTypeTag_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_VehicleStatusDeviceTypeTag_enum2value_1,	/* N => "tag"; sorted by N */
	29,	/* Number of elements in the maps */
	30,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_VehicleStatusDeviceTypeTag_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_VehicleStatusDeviceTypeTag = {
	"VehicleStatusDeviceTypeTag",
	"VehicleStatusDeviceTypeTag",
	&asn_OP_NativeEnumerated,
	asn_DEF_VehicleStatusDeviceTypeTag_tags_1,
	sizeof(asn_DEF_VehicleStatusDeviceTypeTag_tags_1)
		/sizeof(asn_DEF_VehicleStatusDeviceTypeTag_tags_1[0]), /* 1 */
	asn_DEF_VehicleStatusDeviceTypeTag_tags_1,	/* Same as above */
	sizeof(asn_DEF_VehicleStatusDeviceTypeTag_tags_1)
		/sizeof(asn_DEF_VehicleStatusDeviceTypeTag_tags_1[0]), /* 1 */
	{ &asn_OER_type_VehicleStatusDeviceTypeTag_constr_1, &asn_PER_type_VehicleStatusDeviceTypeTag_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_VehicleStatusDeviceTypeTag_specs_1	/* Additional specs */
};

