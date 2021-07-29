#pragma once
#include <memory>
#include <vector>
#include "state.h"
#include "specification.h"

namespace rfscss_spec {
    Specification parse_spec(std::shared_ptr<State> state, std::vector<char> input);
}