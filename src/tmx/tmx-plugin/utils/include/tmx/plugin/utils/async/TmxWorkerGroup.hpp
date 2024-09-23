/*!
 * Copyright (c) 2017 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxWorkerGroup.hpp
 *
 *  Created on: May 5, 2017
 *      @author: Gregory M. Baumgardner
 */

#ifndef UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXWORKERGROUP_HPP_
#define UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXWORKERGROUP_HPP_

#include <tmx/platform.hpp>

#include <tmx/common/types/Map.hpp>
#include <tmx/common/types/Enum.hpp>
#include <tmx/message/TmxMessage.hpp>

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <thread>

#ifndef TMX_DEFAULT_WORKER_ASSIGNMENT_STRATEGY
#define TMX_DEFAULT_WORKER_ASSIGNMENT_STRATEGY Random
#endif

namespace tmx {
namespace plugin {
namespace utils {
namespace async {

template <typename _T>
std::size_t queue_size(_T const &item) {
    return item.size();
}

/*!
 * @enum TmxWorkerAssignmentStrategy
 * @brief An enumeration for the supported assignment strategies
 *
 * @param Random Select a worker randomly
 * @param RoundRobin Select a worker sequentially
 * @param ShortestQueue Select the worker with the shortest queue
 * @param LeastUtilized Select the worker that has been assigned the least
 */
enum class TmxWorkerAssigmentStrategy {
    Random = 0,
    RoundRobin = 1,
    ShortestQueue = 2,
    LeastUtilized = 3
};

/**
 * @brief A class that helps makes assignments to workers
 *
 * This template class defines how tasks are assigned to individual workers within
 * a collection. The collection must be stored elsewhere, and supplied to the
 * assign() operation. The number of groups is meant as a super category for the
 * assignment, with the identifier being a sub-category within the group. This
 * should not be confused with the number of available workers or threads. The
 * assign() operation will choose only out of the available workers, regardless
 * of the number of groups and identifier combinations.
 *
 * Aside from the assignment strategy, this class also recalls previous assignments
 * in order to ensure consistencies in assignments for the same groups and identifiers.
 * This cache is cleared for the group and identifier by using the unassign()
 * operation.
 *
 * @tparam _Tp A type to hold a unique worker identifier, defaults to an unsigned integer
 * @tparam _GroupSz The number of bits used in determining the assignment group, defaults to half of _Tp
 * @tparam _IdentifierSz The number of bits used in determining the identifier within the assignment group,
 * defaults to half of _Tp
 */
template <std::uint8_t _GroupSz, std::uint8_t _IdentifierSz>
class TmxWorkerGroup: public common::TmxTaskExecutor {
    typedef TmxWorkerGroup<_GroupSz, _IdentifierSz> self_type;
    typedef common::types::UInt<_GroupSz> _group_t;
    typedef common::types::UInt<_IdentifierSz> _id_t;
    typedef typename common::types::UInt<_GroupSz + _IdentifierSz>::value_type assign_t;

public:
	typedef typename _group_t::value_type group_type;
	typedef typename _id_t::value_type id_type;

    // The worker group supports only a maximum number of groups and identifiers in the group
	static constexpr std::size_t max_groups = 1 << _group_t::numBits;
	static constexpr std::size_t max_ids = 1 << _id_t::numBits;

    /*!
     * @brief Construct a worker group with the default assignment strategy
     */
	TmxWorkerGroup(): _strategy(TmxWorkerAssigmentStrategy::TMX_DEFAULT_WORKER_ASSIGNMENT_STRATEGY) {
		srand(time (NULL));

		// Initialize the queue assignments
		for (std::size_t i = 0; i < max_groups; i++)
			for (std::size_t j = 0; j < max_ids; j++)
                this->unassign(i, j);

        TLOG(INFO) << "Initialized " << common::type_short_name(*this)
                   << " supports up to " << max_groups
                   << " unique worker groups. Current system supports a hardware concurrency value of "
                   << std::thread::hardware_concurrency();
	}

    /*!
     * @brief Default destructor
     */
    ~TmxWorkerGroup() = default;

    // Remove copy constructor and assignment operator
    TmxWorkerGroup(self_type const &) = delete;
    self_type &operator=(self_type const &) = delete;

	/**
	 * @brief Choose a worker for the group and identifier.
	 *
	 * A group and identifier may optionally supplied to maintain a consistent worker
	 * assignment for that combination. This is to provide a way to multiple operations
	 * on related data local to a common thread for caching purposes. If no worker
	 * assignment currently exists, or group and identifier are set to zero, indicating
	 * existing worker assignment should be ignored, then the item is assigned to a
	 * worker based on the specified assignment strategy, either random (default),
	 * round-robin (one-at-a-time), shortest-queue or the least utilized worker.
	 *
	 * The worker type must have a size() operation to determine the length of the
	 * existing queue.
	 *
	 * @see set_strategy(std::string)
	 * @param begin An iterator to the first worker option
	 * @param end An iterator to the end of workers option
	 * @param group The group identifier, or 0 for no group
	 * @param id The unique identifier in the group, or 0 for no identifier
	 * @return The worker option to assign to
	 */
    template <class _Iter>
    auto &assign(_Iter begin, _Iter end, group_type group = 0, id_type id = 0) {
		std::size_t groupCnt = std::abs(std::distance(begin, end));
		assign_t worker = this->_assignments[group][id];

		// If no group and no id, then any existing thread assignment should be ignored
		if (worker == (assign_t)-1) {
			// No thread assignment.  Assign using assignment strategy
			switch (_strategy) {
			case TmxWorkerAssigmentStrategy::RoundRobin:
                worker = this->_next++;
                if (worker >= groupCnt) {
                    // Restart at the beginning
                    worker = 0;
                    this->_next = 1;
                }
				break;
			case TmxWorkerAssigmentStrategy::Random:
                worker = rand() % groupCnt;
				break;
			case TmxWorkerAssigmentStrategy::ShortestQueue:
                worker = 0;
				for (assign_t i = 1; i < groupCnt; i++) {
					if (queue_size(*(begin + i)) < queue_size(*(begin + worker)))
                        worker = i;
				}
				break;
			case TmxWorkerAssigmentStrategy::LeastUtilized:
                worker = 0;
                for (assign_t i = 1; i < groupCnt; i++) {
                    if (utilization(i) < utilization(worker))
                        worker = i;
                }
				break;
			}

            // Thread for 0 group and 0 id is always unknown
            if (group != 0 || id != 0)
			    this->_assignments[group][id] = worker;
		}

        // If something is wrong, set to the first thread
        if (worker < 0 || worker >= groupCnt)
            worker = 0;

        this->_counts[worker]++;

        return *(begin + worker);
	}

	/**
	 * @brief Unassign the worker assignment for the specified group and identifier
	 *
	 * If it is no longer necessary for the worker assignment to be fixed for
	 * the given combination of group and identifier, the worker assignment
	 * should be removed so the next attempt to assign may result in a different,
	 * and more balanced, result. If this is function never called, however, then
	 * the worker assignment for the group and identifier will be maintained
	 * indefinitely.
	 *
	 * @param group The group identifier
	 * @param id The unique identifier in the group
	 */
	void unassign(group_type group, id_type id) {
        this->_assignments[group][id] = (assign_t)-1;
	}

    /*!
     * @param group The group identifier
     * @param id The unique identifier in the group
     * @return The current assignment for the specified group and identifier
     */
    auto assignment(group_type group, id_type id) {
        return this->_assignments[group][id];
    }

    /*!
     * @param n The worker identifier
     * @return The percentage of total work assigned to the group
     */
    auto utilization(assign_t n) const {
        static common::types::Floatmax::value_type pct = 100.0;

        std::size_t totalWork = 0;
        for (auto const &work: this->_counts)
            totalWork += work.second;

        return this->_counts.count(n) && totalWork > 0 ? pct * this->_counts.at(n) / totalWork : 0.0;
    }

    /*!
     * @return The current assignment strategy
     */
    TmxWorkerAssigmentStrategy get_strategy() const noexcept {
        return this->_strategy;
    }

	/**
	 * @brief Sets the assignment strategy
	 * @param strategy The new strategy
	 */
	void set_strategy(TmxWorkerAssigmentStrategy strategy) noexcept {
        this->_strategy = strategy;
	}

private:
    // The work count for each worker
    typename common::types::Map<assign_t, std::atomic<std::size_t> >::value_type _counts;

    // The assignment cache for each group and identifier
    std::atomic<assign_t> _assignments[max_groups][max_ids];

    // The next assignment
    std::atomic<assign_t> _next { 0 };

    // The assignment strategy used
    TmxWorkerAssigmentStrategy _strategy;
};

} /* End namespace async */
} /* End namespace utils */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* UTILS_INCLUDE_TMX_PLUGIN_UTILS_ASYNC_TMXWORKERGROUP_HPP_ */
