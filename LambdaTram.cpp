
#include "LambdaTram.hpp"

unordered_map<int, std::any> LambdaTram::map_;
std::mutex LambdaTram::map_mutex_;