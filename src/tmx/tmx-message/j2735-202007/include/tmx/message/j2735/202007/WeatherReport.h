/*
 * Generated by asn1c-0.9.29 (http://lionet.info/asn1c)
 * From ASN.1 module "BasicSafetyMessage"
 * 	found in "../../../../../../../../../../src/tmx/tmx-message/j2735-202007/J2735-BasicSafetyMessage.asn"
 * 	`asn1c -fwide-types -fcompound-names -fincludes-quoted -fnative-types -no-gen-example`
 */

#ifndef	_WeatherReport_H_
#define	_WeatherReport_H_


#include "asn_application.h"

/* Including external dependencies */
#include "EssPrecipYesNo.h"
#include "EssPrecipRate.h"
#include "EssPrecipSituation.h"
#include "EssSolarRadiation.h"
#include "EssMobileFriction.h"
#include "CoefficientOfFriction.h"
#include "constr_SEQUENCE.h"

#ifdef __cplusplus
extern "C" {
#endif

/* WeatherReport */
typedef struct WeatherReport {
	EssPrecipYesNo_t	 isRaining;
	EssPrecipRate_t	*rainRate	/* OPTIONAL */;
	EssPrecipSituation_t	*precipSituation	/* OPTIONAL */;
	EssSolarRadiation_t	*solarRadiation	/* OPTIONAL */;
	EssMobileFriction_t	*friction	/* OPTIONAL */;
	CoefficientOfFriction_t	*roadFriction	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} WeatherReport_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_WeatherReport;
extern asn_SEQUENCE_specifics_t asn_SPC_WeatherReport_specs_1;
extern asn_TYPE_member_t asn_MBR_WeatherReport_1[6];

#ifdef __cplusplus
}
#endif

#endif	/* _WeatherReport_H_ */
#include "asn_internal.h"
