#pragma once

#include <string>
#include <string_view>

namespace mse {

// Porter stemmer (English), self-contained.
std::string porter_stem(std::string_view word);

} // namespace mse
