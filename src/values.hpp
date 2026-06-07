#pragma once

#include <vector>
#include "common.hpp"

using Value = double;

struct ValueArray {
    std::vector<Value> values;
};

void writeValueArray(ValueArray& array, Value value);
void printValue(Value value);
