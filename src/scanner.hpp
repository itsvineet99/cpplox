#pragma once

#include "common.hpp"

class Scanner
{
public:
    explicit Scanner(const std::string& source);

private:
    const std::string& source;

    std::size_t start = 0;
    std::size_t current = 0;
    int line = 1;
};
