#pragma once
#include "Util.hpp"

namespace plugin {

    class Funcs {
        public:
            static void Update() {
                /*bool diving = */ UpdateDivingState();
                //logger::info("Diving Set: {}", diving);
            }

            Funcs() = default;
            ~Funcs() = default;
    };

}  // namespace plugin