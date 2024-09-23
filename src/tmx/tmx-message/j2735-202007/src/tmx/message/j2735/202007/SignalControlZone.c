/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "MapData"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-MapData.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "SignalControlZone.h"

asn_TYPE_member_t asn_MBR_SignalControlZone_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct SignalControlZone, zone),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RegionalExtension_120P0,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"zone"
		},
};
static const ber_tlv_tag_t asn_DEF_SignalControlZone_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_SignalControlZone_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* zone */
};
asn_SEQUENCE_specifics_t asn_SPC_SignalControlZone_specs_1 = {
	sizeof(struct SignalControlZone),
	offsetof(struct SignalControlZone, _asn_ctx),
	asn_MAP_SignalControlZone_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	1,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_SignalControlZone = {
	"SignalControlZone",
	"SignalControlZone",
	&asn_OP_SEQUENCE,
	asn_DEF_SignalControlZone_tags_1,
	sizeof(asn_DEF_SignalControlZone_tags_1)
		/sizeof(asn_DEF_SignalControlZone_tags_1[0]), /* 1 */
	asn_DEF_SignalControlZone_tags_1,	/* Same as above */
	sizeof(asn_DEF_SignalControlZone_tags_1)
		/sizeof(asn_DEF_SignalControlZone_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_SignalControlZone_1,
	1,	/* Elements count */
	&asn_SPC_SignalControlZone_specs_1	/* Additional specs */
};

