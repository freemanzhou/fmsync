#pragma once

#include <smart_ptr.hpp>

namespace FileMaker {

class Database;

typedef boost::shared_ptr<Database> DatabasePtr;

}