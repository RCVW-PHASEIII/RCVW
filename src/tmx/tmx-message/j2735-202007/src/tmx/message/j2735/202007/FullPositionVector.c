/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "Common"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-Common.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "FullPositionVector.h"

asn_TYPE_member_t asn_MBR_FullPositionVector_1[] = {
	{ ATF_POINTER, 1, offsetof(struct FullPositionVector, utcTime),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DDateTime,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"utcTime"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FullPositionVector, Long),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Longitude,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"long"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct FullPositionVector, lat),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Latitude,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"lat"
		},
	{ ATF_POINTER, 7, offsetof(struct FullPositionVector, elevation),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Elevation,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"elevation"
		},
	{ ATF_POINTER, 6, offsetof(struct FullPositionVector, heading),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Heading,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"heading"
		},
	{ ATF_POINTER, 5, offsetof(struct FullPositionVector, speed),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TransmissionAndSpeed,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"speed"
		},
	{ ATF_POINTER, 4, offsetof(struct FullPositionVector, posAccuracy),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PositionalAccuracy,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"posAccuracy"
		},
	{ ATF_POINTER, 3, offsetof(struct FullPositionVector, timeConfidence),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_TimeConfidence,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"timeConfidence"
		},
	{ ATF_POINTER, 2, offsetof(struct FullPositionVector, posConfidence),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_PositionConfidenceSet,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"posConfidence"
		},
	{ ATF_POINTER, 1, offsetof(struct FullPositionVector, speedConfidence),
		(ASN_TAG_CLASS_CONTEXT | (9 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SpeedandHeadingandThrottleConfidence,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"speedConfidence"
		},
};
static const int asn_MAP_FullPositionVector_oms_1[] = { 0, 3, 4, 5, 6, 7, 8, 9 };
static const ber_tlv_tag_t asn_DEF_FullPositionVector_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_FullPositionVector_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* utcTime */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* long */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* lat */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* elevation */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* heading */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* speed */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* posAccuracy */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* timeConfidence */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 }, /* posConfidence */
    { (ASN_TAG_CLASS_CONTEXT | (9 << 2)), 9, 0, 0 } /* speedConfidence */
};
asn_SEQUENCE_specifics_t asn_SPC_FullPositionVector_specs_1 = {
	sizeof(struct FullPositionVector),
	offsetof(struct FullPositionVector, _asn_ctx),
	asn_MAP_FullPositionVector_tag2el_1,
	10,	/* Count of tags in the map */
	asn_MAP_FullPositionVector_oms_1,	/* Optional members */
	8, 0,	/* Root/Additions */
	10,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_FullPositionVector = {
	"FullPositionVector",
	"FullPositionVector",
	&asn_OP_SEQUENCE,
	asn_DEF_FullPositionVector_tags_1,
	sizeof(asn_DEF_FullPositionVector_tags_1)
		/sizeof(asn_DEF_FullPositionVector_tags_1[0]), /* 1 */
	asn_DEF_FullPositionVector_tags_1,	/* Same as above */
	sizeof(asn_DEF_FullPositionVector_tags_1)
		/sizeof(asn_DEF_FullPositionVector_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_FullPositionVector_1,
	10,	/* Elements count */
	&asn_SPC_FullPositionVector_specs_1	/* Additional specs */
};

