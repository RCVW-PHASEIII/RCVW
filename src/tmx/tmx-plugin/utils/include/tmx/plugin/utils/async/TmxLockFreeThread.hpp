/*
 * LockFreeThread.h
 *
 *  Created on: May 5, 2017
 *      Author: gmb
 */

#ifndef SRC_LOCKFREETHREAD_H_
#define SRC_LOCKFREETHREAD_H_

#include <tmx/plugin/utils/async/TmxRunnable.hpp>

#include <atomic>
#include <boost/lockfree/spsc_queue.hpp>
#include <thread>

namespace tmx {
namespace plugin {
namespace utils {
namespace async {

/**
 * A class that uses single-producer and single-consumer lock-free queues in order for optimal throughput.  The
 * capacity of each queue can be set, but it defaults to 2K.
 */
template <typename InQueueT, typename OutQueueT = InQueueT, typename Capacity = boost::lockfree::capacity<20480> >
class LockFreeThread: public TmxRunnable {
    typedef InQueueT incoming_item;
    typedef OutQueueT outgoing_item;

public:
	LockFreeThread(): _inSize(0), _outSize(0) {}

	virtual ~LockFreeThread() {
        this->stop();
	}

	/**
	 * Insert into the incoming queue
	 *
	 * @param item The item to push into the queue
	 * @return True if the item was inserted
	 */
	bool push(const incoming_item &item) {
		if (_inQ.push(item)) {
			_inSize++;
			return true;
		}

		return false;
	}

	/**
	 * Insert directly into the outgoing queue.
	 *
	 * @param item The item to push on the queue
	 * @ret True if the item was inserted
	 */
	bool push_out(const outgoing_item &item) {
		if (_outQ.push(item)) {
			_outSize++;
			return true;
		}

		return false;
	}

	/**
	 * Remove the next item from the outgoing queue
	 *
	 * @param item The item from the queue, or undefined if nothing exists
	 * @return True if the item is defined
	 */
	bool pop(outgoing_item &item) {
		if (_outQ.pop(item)) {
			_outSize--;
			return true;
		}

		return false;
	}

	/**
	 * @return The size of the incoming queue
	 */
	uint64_t in_queue_size() {
		uint64_t ret = _inSize;
		return ret;
	}

	/**
	 * @return The size of the outgoing queue
	 */
	uint64_t out_queue_size() {
		uint64_t ret = _outSize;
		return ret;
	}

	/**
	 * Obtain the id of this thread
	 *
	 * @return The identifier object for this thread
	 */
	std::thread::id get_id() {
		if (_thread)
			return _thread->get_id();
		else
			return std::this_thread::get_id();
	}

	/**
	 * @return True if the thread is joinable
	 */
	bool joinable() {
		return (_thread ? _thread->joinable() : false);
	}

	/**
	 * Join the executing thread, if possible
	 */
	void join() {
		if (joinable())
			_thread->join();
	}
protected:
	/**
	 * The function that processes the items from the incoming queue.  It may or may not
	 * write to the outgoing queue.
	 */
	virtual void doWork(incoming_item &item) = 0;

	/**
	 * A function that idlles the processor when there is nothing to process
	 */
	virtual void idle() = 0;
private:
	void process() {
		while (_active) {
			incoming_item item;
			if (_inQ.pop(item)) {
				doWork(item);
				_inSize--;
			} else {
				idle();
			}
		}
	}

	// The lock free queues, both in and out
	boost::lockfree::spsc_queue<incoming_item, Capacity> _inQ;
	boost::lockfree::spsc_queue<outgoing_item, Capacity> _outQ;
	std::atomic<uint64_t> _inSize;
	std::atomic<uint64_t> _outSize;
};

} /* End namespace async */
} /* End namespace utils */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* SRC_LOCKFREETHREAD_H_ */
