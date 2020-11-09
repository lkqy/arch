#pragma once
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "knitter/utility/stringify.h"

namespace knitter {

template <typename... ArgsT>
void throw_re(const char* fname, const size_t line, ArgsT&&... args) {
    std::ostringstream oss;
    oss << "[" << fname << ":" << line << "] ";
    ostreamize(oss, std::forward<ArgsT>(args)...);
    throw std::runtime_error(oss.str());
}
}

#define KNITTER_THROW(...) knitter::throw_re(__FILE__, __LINE__, __VA_ARGS__);
