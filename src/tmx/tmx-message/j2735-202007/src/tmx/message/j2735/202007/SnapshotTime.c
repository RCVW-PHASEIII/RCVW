/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "ProbeDataManagement"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-ProbeDataManagement.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "SnapshotTime.h"

asn_TYPE_member_t asn_MBR_SnapshotTime_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SnapshotTime, speed1),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GrossSpeed,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"speed1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SnapshotTime, time1),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SecondOfTime,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"time1"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SnapshotTime, speed2),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GrossSpeed,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"speed2"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct SnapshotTime, time2),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_SecondOfTime,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"time2"
		},
};
static const ber_tlv_tag_t asn_DEF_SnapshotTime_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SnapshotTime_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* speed1 */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* time1 */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* speed2 */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 } /* time2 */
};
asn_SEQUENCE_specifics_t asn_SPC_SnapshotTime_specs_1 = {
	sizeof(struct SnapshotTime),
	offsetof(struct SnapshotTime, _asn_ctx),
	asn_MAP_SnapshotTime_tag2el_1,
	4,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_SnapshotTime = {
	"SnapshotTime",
	"SnapshotTime",
	&asn_OP_SEQUENCE,
	asn_DEF_SnapshotTime_tags_1,
	sizeof(asn_DEF_SnapshotTime_tags_1)
		/sizeof(asn_DEF_SnapshotTime_tags_1[0]), /* 1 */
	asn_DEF_SnapshotTime_tags_1,	/* Same as above */
	sizeof(asn_DEF_SnapshotTime_tags_1)
		/sizeof(asn_DEF_SnapshotTime_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_SnapshotTime_1,
	4,	/* Elements count */
	&asn_SPC_SnapshotTime_specs_1	/* Additional specs */
};

