#pragma once
#include "Util.hpp"

namespace plugin {

    class Vars {
        public:
            static void SetValid(bool _shouldUpdate) {
                ShouldUpdate = _shouldUpdate;
                if (!Ongoing) {
                    Update();
                }
            }

            static void Update() {
                if (ShouldUpdate) {
                    Ongoing = true;
                    /*bool diving = */UpdateDivingState();
                    //logger::info("Diving Set: {}", diving);

                    _THREAD_POOL.enqueue([]() {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        //logger::info("Updating...");
                        SKSE::GetTaskInterface()->AddTask([] { Update(); });
                    });
                }
                else {
                    Ongoing = false;
                }
            }

            Vars() = default;
            ~Vars() = default;

        private:
            inline static bool ShouldUpdate = false;
            inline static bool Ongoing = false;
    };

}  // namespace plugin