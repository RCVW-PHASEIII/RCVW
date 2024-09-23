/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "TravelerInformation"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-TravelerInformation.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#include "GeographicalPath.h"

static int
memb_regional_constraint_1(const asn_TYPE_descriptor_t *td, const void *sptr,
			asn_app_constraint_failed_f *ctfailcb, void *app_key) {
	size_t size;
	
	if(!sptr) {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: value not given (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
	
	/* Determine the number of elements */
	size = _A_CSEQUENCE_FROM_VOID(sptr)->count;
	
	if((size >= 1 && size <= 4)) {
		/* Perform validation of the inner elements */
		return SEQUENCE_OF_constraint(td, sptr, ctfailcb, app_key);
	} else {
		ASN__CTFAIL(app_key, td, sptr,
			"%s: constraint failed (%s:%d)",
			td->name, __FILE__, __LINE__);
		return -1;
	}
}

static asn_oer_constraints_t asn_OER_type_description_constr_9 CC_NOTUSED = {
	{ 0, 0 },
	-1};
static asn_per_constraints_t asn_PER_type_description_constr_9 CC_NOTUSED = {
	{ APC_CONSTRAINED | APC_EXTENSIBLE,  2,  2,  0,  2 }	/* (0..2,...) */,
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_type_regional_constr_14 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..4)) */};
static asn_per_constraints_t asn_PER_type_regional_constr_14 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 2,  2,  1,  4 }	/* (SIZE(1..4)) */,
	0, 0	/* No PER value map */
};
static asn_oer_constraints_t asn_OER_memb_regional_constr_14 CC_NOTUSED = {
	{ 0, 0 },
	-1	/* (SIZE(1..4)) */};
static asn_per_constraints_t asn_PER_memb_regional_constr_14 CC_NOTUSED = {
	{ APC_UNCONSTRAINED,	-1, -1,  0,  0 },
	{ APC_CONSTRAINED,	 2,  2,  1,  4 }	/* (SIZE(1..4)) */,
	0, 0	/* No PER value map */
};
static asn_TYPE_member_t asn_MBR_description_9[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct GeographicalPath__description, choice.path),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_OffsetSystem,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"path"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct GeographicalPath__description, choice.geometry),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_GeometricProjection,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"geometry"
		},
	{ ATF_NOFLAGS, 0, offsetof(struct GeographicalPath__description, choice.oldRegion),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_ValidRegion,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"oldRegion"
		},
};
static const asn_TYPE_tag2member_t asn_MAP_description_tag2el_9[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* path */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* geometry */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 } /* oldRegion */
};
static asn_CHOICE_specifics_t asn_SPC_description_specs_9 = {
	sizeof(struct GeographicalPath__description),
	offsetof(struct GeographicalPath__description, _asn_ctx),
	offsetof(struct GeographicalPath__description, present),
	sizeof(((struct GeographicalPath__description *)0)->present),
	asn_MAP_description_tag2el_9,
	3,	/* Count of tags in the map */
	0, 0,
	3	/* Extensions start */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_description_9 = {
	"description",
	"description",
	&asn_OP_CHOICE,
	0,	/* No effective tags (pointer) */
	0,	/* No effective tags (count) */
	0,	/* No tags (pointer) */
	0,	/* No tags (count) */
	{ &asn_OER_type_description_constr_9, &asn_PER_type_description_constr_9, CHOICE_constraint },
	asn_MBR_description_9,
	3,	/* Elements count */
	&asn_SPC_description_specs_9	/* Additional specs */
};

static asn_TYPE_member_t asn_MBR_regional_14[] = {
	{ ATF_POINTER, 0, 0,
		(ASN_TAG_CLASS_UNIVERSAL | (16 << 2)),
		0,
		&asn_DEF_RegionalExtension_120P0,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		""
		},
};
static const ber_tlv_tag_t asn_DEF_regional_tags_14[] = {
	(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static asn_SET_OF_specifics_t asn_SPC_regional_specs_14 = {
	sizeof(struct GeographicalPath__regional),
	offsetof(struct GeographicalPath__regional, _asn_ctx),
	0,	/* XER encoding is XMLDelimitedItemList */
};
static /* Use -fall-defs-global to expose */
asn_TYPE_descriptor_t asn_DEF_regional_14 = {
	"regional",
	"regional",
	&asn_OP_SEQUENCE_OF,
	asn_DEF_regional_tags_14,
	sizeof(asn_DEF_regional_tags_14)
		/sizeof(asn_DEF_regional_tags_14[0]) - 1, /* 1 */
	asn_DEF_regional_tags_14,	/* Same as above */
	sizeof(asn_DEF_regional_tags_14)
		/sizeof(asn_DEF_regional_tags_14[0]), /* 2 */
	{ &asn_OER_type_regional_constr_14, &asn_PER_type_regional_constr_14, SEQUENCE_OF_constraint },
	asn_MBR_regional_14,
	1,	/* Single element */
	&asn_SPC_regional_specs_14	/* Additional specs */
};

asn_TYPE_member_t asn_MBR_GeographicalPath_1[] = {
	{ ATF_POINTER, 9, offsetof(struct GeographicalPath, name),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DescriptiveName,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"name"
		},
	{ ATF_POINTER, 8, offsetof(struct GeographicalPath, id),
		(ASN_TAG_CLASS_CONTEXT | (1 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_RoadSegmentReferenceID,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"id"
		},
	{ ATF_POINTER, 7, offsetof(struct GeographicalPath, anchor),
		(ASN_TAG_CLASS_CONTEXT | (2 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_Position3D,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"anchor"
		},
	{ ATF_POINTER, 6, offsetof(struct GeographicalPath, laneWidth),
		(ASN_TAG_CLASS_CONTEXT | (3 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_LaneWidth,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"laneWidth"
		},
	{ ATF_POINTER, 5, offsetof(struct GeographicalPath, directionality),
		(ASN_TAG_CLASS_CONTEXT | (4 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_DirectionOfUse,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"directionality"
		},
	{ ATF_POINTER, 4, offsetof(struct GeographicalPath, closedPath),
		(ASN_TAG_CLASS_CONTEXT | (5 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_BOOLEAN,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"closedPath"
		},
	{ ATF_POINTER, 3, offsetof(struct GeographicalPath, direction),
		(ASN_TAG_CLASS_CONTEXT | (6 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_HeadingSlice,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"direction"
		},
	{ ATF_POINTER, 2, offsetof(struct GeographicalPath, description),
		(ASN_TAG_CLASS_CONTEXT | (7 << 2)),
		+1,	/* EXPLICIT tag at current level */
		&asn_DEF_description_9,
		0,
		{ 0, 0, 0 },
		0, 0, /* No default value */
		"description"
		},
	{ ATF_POINTER, 1, offsetof(struct GeographicalPath, regional),
		(ASN_TAG_CLASS_CONTEXT | (8 << 2)),
		0,
		&asn_DEF_regional_14,
		0,
		{ &asn_OER_memb_regional_constr_14, &asn_PER_memb_regional_constr_14,  memb_regional_constraint_1 },
		0, 0, /* No default value */
		"regional"
		},
};
static const int asn_MAP_GeographicalPath_oms_1[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
static const ber_tlv_tag_t asn_DEF_GeographicalPath_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_GeographicalPath_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 }, /* name */
    { (ASN_TAG_CLASS_CONTEXT | (1 << 2)), 1, 0, 0 }, /* id */
    { (ASN_TAG_CLASS_CONTEXT | (2 << 2)), 2, 0, 0 }, /* anchor */
    { (ASN_TAG_CLASS_CONTEXT | (3 << 2)), 3, 0, 0 }, /* laneWidth */
    { (ASN_TAG_CLASS_CONTEXT | (4 << 2)), 4, 0, 0 }, /* directionality */
    { (ASN_TAG_CLASS_CONTEXT | (5 << 2)), 5, 0, 0 }, /* closedPath */
    { (ASN_TAG_CLASS_CONTEXT | (6 << 2)), 6, 0, 0 }, /* direction */
    { (ASN_TAG_CLASS_CONTEXT | (7 << 2)), 7, 0, 0 }, /* description */
    { (ASN_TAG_CLASS_CONTEXT | (8 << 2)), 8, 0, 0 } /* regional */
};
asn_SEQUENCE_specifics_t asn_SPC_GeographicalPath_specs_1 = {
	sizeof(struct GeographicalPath),
	offsetof(struct GeographicalPath, _asn_ctx),
	asn_MAP_GeographicalPath_tag2el_1,
	9,	/* Count of tags in the map */
	asn_MAP_GeographicalPath_oms_1,	/* Optional members */
	9, 0,	/* Root/Additions */
	9,	/* First extension addition */
};
asn_TYPE_descriptor_t asn_DEF_GeographicalPath = {
	"GeographicalPath",
	"GeographicalPath",
	&asn_OP_SEQUENCE,
	asn_DEF_GeographicalPath_tags_1,
	sizeof(asn_DEF_GeographicalPath_tags_1)
		/sizeof(asn_DEF_GeographicalPath_tags_1[0]), /* 1 */
	asn_DEF_GeographicalPath_tags_1,	/* Same as above */
	sizeof(asn_DEF_GeographicalPath_tags_1)
		/sizeof(asn_DEF_GeographicalPath_tags_1[0]), /* 1 */
	{ 0, 0, SEQUENCE_constraint },
	asn_MBR_GeographicalPath_1,
	9,	/* Elements count */
	&asn_SPC_GeographicalPath_specs_1	/* Additional specs */
};

