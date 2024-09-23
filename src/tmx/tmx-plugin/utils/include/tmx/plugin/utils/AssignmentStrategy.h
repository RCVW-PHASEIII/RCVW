
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
 

#ifndef INCLUDE_ASSIGNMENTSTRATEGY_H_
#define INCLUDE_ASSIGNMENTSTRATEGY_H_

#ifndef CONSTEXPR
#if __cplusplus > 199711L
#define CONSTEXPR constexpr
#else
#ifdef __GNUC__
#define CONSTEXPR __attribute__ ((unused))
#else
#define CONSTEXPR
#endif /* __GNUC__ */
#endif /* __cplusplus > 199711L */
#endif

#if __cplusplus >= 201103L
#include <tmx/utils/Enum.hpp>
#endif /* __cplusplus >= 201103L */
		
	
namespace tmx {
	
		
namespace utils {
	
			
enum class AssignmentStrategy 
{
		Random = 0,
		RoundRobin = 1,
		ShortestQueue = 2,
		PriorityQueue = 3
};
		
static CONSTEXPR const char *ASSIGNMENTSTRATEGY_RANDOM_STRING = "Random";
static CONSTEXPR const char *ASSIGNMENTSTRATEGY_ROUNDROBIN_STRING = "RoundRobin";
static CONSTEXPR const char *ASSIGNMENTSTRATEGY_SHORTESTQUEUE_STRING = "ShortestQueue";
static CONSTEXPR const char *ASSIGNMENTSTRATEGY_PRIORITYQUEUE_STRING = "PriorityQueue";
		
static CONSTEXPR const char *ASSIGNMENTSTRATEGY_ALL_STRINGS[] = 
{
		ASSIGNMENTSTRATEGY_RANDOM_STRING,
		ASSIGNMENTSTRATEGY_ROUNDROBIN_STRING,
		ASSIGNMENTSTRATEGY_SHORTESTQUEUE_STRING,
		ASSIGNMENTSTRATEGY_PRIORITYQUEUE_STRING
};
		
		
} /* End namespace utils */
		
	
} /* End namespace tmx */
		

#if __cplusplus >= 201103L
namespace tmx {
		
template <tmx::utils::AssignmentStrategy V> struct EnumName<tmx::utils::AssignmentStrategy, V> {
	static constexpr const char *name = tmx::utils::ASSIGNMENTSTRATEGY_ALL_STRINGS[static_cast<size_t>(V)];
};
template <> struct EnumSequenceBuilder<tmx::utils::AssignmentStrategy> {
	typedef EnumSequence<tmx::utils::AssignmentStrategy,
		tmx::utils::AssignmentStrategy::Random,
		tmx::utils::AssignmentStrategy::RoundRobin,
		tmx::utils::AssignmentStrategy::ShortestQueue,
		tmx::utils::AssignmentStrategy::PriorityQueue> type;
};
	
} /* End namespace tmx */
#endif /* __cplusplus >= 201103L */ 
#endif /* INCLUDE_ASSIGNMENTSTRATEGY_H_ */
	