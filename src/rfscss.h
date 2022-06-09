#pragma once
#include <memory>
#include <vector>
#include "state.h"
#include "specification.h"

namespace rfscss_spec {
    Specification parse_spec(std::unique_ptr<State>& state, std::string input);
}