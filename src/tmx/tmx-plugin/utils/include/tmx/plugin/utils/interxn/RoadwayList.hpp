/*
 * RoadwayList.h
 *
 *  Created on: Jun 16, 2016
 *      Author: ivp
 */

#ifndef SRC_INTERSECTIONLIST_H_
#define SRC_INTERSECTIONLIST_H_

#include "Roadway.hpp"

#include <iostream>
#include <unordered_map>
#include <hashtable.h>

namespace tmx {
namespace plugin {
namespace utils {
namespace interxn {

///Tmx units may be within range of two or more MAP messages. Both of these need to be
///simultaneously maintained (for efficiency) within memory.
class RoadwayList {
public:
    RoadwayList();

    virtual ~RoadwayList();

    ///Wrapper for Roadway.LoadMap.  Maintains a list of all MAPped Roadways. Returns a Pair containing
    ///the roadway object resulting from the MapDataMessage, and true if the map was loaded new and false if it was unchanged data.
    std::pair<Roadway *, bool> LoadMap(MapData &msg);

private:

    ///Holds list of [RoadwayId, roadway object] for all encountered MAP files.
    std::unordered_map<int, Roadway> _roadways;
};

}}}} /* namespace tmx::plugin::utils::interxn */

#endif /* SRC_INTERSECTIONLIST_H_ */
