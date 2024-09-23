/*
 * IntersectionList.h
 *
 *  Created on: Jun 16, 2016
 *      Author: ivp
 */

#ifndef SRC_INTERSECTIONLIST_H_
#define SRC_INTERSECTIONLIST_H_

#include "Intersection.hpp"

#include <iostream>
#include <unordered_map>
#include <hashtable.h>

namespace tmx {
namespace plugin {
namespace utils {
namespace interxn {

///Tmx units may be within range of two or more MAP messages. Both of these need to be
///simultaneously maintained (for efficiency) within memory.
class IntersectionList {
public:
    IntersectionList();

    virtual ~IntersectionList();

    ///Wrapper for Intersection.LoadMap.  Maintains a list of all MAPped Intersections. Returns a Pair containing
    ///the intersection object resulting from the MapDataMessage, and true if the map was loaded new and false if it was unchanged data.
    std::pair<Intersection *, bool> LoadMap(std::shared_ptr<MapData> msg);

private:

    ///Holds list of [IntersectionId, intersection object] for all encountered MAP files.
    std::unordered_map<int, Intersection> _intersections;
};

}}}} /* namespace tmx::plugin::utils::interxn */

#endif /* SRC_INTERSECTIONLIST_H_ */
