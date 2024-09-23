/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TravelerInformation"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TravelerInformation.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "Circle.h"

asn_TYPE_member_t asn_MBR_Circle_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct Circle, center),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Position3D,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"center"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Circle, radius),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Radius_B12,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"radius"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct Circle, units),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DistanceUnits,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"units"
		},
};
static const ber_tlv_tag_t asn_DEF_Circle_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_Circle_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* center */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* radius */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* units */
};
asn_SEQUENCE_specifics_t asn_SPC_Circle_specs_1 = {
	sizeof(struct Circle),
	offsetof(struct Circle, _asn_ctx),
	asn_MAP_Circle_tag2el_1,
	3,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_Circle = {
	"Circle",
	"Circle",
	&asn_OP_SEQUENCE,
	asn_DEF_Circle_tags_1,
	sizeof(asn_DEF_Circle_tags_1)
		/sizeof(asn_DEF_Circle_tags_1[0]), /* 1 */
	asn_DEF_Circle_tags_1,	/* Same as above */
	sizeof(asn_DEF_Circle_tags_1)
		/sizeof(asn_DEF_Circle_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_Circle_1,
	3,	/* Elements count */
	&asn_SPC_Circle_specs_1	/* Additional specs */
};

