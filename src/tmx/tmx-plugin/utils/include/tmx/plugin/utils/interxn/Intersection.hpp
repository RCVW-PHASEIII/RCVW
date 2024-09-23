/*
 * Intersection.h
 *
 *  Created on: Apr 29, 2016
 *      Author: ivp
 */

#ifndef SRC_INTERSECTION_H_
#define SRC_INTERSECTION_H_

#include <tmx/common/TmxLogger.hpp>
#include <tmx/common/types/Any.hpp>
#include <tmx/message/j2735/202007/MapData.h>
#include <tmx/message/j2735/202007/SPAT.h>
#include <tmx/plugin/utils/Clock.hpp>
#include <tmx/plugin/utils/geo/GeoVector.hpp>
#include <tmx/plugin/utils/interxn/Region.hpp>
#include <tmx/plugin/utils/interxn/MapSupport.hpp>

#include <atomic>

//Forward declaration of unit test class required to friend it for testing.
namespace unit_test{
 class IntersectionTest_UpdateRegions_Test;
 class IntersectionTest_CheckRegionForVehicleLaneId_Test;
 class IntersectionTest_GetPointFromNodeOffset_Test;
}

namespace tmx {
namespace plugin {
namespace utils {
namespace interxn {

/**
 * Direction(path) of intended traversal through the intersection.
 */
enum class Direction
{
	Dir_Straight=0,
	Dir_Left,
	Dir_Right
};

enum class Signal
{
	Unknown=0,
	Green,
	Yellow,
	Red
};
/**
 * Intersections are mapped to 4 regions assigned clockwise to the ways to enter the intersection.
 * It does not matter which entry is assigned to 1 as long as they are clockwise.
 *          |  2  |
 *          |     |
 * -------------------------
 *    1     |     |     3
 * -------------------------
 *          |  4  |
 *          |     |
 *
 *Vehicles:
 * Lane Ids 10-19 are region1 , LaneIds 20-29 are region 2, LaneIds 30-39 are region 3,
 * Lane Ids 40-49 are region 4
 * Crosswalks:
 * Lane Ids 1 are region1 , LaneIds 2 are region 2, LaneIds 3 are region 3,
 * Lane Ids 4 are region 4
 * Curbs:
 * Lane Ids 110-119 are region1 , LaneIds 120-129 are region 2, LaneIds 130-139 are region 3,
 * Lane Ids 140-149 are region 4.
 * As a vehicle approaches an intersection, odd curb areas are to the left and even curb areas are on the right.
 *
 *
 * If a roadway is not a standard fourway, the region numbers are still assigned (in the MAP) as if it were a standard
 * fourway:
 *          |  2  |
 *          |     |
 * ----------------
 *    1     |     |
 * ----------------
 *          |  4  |
 *          |     |
 *  In this example, Region 3 will just be empty/not populated.
 */


#define QUERYINTERSECTIONMAPID "MapData.intersections.IntersectionGeometry.id.id"
class Intersection {
public:
	friend class unit_test::IntersectionTest_UpdateRegions_Test;//friend unit test for testing private members.
	friend class unit_test::IntersectionTest_CheckRegionForVehicleLaneId_Test;//friend unit test for testing private members.
	friend class unit_test::IntersectionTest_GetPointFromNodeOffset_Test;//friend unit test for testing private members.
	Intersection();
	virtual ~Intersection();

	///Compares the MAP message to the currently loaded version and updates all data/Regions accordingly if its new.
	///Returns true if the map was truly updated/new. Returns false if the map was not new and thus unchanged.
	bool LoadMap(std::shared_ptr<MapData> msg);

	bool UpdateSpat(std::shared_ptr<SPAT> msg);

	void ClearMap();

	void ClearSpat();

	///Returns the intersection id of the loaded/parsed MAP intersection.
	int GetMapId();
	/**
	 * Returns true if the spat message indicates that lane Id has a pedestrian.
	 */
	bool IsPedestrianReportedForConnectionId(int laneId);

	/**
	 * Compares the Intersection id of the passed in spat message to the object's active map intersection id.
	 */
	bool DoesSpatMatchMap(SPAT &msg);
	/**
	 * Some applications do certain actions for all cases except a red light. This simplified
	 * call examines the signal phase for the signalGroup provided and returns true for red
	 * and false for all other permutations. False also returned if unknown/undetermined.
	 * relevantFields is populated with the excerpts from the xml used to make the determination.
	 */

	Signal GetSignalForLocation(double lat, double lon);
	Signal GetSignalForSignalGroup(int signalGroupId);

	int GetSignalGroupForLocation(double lat, double lon);

	uint64_t GetMsTimeSinceEpoch();
	double TimeRemainingForLocation_Sec(double lat, double lon);
	double TimeRemainingForSignalGroup(int signalGroupId);

	bool IsSignalForGroupRedLight(SPAT &msg, int signalGroup);

	//bool LoadMap(ParsedMap& parsedMap);
	/**
	 * Returns a list of all the lane IDs that correspond to curbs in relevant region,
	 * based on the MAP data.
	 * @param region Integer of region of interest.
	 */
	std::list<int>  GetCurbLaneIdsForRegion(int region) const;
	/**
	 * Returns a list of all the lane IDs that correspond to crosswalks in relevant region,
	 * based on the MAP data.
	 * @param region Integer of region of interest.
	 */
	std::list<int> GetCrosswalkLaneIdsForRegion(int region) const;
	/**
	 * Checks for vehicle lane among the loaded regions.  Used for determining ingress region after
	 *current lane has been discovered.
	 *current @param laneId current lane of vehicle
	 */
	int GetRegionForVehicleLaneId(int laneId) const;
	int GetRegionForCrosswalkLaneId(int laneId) const;
	int GetRegionForCurbLaneId(int laneId) const;
	/*
	 * Determines which region number the vehicle is destined to exit from by using the region of approach, the
	 * intersection configuration rules, and the intended direction of travel.
	 * @param approachRegion region number vehicle is entering intersection from.
	 * @param direction Vehicle is either traveling left, straight, or right through the intersection.
	 */
	int FindEgressRegion(int approachRegion, Direction direction) const;

	int GetLaneIdForLocation(double lat, double lon);
	bool IsLocationStillInLane(double lat, double lon, int laneId);

	bool IsPointOnMap(double lat, double lon);

	double GetDistanceToIntersection(double lat, double lon);

	double GetCurrentGrade(double lat, double lon);

	double GetStoppingDistance(double lat, double lon, double speed, double mu, double reactionTime);

	int GetMapRevision(MapData &msg);

	MapMatchResult GetLaneForLocation(double lat, double lon);

	/*
	 * Emergency vehicles may use egress lane when approaching and intersection. This method returns the approach
	 * lane using vehicle heading to determine direction of travel in lane
	 */
	MapMatchResult GetApproachLaneForLocation(double lat, double lon, double heading);

	std::list<MapLane> GetMapLanes(MapData &msg);
	MapLane GetMapLane(MapData &msg, int laneId);


	std::list<LaneNode> GetLaneNodes(MapData &msg, int laneId, double totalXOffset, double totalYOffset);

	///Populated from MAP data with lanes for all regions.
	///Index 0 is a dummy region placeholder (TODO could be used for any 'leftovers' that don't follow our rules)
	std::vector<Region> Regions;
	ParsedMap Map;
	///Returns lat/longs of all points in a list
	std::string MapToString();

	std::shared_ptr<MapData> MapMessage;
	std::shared_ptr<SPAT> SpatMsg;
	uint64_t lastSpatTime{0};
	uint64_t lastMapUpdateAttemptTime{0};

	//Return MAP and SPAT loaded status
	bool IsMapLoaded();
	bool IsSpatLoaded();

private:
	/*
	 * Parses MAP data lanes into their respective 'Region' buckets based on the MAP intersection agreement.
	 * @param laneId MAP message lane identifier.
	 *
	 */
	int UpdateRegionForLaneId(int laneId);
	/*
	 * Iterates over all vehicle lanes in the region to see if the lane is within it.
	 * @param laneId lane that is being searched for in the region by lane identifier.
	 */
	int CheckRegionForVehicleLaneId(const Region& r, int laneId) const;
	int CheckRegionForCurbLaneId(const Region& r, int laneId) const;
	int CheckRegionForCrosswalkLaneId(const Region& r, int laneId) const;

	/**Updates region info based on updated map data
	 *
	 */
	void UpdateRegions();

	/**
	 * Determines which lanes are computed and then compares the reference lane id of the computed lane to the vehicle
	 * lane with the nodes already computed and assgins the nodes with the given computed offset to the computed lane node list
	 */
	void SetComputedLanes(std::list<MapLane> &lanes);
	/**
	 * Get the offset values for the computed lanes nodes
	 */
	std::list<LaneNode> GetOffsetNodeList(std::list<LaneNode> &nodes, geo::WGS84Point &laneNodeOffset);

	///Call after setting the reference point to begin building the bounding box for the MAP.
	void InitMapBoundaryBox();
	///Check point and update bounding box to include it.
	void UpdateMapBoundaryBox(geo::WGS84Point point);

	///This is the revision number from the MAP file that indicates if the contents have been updated
	///and should be reparsed, or if they are unchanged.
	int _mapVersion;
	///The Id for the intersection of the MAP file.
	int _intersectionId;

	bool _isMapLoaded;
	bool _isSpatLoaded;
};

}}}} // namespace tmx::plugin::utils::interxn

#endif /* SRC_INTERSECTION_H_ */
