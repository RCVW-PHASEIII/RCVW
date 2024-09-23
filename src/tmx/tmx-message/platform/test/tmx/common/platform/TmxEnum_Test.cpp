/*
 * Copyright (c) 2022 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxEnum_Test.cpp
 *
 *  Created on: Oct 11, 2022
 *      @author: Gregory M. Baumgardner
 */

#define TMX_ENUM_RANGE_MAX 1300
#define TMX_ENUM_CASE_COMPARE 1

// We really only need to verify the non-magic version,
// although commenting this out would also test the
// Magic Enum version
//#define TMX_NO_MAGIC_ENUM 1
#include <iostream>
#include <tmx/common/platform/enumeration/enum.hpp>

#include <boost/test/unit_test.hpp>

namespace com {
namespace example {

enum class DeutschNumber {
	Null,
	Eins,
	Zwei,
	Drei,
	Vier,
	Funf,
	Sechs,
	Sieben,
	Acht,
	Neun,
	Zehn,
	Elf,
	Zwolf,
	Dreizehn,
	Vierzehn,
	Funfzehn,
	Sechzehn,
	Siebzehn,
	Achtzehn,
	Neunzehn,
	Zwanzig
};

enum class RTCM3_MESSAGE_TYPE {
	L1OnlyGPSRTKObservable = 1001,
	ExtendedL1OnlyGPSRTKObservable = 1002,
	L1L2GPSRTKObservable = 1003,
	ExtendedL1L2GPSRTKObservable = 1004,
	StationaryRTKReferenceStationARP = 1005,
	StationaryRTKReferenceStationARPwithAntennaHeight = 1006,
	AntennaDescriptor = 1007,
	AntennaDescriptorAndSerialNumber = 1008,
	L1OnlyGLONASSRTKObservable = 1009,
	ExtendedL1OnlyGLONASSRTKObservable = 1010,
	L1L2GLONASSRTKObservable = 1011,
	ExtendedL1L2GLONASSRTKObservable = 1012,
	SystemParameters = 1013,
	NetworkAuxiliaryStationData = 1014,
	GPSIonosphericCorrectionDifferences = 1015,
	GPSGeometricCorrectionDifferences = 1016,
	GPSCombinedGeometricAndIonosphericCorrectionDifferences = 1017,
	AlternativeIonosphericCorrectDifferences = 1018,
	GPSEphmerides = 1019,
	GLONASSEphemerides = 1020,
	HelmertAbridgedMolodenskiTransformationParameters = 1021,
	MolodenskiBadekasTransformationParameters = 1022,
	ResidualsEllipsoidalGridRepresentation = 1023,
	ResidualsPlanGridRepresentation = 1024,
	ProjectionParametersProjectionTypesOtherLCC2SP = 1025,
	ProjectionParametersLCC2SP = 1026,
	ProjectionParametersOM = 1027,
	GlobalToPlateFixedTransformation = 1028,
	UnicodeTextString = 1029,
	GPSNetworkRTKResidualMessage = 1030,
	GLONASSNetworkRTKResidualMessage = 1031,
	PhysicalReferenceStationPositionMessage = 1032,
	ReceiverAndAntennaDescription = 1033,
	GPSNetworkFKPGradient = 1034,
	GLONASSNetworkFKPGradient = 1035,
	GLONASSIonosphericCorrectionDifferences = 1037,
	GLONASSGeometricCorrectionDifferences = 1038,
	GLONASSCombinedGeometricAndIonosphericCorrectionDifferences = 1039,
	BDSSatelliteEphemerisData = 1042,
	QZSSEphemerides = 1044,
	GalileoFNAVSatelliteEphemerisData = 1045,
	GalileoINAVSatelliteEphemerisData = 1046,
	SSRGPSOribitCorrection = 1057,
	SSRGPSClockCorrection = 1058,
	SSRGPSCodeBias = 1059,
	SSRGPSCombinedOrbitAndClockCorrection = 1060,
	SSRGPSURA = 1061,
	SSRGPSHighRateClockCorrection = 1062,
	SSRGLONASSOribitCorrection = 1063,
	SSRGLONASSClockCorrection = 1064,
	SSRGLONASSCodeBias = 1065,
	SSRGLONASSCombinedOrbitAndClockCorrection = 1066,
	SSRGLONASSURA = 1067,
	SSRGLONASSHighRateClockCorrection = 1068,
	GPSMSM1 = 1071,
	GPSMSM2 = 1072,
	GPSMSM3 = 1073,
	GPSMSM4 = 1074,
	GPSMSM5 = 1075,
	GPSMSM6 = 1076,
	GPSMSM7 = 1077,
	GLONASSMSM1 = 1081,
	GLONASSMSM2 = 1082,
	GLONASSMSM3 = 1083,
	GLONASSMSM4 = 1084,
	GLONASSMSM5 = 1085,
	GLONASSMSM6 = 1086,
	GLONASSMSM7 = 1087,
	GalileoMSM1 = 1091,
	GalileoMSM2 = 1092,
	GalileoMSM3 = 1093,
	GalileoMSM4 = 1094,
	GalileoMSM5 = 1095,
	GalileoMSM6 = 1096,
	GalileoMSM7 = 1097,
	SBASMSM1 = 1101,
	SBASMSM2 = 1102,
	SBASMSM3 = 1103,
	SBASMSM4 = 1104,
	SBASMSM5 = 1105,
	SBASMSM6 = 1106,
	SBASMSM7 = 1107,
	QZSSMSM1 = 1111,
	QZSSMSM2 = 1112,
	QZSSMSM3 = 1113,
	QZSSMSM4 = 1114,
	QZSSMSM5 = 1115,
	QZSSMSM6 = 1116,
	QZSSMSM7 = 1117,
	BeiDouMSM1 = 1121,
	BeiDouMSM2 = 1122,
	BeiDouMSM3 = 1123,
	BeiDouMSM4 = 1124,
	BeiDouMSM5 = 1125,
	BeiDouMSM6 = 1126,
	BeiDouMSM7 = 1127,
	GLONASSL1L2CodePhaseBiases = 1230
};


// This is a hack because Boost unit test does not correctly use namespaces to find the
// operator<< overload. The unit test enums must be in the same namespace as their
// ostream operators.
std::ostream &operator<<(std::ostream &os, com::example::DeutschNumber value) {
	os << tmx::common::enums::enum_name(value);
	return os;
}

std::ostream &operator<<(std::ostream &os, std::optional<com::example::DeutschNumber> const &value) {
	if (value)
		os << tmx::common::enums::enum_name(*value);
	return os;
}

std::ostream &operator<<(std::ostream &os, com::example::RTCM3_MESSAGE_TYPE value) {
	os << tmx::common::enums::enum_name(value);
	return os;
}

std::ostream &operator<<(std::ostream &os, std::optional<com::example::RTCM3_MESSAGE_TYPE> const &value) {
	if (value)
		os << tmx::common::enums::enum_name(*value);
	return os;
}

}
}

TMX_ENUM_SEQUENCE(com::example::DeutschNumber, Null, Eins, Zwei, Drei, Vier, Funf, Sechs, Sieben, Acht, Neun, Zehn, Elf, Zwolf, Dreizehn, Vierzehn, Funfzehn, Sechzehn, Siebzehn, Achtzehn, Neunzehn, Zwanzig)

#define RTCM3_MESSAGES_PT1 L1OnlyGPSRTKObservable, ExtendedL1OnlyGPSRTKObservable, L1L2GPSRTKObservable, ExtendedL1L2GPSRTKObservable, StationaryRTKReferenceStationARP, \
		StationaryRTKReferenceStationARPwithAntennaHeight, AntennaDescriptor, AntennaDescriptorAndSerialNumber, L1OnlyGLONASSRTKObservable, ExtendedL1OnlyGLONASSRTKObservable, L1L2GLONASSRTKObservable, \
		ExtendedL1L2GLONASSRTKObservable, SystemParameters, NetworkAuxiliaryStationData, GPSIonosphericCorrectionDifferences, GPSGeometricCorrectionDifferences, \
		GPSCombinedGeometricAndIonosphericCorrectionDifferences, AlternativeIonosphericCorrectDifferences, GPSEphmerides, GLONASSEphemerides, HelmertAbridgedMolodenskiTransformationParameters, \
		MolodenskiBadekasTransformationParameters, ResidualsEllipsoidalGridRepresentation, ResidualsPlanGridRepresentation, ProjectionParametersProjectionTypesOtherLCC2SP, ProjectionParametersLCC2SP
#define RTCM3_MESSAGES_PT2	ProjectionParametersOM, GlobalToPlateFixedTransformation, UnicodeTextString, GPSNetworkRTKResidualMessage, GLONASSNetworkRTKResidualMessage, PhysicalReferenceStationPositionMessage, \
		ReceiverAndAntennaDescription, GPSNetworkFKPGradient, GLONASSNetworkFKPGradient, GLONASSIonosphericCorrectionDifferences, GLONASSGeometricCorrectionDifferences, \
		GLONASSCombinedGeometricAndIonosphericCorrectionDifferences, BDSSatelliteEphemerisData, QZSSEphemerides, GalileoFNAVSatelliteEphemerisData, GalileoINAVSatelliteEphemerisData, \
		SSRGPSOribitCorrection, SSRGPSClockCorrection, SSRGPSCodeBias, SSRGPSCombinedOrbitAndClockCorrection, SSRGPSURA, SSRGPSHighRateClockCorrection, SSRGLONASSOribitCorrection, \
		SSRGLONASSClockCorrection, SSRGLONASSCodeBias, SSRGLONASSCombinedOrbitAndClockCorrection, SSRGLONASSURA, SSRGLONASSHighRateClockCorrection
#define	RTCM3_MESSAGES_PT3 GPSMSM1, GPSMSM2, GPSMSM3, GPSMSM4, GPSMSM5, GPSMSM6, GPSMSM7, GLONASSMSM1, GLONASSMSM2, GLONASSMSM3, GLONASSMSM4, GLONASSMSM5, GLONASSMSM6, GLONASSMSM7, \
		GalileoMSM1, GalileoMSM2, GalileoMSM3, GalileoMSM4, GalileoMSM5, GalileoMSM6, GalileoMSM7, SBASMSM1, SBASMSM2, SBASMSM3, SBASMSM4, SBASMSM5, SBASMSM6, SBASMSM7, \
		QZSSMSM1, QZSSMSM2, QZSSMSM3, QZSSMSM4, QZSSMSM5, QZSSMSM6, QZSSMSM7, BeiDouMSM1, BeiDouMSM2, BeiDouMSM3, BeiDouMSM4, BeiDouMSM5, BeiDouMSM6, BeiDouMSM7, GLONASSL1L2CodePhaseBiases

namespace tmx { namespace common { namespace enums {

template <> constexpr auto enum_sequence<com::example::RTCM3_MESSAGE_TYPE>() noexcept {
	return static_array<com::example::RTCM3_MESSAGE_TYPE,
			TMX_ENUM_GENERATE_ALL_IDS(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT1),
			TMX_ENUM_GENERATE_ALL_IDS(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT2),
			TMX_ENUM_GENERATE_ALL_IDS(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT3)
	> {};
}
template <> constexpr auto enum_names<com::example::RTCM3_MESSAGE_TYPE>() noexcept { \
	return std::array<const_string, enum_count<com::example::RTCM3_MESSAGE_TYPE>()> {
			TMX_ENUM_GENERATE_ALL_NAMES(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT1),
			TMX_ENUM_GENERATE_ALL_NAMES(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT2),
			TMX_ENUM_GENERATE_ALL_NAMES(com::example::RTCM3_MESSAGE_TYPE, RTCM3_MESSAGES_PT3)
	};
}

}}}

enum class NoNamespace {};

using namespace std;
using namespace tmx::common;
using namespace tmx::common::enums;
using namespace com::example;

BOOST_AUTO_TEST_CASE( tmxtest_enum_cast ) {
	// Some basic sanity checks
	BOOST_CHECK_EQUAL(enum_cast<DeutschNumber>(11),     DeutschNumber::Elf);
	BOOST_CHECK_LT   (enum_cast<DeutschNumber>(5),      DeutschNumber::Neun);
	BOOST_CHECK_EQUAL(enum_cast<DeutschNumber>("Eins"), DeutschNumber::Eins);
	BOOST_CHECK_NE   (enum_cast<DeutschNumber>("acht"), DeutschNumber::Acht);

	// Verify all German number conversions
	for (size_t n = 0; n <= 20; n++) {
		DeutschNumber num = static_cast<DeutschNumber>(n);
		BOOST_CHECK_EQUAL(enum_cast<DeutschNumber>(n), num);
	}

	// Verify all RTCM .message name conversions
	const string nm[] { "GPS", "GLONASS", "Galileo", "SBAS", "QZSS", "BeiDou" };
	for (size_t i = 0; i < 6; i++) {
		for (size_t j = 1; j <= 7; j++) {
			string s = nm[i] + "MSM" + ::to_string(j);
			size_t num = 1070 + 10 * i + j;

			auto n = static_cast<RTCM3_MESSAGE_TYPE>(num);
			BOOST_CHECK_EQUAL(enum_cast<RTCM3_MESSAGE_TYPE>(s), n);

			transform(s.begin(), s.end(), s.begin(), ::tolower);
			BOOST_CHECK_NE(enum_cast<RTCM3_MESSAGE_TYPE>(s), n);
			BOOST_CHECK_EQUAL(enum_cast<RTCM3_MESSAGE_TYPE>(s, false), n);

			if (i == 2 || i == 5) {
				transform(s.begin(), s.end(), s.begin(), ::tolower);
				BOOST_CHECK_NE(enum_cast<RTCM3_MESSAGE_TYPE>(s), n);
				BOOST_CHECK_EQUAL(enum_cast<RTCM3_MESSAGE_TYPE>(s, false), n);
			}
		}
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_value ) {
	BOOST_CHECK_EQUAL(enum_value<DeutschNumber>(0), DeutschNumber::Null);
	BOOST_CHECK_EQUAL(enum_value<DeutschNumber>(15), DeutschNumber::Funfzehn);
	BOOST_CHECK_EQUAL(enum_value<RTCM3_MESSAGE_TYPE>(7), RTCM3_MESSAGE_TYPE::AntennaDescriptorAndSerialNumber);
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_values ) {
	auto values = enum_values<DeutschNumber>();
	BOOST_CHECK_EQUAL(values.size(), 21);

	for (size_t i = 0; i < values.size(); i++) {
		auto n = static_cast<DeutschNumber>(i);
		BOOST_CHECK_EQUAL(values[i], n);
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_count ) {
	BOOST_CHECK_EQUAL(enum_count<DeutschNumber>(), 21);
	BOOST_CHECK_EQUAL(enum_count<RTCM3_MESSAGE_TYPE>(), 97);
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_integer ) {
	int x;
	// Check all the German numbers and the first 20 RTCM
	for (size_t i = 0; i <= 20; i++) {
		x = enum_integer(static_cast<DeutschNumber>(i));
		BOOST_CHECK_EQUAL(x, i);
		x = enum_integer(static_cast<RTCM3_MESSAGE_TYPE>(i + 1001));
		BOOST_CHECK_EQUAL(x, i + 1001);
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_name ) {
	auto x = enum_name<DeutschNumber::Drei>();
	BOOST_TEST(strcmp("Drei", x.data()) == 0);

	// Verify all RTCM .message name conversions
	const string nm[] { "GPS", "GLONASS", "Galileo", "SBAS", "QZSS", "BeiDou" };
	for (size_t i = 0; i < 6; i++) {
		for (size_t j = 1; j <= 7; j++) {
			string s = nm[i] + "MSM" + ::to_string(j);
			size_t num = 1070 + 10 * i + j;

			auto n = static_cast<RTCM3_MESSAGE_TYPE>(num);
			BOOST_CHECK_EQUAL(enum_name(n), s);
		}
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_names ) {
	auto values = enum_names<RTCM3_MESSAGE_TYPE>();
	BOOST_CHECK_EQUAL(values.size(), 97);

	// Verify all RTCM .message name conversions
	const string nm[] { "GPS", "GLONASS", "Galileo", "SBAS", "QZSS", "BeiDou" };

	size_t idx = 54;
	for (size_t i = 0; i < 6; i++) {
		for (size_t j = 1; j <= 7; j++) {
			string s = nm[i] + "MSM" + ::to_string(j);
			BOOST_CHECK_EQUAL(values[idx++], s);
		}
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_entries ) {
	auto values = enum_entries<DeutschNumber>();
	BOOST_CHECK_EQUAL(values.size(), 21);

	for (size_t i = 0; i < values.size(); i++) {
		auto n = static_cast<DeutschNumber>(i);
		BOOST_CHECK_EQUAL(values[i].first, n);
	}

	auto moreValues = enum_entries<RTCM3_MESSAGE_TYPE>();
	BOOST_CHECK_EQUAL(moreValues.size(), 97);

	// Verify all RTCM .message name conversions
	const string nm[] { "GPS", "GLONASS", "Galileo", "SBAS", "QZSS", "BeiDou" };

	size_t idx = 54;
	for (size_t i = 0; i < 6; i++) {
		for (size_t j = 1; j <= 7; j++) {
			string s = nm[i] + "MSM" + ::to_string(j);
			BOOST_CHECK_EQUAL(moreValues[idx++].second, s);
		}
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_index ) {
	size_t x;
	// Check all the German numbers and the first 20 RTCM
	for (size_t i = 0; i <= 20; i++) {
		x = enum_index(static_cast<DeutschNumber>(i));
		BOOST_CHECK_EQUAL(x, i);
		x = enum_index(static_cast<RTCM3_MESSAGE_TYPE>(i + 1001));
		BOOST_CHECK_EQUAL(x, i);
	}
}

BOOST_AUTO_TEST_CASE( tmxtest_enum_contains ) {
	for (size_t i = 0; i <= 20; i++) {
		auto n = static_cast<DeutschNumber>(i);
		BOOST_CHECK_EQUAL(enum_contains(n), true);
		BOOST_CHECK_EQUAL(enum_contains<DeutschNumber>(i), true);
		BOOST_CHECK_EQUAL(enum_contains<RTCM3_MESSAGE_TYPE>(i), false);
	}

	// Verify all RTCM .message name conversions
	const string nm[] { "GPS", "GLONASS", "Galileo", "SBAS", "QZSS", "BeiDou" };

	for (size_t i = 0; i < 6; i++) {
		for (size_t j = 1; j <= 7; j++) {
			string s = nm[i] + "MSM" + ::to_string(j);
			size_t num = 1070 + 10 * i + j;

			BOOST_CHECK_EQUAL(enum_contains<DeutschNumber>(s), false);
			BOOST_CHECK_EQUAL(enum_contains<DeutschNumber>(num), false);
			BOOST_CHECK_EQUAL(enum_contains<RTCM3_MESSAGE_TYPE>(s), true);
			BOOST_CHECK_EQUAL(enum_contains<RTCM3_MESSAGE_TYPE>(num), true);
		}
	}

}

BOOST_AUTO_TEST_CASE( tmxtest_enum_type_name ) {
	// Type names will have the fully qualified namespace
	auto val = DeutschNumber::Vier;
	BOOST_CHECK_EQUAL(enum_type_name(val), "com::example::DeutschNumber");
	BOOST_CHECK_EQUAL(enum_type_name<RTCM3_MESSAGE_TYPE>(), "com::example::RTCM3_MESSAGE_TYPE");
	BOOST_CHECK_EQUAL(enum_type_name<NoNamespace>(), "NoNamespace");
}

