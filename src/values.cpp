#include "values.hpp"
#include "memory.h"

void writeValueArray(ValueArray& array, Value value)
{
    array.values.push_back(value);
}

void printValue(Value value) {
 std::cout << value;
}
