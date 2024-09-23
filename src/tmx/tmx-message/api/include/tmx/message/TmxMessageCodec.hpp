/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxMessageCodec.h
 *
 *  Created on: Jun 18, 2021
 *      @author: Gregory M. Baumgardner
 */

#ifndef INCLUDE_TMX_MESSAGE_TMXMESSAGECODEC_HPP_
#define INCLUDE_TMX_MESSAGE_TMXMESSAGECODEC_HPP_

#include <tmx/common/types/bytes_type.hpp>
#include <tmx/message/TmxMessage.hpp>
#include <string>

namespace tmx {
namespace message {

/*!
 * @brief An abstract class for encoding and decoding objects with TMX.
 */
class TmxMessageCodec {
	using bytes = tmx::common::types::bytes_type<>;

public:
	// No copy constructor
	TmxMessageCodec(const TmxMessageCodec &other) = delete;
	virtual ~TmxMessageCodec();

	// No assignment operator
	TmxMessageCodec &operator=(const TmxMessageCodec &other) = delete;

	/*!
	 * @return The name of the codec
	 */
	std::string_view get_name() const noexcept;


	static bytes encode(tmx::message::TmxMessage message);
	static tmx::message::TmxMessage decode(bytes data);

protected:
	TmxMessageCodec(std::string_view name);

	/*!
	 * @brief Get the codec by name.
	 *
	 * @param[in] name The name of the codec to get
	 * @return A pointer to the codec implementation, if it exists, or null otherwise
	 *
	static TmxMessageCodec *get_codec(std::string_view name) const noexcept;
*/
	/*!
	 * @brief Encode a message object as a sequence of bytes, according to the codec
	 *
	 * Each codec implementation will create a unique set of
	 *
	virtual tmx::support::types::byte_string_t encode(const void *message) = 0;
	virtual void *decode(tmx::support::types::byte_string_t bytes);
*/
	std::string _name;
};

} /* End namespace message */
} /* End namespace tmx */

#endif /* INCLUDE_TMX_MESSAGE_TMXMESSAGECODEC_HPP_ */
