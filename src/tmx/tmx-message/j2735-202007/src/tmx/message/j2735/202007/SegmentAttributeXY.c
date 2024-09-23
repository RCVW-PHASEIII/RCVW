/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "SegmentAttributeXY.h"

/*
 * This type is implemented using NativeEnumerated,
 * so here we adjust the DEF accordingly.
 */
static asn_oer_constraints_t asn_OER_type_SegmentAttributeXY_constr_1 CC_NOTUSED = {
	{ 0, 0 },
	-1};
asn_per_constraints_t asn_PER_type_SegmentAttributeXY_constr_1 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  6,  6,  0,  37 }	/* (0..37,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static const asn_INTEGER_enum_map_t asn_MAP_SegmentAttributeXY_value2enum_1[] = {
	{ 0,	8,	"reserved" },
	{ 1,	10,	"doNotBlock" },
	{ 2,	9,	"whiteLine" },
	{ 3,	15,	"mergingLaneLeft" },
	{ 4,	16,	"mergingLaneRight" },
	{ 5,	10,	"curbOnLeft" },
	{ 6,	11,	"curbOnRight" },
	{ 7,	17,	"loadingzoneOnLeft" },
	{ 8,	18,	"loadingzoneOnRight" },
	{ 9,	18,	"turnOutPointOnLeft" },
	{ 10,	19,	"turnOutPointOnRight" },
	{ 11,	21,	"adjacentParkingOnLeft" },
	{ 12,	22,	"adjacentParkingOnRight" },
	{ 13,	22,	"adjacentBikeLaneOnLeft" },
	{ 14,	23,	"adjacentBikeLaneOnRight" },
	{ 15,	14,	"sharedBikeLane" },
	{ 16,	14,	"bikeBoxInFront" },
	{ 17,	17,	"transitStopOnLeft" },
	{ 18,	18,	"transitStopOnRight" },
	{ 19,	17,	"transitStopInLane" },
	{ 20,	24,	"sharedWithTrackedVehicle" },
	{ 21,	10,	"safeIsland" },
	{ 22,	15,	"lowCurbsPresent" },
	{ 23,	18,	"rumbleStripPresent" },
	{ 24,	23,	"audibleSignalingPresent" },
	{ 25,	21,	"adaptiveTimingPresent" },
	{ 26,	22,	"rfSignalRequestPresent" },
	{ 27,	20,	"partialCurbIntrusion" },
	{ 28,	11,	"taperToLeft" },
	{ 29,	12,	"taperToRight" },
	{ 30,	17,	"taperToCenterLine" },
	{ 31,	15,	"parallelParking" },
	{ 32,	13,	"headInParking" },
	{ 33,	11,	"freeParking" },
	{ 34,	25,	"timeRestrictionsOnParking" },
	{ 35,	10,	"costToPark" },
	{ 36,	19,	"midBlockCurbPresent" },
	{ 37,	21,	"unEvenPavementPresent" }
	/* This list is extensible */
};
static const unsigned int asn_MAP_SegmentAttributeXY_enum2value_1[] = {
	25,	/* adaptiveTimingPresent(25) */
	13,	/* adjacentBikeLaneOnLeft(13) */
	14,	/* adjacentBikeLaneOnRight(14) */
	11,	/* adjacentParkingOnLeft(11) */
	12,	/* adjacentParkingOnRight(12) */
	24,	/* audibleSignalingPresent(24) */
	16,	/* bikeBoxInFront(16) */
	35,	/* costToPark(35) */
	5,	/* curbOnLeft(5) */
	6,	/* curbOnRight(6) */
	1,	/* doNotBlock(1) */
	33,	/* freeParking(33) */
	32,	/* headInParking(32) */
	7,	/* loadingzoneOnLeft(7) */
	8,	/* loadingzoneOnRight(8) */
	22,	/* lowCurbsPresent(22) */
	3,	/* mergingLaneLeft(3) */
	4,	/* mergingLaneRight(4) */
	36,	/* midBlockCurbPresent(36) */
	31,	/* parallelParking(31) */
	27,	/* partialCurbIntrusion(27) */
	0,	/* reserved(0) */
	26,	/* rfSignalRequestPresent(26) */
	23,	/* rumbleStripPresent(23) */
	21,	/* safeIsland(21) */
	15,	/* sharedBikeLane(15) */
	20,	/* sharedWithTrackedVehicle(20) */
	30,	/* taperToCenterLine(30) */
	28,	/* taperToLeft(28) */
	29,	/* taperToRight(29) */
	34,	/* timeRestrictionsOnParking(34) */
	19,	/* transitStopInLane(19) */
	17,	/* transitStopOnLeft(17) */
	18,	/* transitStopOnRight(18) */
	9,	/* turnOutPointOnLeft(9) */
	10,	/* turnOutPointOnRight(10) */
	37,	/* unEvenPavementPresent(37) */
	2	/* whiteLine(2) */
	/* This list is extensible */
};
const asn_INTEGER_specifics_t asn_SPC_SegmentAttributeXY_specs_1 = {
	asn_MAP_SegmentAttributeXY_value2enum_1,	/* "tag" => N; sorted by tag */
	asn_MAP_SegmentAttributeXY_enum2value_1,	/* N => "tag"; sorted by N */
	38,	/* Number of elements in the maps */
	39,	/* Extensions before this member */
	1,	/* Strict enumeration */
	0,	/* Native long size */
	0
};
static const ber_tlv_tag_t asn_DEF_SegmentAttributeXY_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (10 << 2))
};
asn_TYPE_descriptor_t asn_DEF_SegmentAttributeXY = {
	"SegmentAttributeXY",
	"SegmentAttributeXY",
	&asn_OP_NativeEnumerated,
	asn_DEF_SegmentAttributeXY_tags_1,
	sizeof(asn_DEF_SegmentAttributeXY_tags_1)
		/sizeof(asn_DEF_SegmentAttributeXY_tags_1[0]), /* 1 */
	asn_DEF_SegmentAttributeXY_tags_1,	/* Same as above */
	sizeof(asn_DEF_SegmentAttributeXY_tags_1)
		/sizeof(asn_DEF_SegmentAttributeXY_tags_1[0]), /* 1 */
	{ &asn_OER_type_SegmentAttributeXY_constr_1, &asn_PER_type_SegmentAttributeXY_constr_1, NativeEnumerated_constraint },
	0, 0,	/* Defined elsewhere */
	&asn_SPC_SegmentAttributeXY_specs_1	/* Additional specs */
};

