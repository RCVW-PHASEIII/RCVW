/*!
 * Copyright (c) 2021 Battelle Memorial Institute
 *
 * All Rights Reserved.
 *
 * @file TmxDataAccessObject.cpp
 *
 *  Created on: Mar 9, 2023
 *      @author: Gregory M. Baumgardner
 */

#include <tmx/plugin/dao/TmxDataAccessObject.hpp>

using namespace tmx::common;
using namespace tmx::common::types;

namespace tmx {
namespace plugin {
namespace dao {

TmxDataAccessObject::TmxDataAccessObject(): _data() { }

TmxDataAccessObject::TmxDataAccessObject(Any &obj): _data(obj) { }

TmxDataAccessObject::TmxDataAccessObject(Any const &obj): _data(obj) { }

TmxDataAccessObject::TmxDataAccessObject(TmxDataAccessObject const &copy): _data(copy._data) { }

} /* End namespace dao */
} /* End namespace plugin */
} /* End namespace tmx */