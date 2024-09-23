/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file Uuid.hpp
 *
 *  Created on: Aug 19, 2016
 *      @author: Ben Paselsky
 */

#ifndef UTILS_INCLUDE_TMX_PLUGIN_UTILS_UUID_HPP_
#define UTILS_INCLUDE_TMX_PLUGIN_UTILS_UUID_HPP_

#include <string>
#include <uuid/uuid.h>

namespace tmx {
namespace plugin {
namespace utils {

class Uuid {
public:
    inline static std::string NewGuid() {
        char str[37];
        uuid_t id;
        uuid_generate(id);
        uuid_unparse(id, str);
        return std::string(str);
    }
};

} /* End namespace utils */
} /* End namespace plugin */
} /* End namespace tmx */

#endif /* UTILS_INCLUDE_TMX_PLUGIN_UTILS_UUID_HPP_ */
