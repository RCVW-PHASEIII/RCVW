/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TravelerInformation"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TravelerInformation.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "NodeLL.h"

asn_TYPE_member_t asn_MBR_NodeLL_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct NodeLL, delta),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_NodeOffsetPointLL,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"delta"
		},
	{ ATF_POINTER, 1, offsetof(struct NodeLL, attributes),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NodeAttributeSetLL,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"attributes"
		},
};
static const int asn_MAP_NodeLL_oms_1[] = { 1 };
static const ber_tlv_tag_t asn_DEF_NodeLL_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_NodeLL_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* delta */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 } /* attributes */
};
asn_SEQUENCE_specifics_t asn_SPC_NodeLL_specs_1 = {
	sizeof(struct NodeLL),
	offsetof(struct NodeLL, _asn_ctx),
	asn_MAP_NodeLL_tag2el_1,
	2,	/* Count of tags in the map */
	asn_MAP_NodeLL_oms_1,	/* Optional members */
	1, 0,	/* Root/Additions */
	2,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_NodeLL = {
	"NodeLL",
	"NodeLL",
	&asn_OP_SEQUENCE,
	asn_DEF_NodeLL_tags_1,
	sizeof(asn_DEF_NodeLL_tags_1)
		/sizeof(asn_DEF_NodeLL_tags_1[0]), /* 1 */
	asn_DEF_NodeLL_tags_1,	/* Same as above */
	sizeof(asn_DEF_NodeLL_tags_1)
		/sizeof(asn_DEF_NodeLL_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_NodeLL_1,
	2,	/* Elements count */
	&asn_SPC_NodeLL_specs_1	/* Additional specs */
};

