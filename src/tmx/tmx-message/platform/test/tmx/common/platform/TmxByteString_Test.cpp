/*!
 * Copyright (c) 2023 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxByteString_Test.cpp
 *
 *  Created on: Jul 19, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/common/platform/types/byte_string.hpp>

#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test.hpp>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>

using namespace tmx::common;

const char strVal[] = "This is a TMX unit test string with 50 characters.";
const byte_sequence strBytes = to_byte_sequence(strVal, 50);

const std::uint8_t ucharVal = 0x4D;
const byte_t ucharBytes[] = { (byte_t)ucharVal };

const std::int8_t charVal = -1 * ucharVal;
const byte_t charBytes[] = { (byte_t)0xB3 };

const std::uint32_t uintVal = 150263987;
const byte_t uintBytes[] = { (byte_t)0x08, (byte_t)0xF4, (byte_t)0xD8, (byte_t)0xB3 };

const std::int32_t intVal = -1 * uintVal;
const byte_t intBytes[] = { (byte_t)0xF7, (byte_t)0x0B, (byte_t)0x27, (byte_t)0x4D };

const double floatVal = -3.1234000987654e9;
const byte_t floatBytes[] = { (byte_t)0xC1, (byte_t)0xE7, (byte_t)0x45, (byte_t)0x69,
                              (byte_t)0xB4, (byte_t)0x58, (byte_t)0x7E, (byte_t)0x28};

template <typename _Map>
void check_byte_strings(_Map const &m) {
    int base = m.size;
    int bits = log(base) / log(2);
    double factor = bits / 8.0;
    int chars = 1 / factor;

    // Encode the string
    auto encoded = byte_string_encode(strBytes, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), strlen(strVal));

    int cnt = 0;
    for (const char &c: strVal) {
        if (cnt < encoded.length()) {
            auto s = encoded.substr(cnt, chars);
            BOOST_CHECK_EQUAL(c, strtol(s.c_str(), nullptr, base));

            cnt += chars;
        }
    }

    // Decode the string
    auto decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(0, strcmp((const char *)decoded.c_str(), strVal));

    // Encode the unsigned byte
    encoded = byte_string_encode_value(ucharVal, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), 1);
    BOOST_CHECK_EQUAL((std::uint8_t)ucharBytes[0], strtol(encoded.c_str(), nullptr, base));

    // Decode the unsigned byte
    decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(ucharVal, get_value<std::uint8_t>(decoded.c_str()));

    // Encode the signed byte
    encoded = byte_string_encode_value(charVal, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), 1);
    BOOST_CHECK_EQUAL((std::uint8_t)charBytes[0], strtol(encoded.c_str(), nullptr, base));

    // Decode the signed byte
    decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(charVal, get_value<std::int8_t>(decoded.c_str()));

    // Encode the unsigned integer
    encoded = byte_string_encode_value(uintVal, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), 4);
    cnt = 0;
    for (const byte_t &b: uintBytes) {
        if (cnt < encoded.length()) {
            auto s = encoded.substr(cnt, chars);
            BOOST_CHECK_EQUAL((std::uint8_t)b, strtol(s.c_str(), nullptr, base));

            cnt += chars;
        }
    }

    // Decode the unsigned integer
    decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(uintVal, get_value<std::uint32_t>(decoded.c_str()));

    // Encode the signed integer
    encoded = byte_string_encode_value(intVal, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), 4);

    cnt = 0;
    for (const byte_t &b: intBytes) {
        if (cnt < encoded.length()) {
            auto s = encoded.substr(cnt, chars);
            BOOST_CHECK_EQUAL((std::uint8_t)b, strtol(s.c_str(), nullptr, base));

            cnt += chars;
        }
    }

    // Decode the signed integer
    decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(intVal, get_value<std::int32_t>(decoded.c_str()));

    // Encode the double
    encoded = byte_string_encode_value(floatVal, m);
    BOOST_CHECK_EQUAL(factor * encoded.size(), 8);

    cnt = 0;
    for (const byte_t &b: floatBytes) {
        if (cnt < encoded.length()) {
            auto s = encoded.substr(cnt, chars);
            BOOST_CHECK_EQUAL((std::uint8_t)b, strtol(s.c_str(), nullptr, base));

            cnt += chars;
        }
    }

    // Decode the double
    decoded = byte_string_decode(encoded, m);
    BOOST_CHECK_EQUAL(floatVal, get_value<double>(decoded.c_str()));
}


BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_bin ) {
    check_byte_strings(tmx::common::binary::value);
}

BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_hex ) {
    check_byte_strings(tmx::common::hexadecimal::value);
}

BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_base4 ) {
    check_byte_strings(tmx::common::static_array<char, '0', '1', '2', '3'> { });
}


//BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_octal ) {
//    std::uint64_t val = 10770587297673;
//    auto encoded = byte_string_encode_value(val, tmx::common::octal::value);
//    auto decoded = byte_string_decode(encoded, tmx::common::octal::value);
//
//    BOOST_CHECK_EQUAL(encoded, "234567101723611");
//    BOOST_CHECK_EQUAL((const char *)decoded.c_str(), (const char *)floatBytes);
//}

BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_base32 ) {
    const char *phrase = "The quick brown fox jumps over the lazy dog!";
    const char *phraseB32 = "KRUGKIDROVUWG2ZAMJZG653OEBTG66BANJ2W24DTEBXXMZLSEB2GQZJANRQXU6JAMRXWOII=";

    auto encoded = byte_string_encode(to_byte_sequence(phrase), tmx::common::base32::value);
    auto decoded = byte_string_decode(encoded, tmx::common::base32::value);
    BOOST_CHECK_EQUAL(encoded, phraseB32);
    BOOST_CHECK_EQUAL((const char *)decoded.c_str(), phrase);
}

BOOST_AUTO_TEST_CASE ( tmxtest_byte_encoding_base64 ) {
    const char *phrase = "The Lord is my shepherd; I shall not want.\n"
                         "He maketh me to lie down in green pastures: he leadeth me beside the still waters.\n"
                         "He restoreth my soul: he leadeth me in the paths of righteousness for his name's sake.\n"
                         "Yea, though I walk through the valley of the shadow of death, I will fear no evil: for thou art with me; thy rod and thy staff they comfort me.\n"
                         "Thou preparest a table before me in the presence of mine enemies: thou anointest my head with oil; my cup runneth over.\n"
                         "Surely goodness and mercy shall follow me all the days of my life: and I will dwell in the house of the Lord for ever.";
    const char *phraseB32 = "VGhlIExvcmQgaXMgbXkgc2hlcGhlcmQ7IEkgc2hhbGwgbm90IHdhbnQuCkhlIG1ha2V0aCBtZSB0"
                            "byBsaWUgZG93biBpbiBncmVlbiBwYXN0dXJlczogaGUgbGVhZGV0aCBtZSBiZXNpZGUgdGhlIHN0"
                            "aWxsIHdhdGVycy4KSGUgcmVzdG9yZXRoIG15IHNvdWw6IGhlIGxlYWRldGggbWUgaW4gdGhlIHBh"
                            "dGhzIG9mIHJpZ2h0ZW91c25lc3MgZm9yIGhpcyBuYW1lJ3Mgc2FrZS4KWWVhLCB0aG91Z2ggSSB3"
                            "YWxrIHRocm91Z2ggdGhlIHZhbGxleSBvZiB0aGUgc2hhZG93IG9mIGRlYXRoLCBJIHdpbGwgZmVh"
                            "ciBubyBldmlsOiBmb3IgdGhvdSBhcnQgd2l0aCBtZTsgdGh5IHJvZCBhbmQgdGh5IHN0YWZmIHRo"
                            "ZXkgY29tZm9ydCBtZS4KVGhvdSBwcmVwYXJlc3QgYSB0YWJsZSBiZWZvcmUgbWUgaW4gdGhlIHBy"
                            "ZXNlbmNlIG9mIG1pbmUgZW5lbWllczogdGhvdSBhbm9pbnRlc3QgbXkgaGVhZCB3aXRoIG9pbDsg"
                            "bXkgY3VwIHJ1bm5ldGggb3Zlci4KU3VyZWx5IGdvb2RuZXNzIGFuZCBtZXJjeSBzaGFsbCBmb2xs"
                            "b3cgbWUgYWxsIHRoZSBkYXlzIG9mIG15IGxpZmU6IGFuZCBJIHdpbGwgZHdlbGwgaW4gdGhlIGhv"
                            "dXNlIG9mIHRoZSBMb3JkIGZvciBldmVyLg==";

    auto encoded = byte_string_encode(to_byte_sequence(phrase), tmx::common::base64::value);
    auto decoded = byte_string_decode(encoded, tmx::common::base64::value);
    BOOST_CHECK_EQUAL(encoded, phraseB32);
    BOOST_CHECK_EQUAL((const char *)decoded.c_str(), phrase);
}