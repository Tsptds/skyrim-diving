#pragma once
#include "Util.hpp"

namespace plugin {

    class Diving {
        public:
            static void Update() {
                bool res = UpdateDivingState(player);
                logger::info("Diving >> {}", res ? "Yes" : "No");
                player->SetGraphVariableInt("bSimpleDiving_IsDiving", res);
                // later: use *dirPtr after the thread finishes (sync if needed)
            }

            Diving() = default;
            ~Diving() = default;

            static void SetPlayerRef(RE::Actor *in) {
                player = in;
            }
            /* Get This Once */
            inline static RE::Actor *player;
    };

}  // namespace plugin