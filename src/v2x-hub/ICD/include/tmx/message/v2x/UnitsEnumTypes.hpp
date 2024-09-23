
/**
 * WARNING: DO NOT EDIT THIS FILE
 *
 * This source file was generated from the CPlusPlusGenerator.xsl stylesheet
 *
 * Your changes will be overwritten when this tool executes again.  In order
 * to edit the contents, you must go into the source XML files
 *
 * @author Greg Baumgardner
 */
 

#ifndef INCLUDE_UNITSENUMTYPES_H_
#define INCLUDE_UNITSENUMTYPES_H_

#include <tmx/common/types/Enum.hpp>

namespace tmx {
namespace message {
namespace v2x {
namespace units {

enum class Time {
    ns = 0,
    us = 1,
    ms = 2,
    s = 3,
    min = 4,
    hrs = 5,
    days = 6
};

enum class Distance {
    nm = 0,
    um = 1,
    mm = 2,
    cm = 3,
    m = 4,
    km = 5,
    in = 6,
    ft = 7,
    yd = 8,
    mi = 9
};


enum class Speed {
    mps = 0,
    kph = 1,
    ftpers = 2,
    mph = 3,
    knots = 4
};

enum class Acceleration {
    Gal = 0,
    mperspers = 1,
    kmperhrperhr = 2,
    ftperspers = 3,
    miperhrperhr = 4,
    g = 5
};

enum class Mass {
    mg = 0,
    g = 1,
    kg = 2,
    tonne = 3,
    oz = 4,
    lb = 5,
    ton = 6,
    st = 7
};

enum class Force {
    N = 0,
    dyn = 1,
    lbf = 2,
    kp = 3
};

enum class Energy {
    J = 0,
    kJ = 1,
    cal = 2,
    kcal = 3,
    BTU = 4,
    kWh = 5,
    ftlb = 6,
    eV = 7,
    erg = 8
};

enum class Power {
    mW = 0,
    W = 1,
    kW = 2,
    MegaW = 3,
    GW = 4,
    ergspers = 5,
    lbfpers = 6,
    hp = 7,
    dBm = 8,
    kcalperhr = 9
};


enum class Angle {
    deg = 0,
    pirad = 1,
    rad = 2,
    grad = 3
};

enum class Frequency {
    Hz = 0,
    kHz = 1,
    MHz = 2,
    GHz = 3,
    rpm = 4,
    piradpers = 5,
    radpers = 6
};

enum class Temperature {
    C = 0,
    F = 1,
    K = 2
};

enum class Percent {
    decimal = 0,
    pct = 1
};

enum class Area {
    sqcm = 0,
    sqm = 1,
    sqkm = 2,
    ha = 3,
    sqin = 4,
    sqft = 5,
    sqyd = 6,
    sqmi = 7,
    acre = 8
};

enum class Volume {
    ml = 0,
    l = 1,
    cubicm = 2,
    cubicin = 3,
    cubicft = 4,
    cubicyd = 5,
    tsp = 6,
    tbsp = 7,
    floz = 8,
    cup = 9,
    pt = 10,
    qt = 11,
    gal = 12
};

enum class Pressure {
    Pa = 0,
    bar = 1,
    atm = 2,
    psi = 3,
    mmhg = 4
};

enum class FuelEconomy {
    kmperl = 0,
    mpg = 1
};

enum class Digital {
    bits = 0,
    kbit = 1,
    Mbit = 2,
    Gbit = 3,
    Tbit = 4,
    Pbit = 5,
    bytes = 6,
    kB = 7,
    MB = 8,
    GB = 9,
    TB = 10,
    PB = 11
};

enum class Bandwidth {
    bps = 0,
    kbps = 1,
    Mbps = 2,
    Gbps = 3,
    Tbps = 4,
    Pbps = 5,
    bytespers = 6,
    kbytespers = 7,
    Mbytespers = 8,
    Gbytespers = 9,
    Tbytespers = 10,
    Pbytespers = 11
};

} /* End namespace units */
} /* End namespace v2x */
} /* End namespace messages */
} /* End namespace tmx */

#endif /* INCLUDE_UNITSENUMTYPES_H_ */
	