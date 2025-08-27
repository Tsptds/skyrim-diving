#pragma once
#include "Runtime.hpp"

namespace Hooks {

    class CameraHandler {
        public:
            static bool InstallCamStateHooks();

        private:
            struct TPP {
                    struct Install {
                            static bool Update();
                    };

                    struct Callback {
                            static void Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                    };
            };

            struct FPP {
                    struct Install {
                            static bool Update();
                    };

                    struct Callback {
                            static void Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState);
                    };

                    struct OG {
                            static inline REL::Relocation<decltype(Callback::Update)> _Update;
                    };
            };
    };
}  // namespace Hooks

// Install

bool Hooks::CameraHandler::InstallCamStateHooks() {
    bool res = false;

    res &= TPP::Install::Update();
    res &= FPP::Install::Update();

    return res;
}

bool Hooks::CameraHandler::TPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_ThirdPersonState[0]};
    OG::_Update = vtbl.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        logger::error("TPP Update Hook Not Installed");
        return false;
    }
    return true;
}
bool Hooks::CameraHandler::FPP::Install::Update() {
    /* VTABLE 0 ->TesCameraState /  1 ->PlayerInputHandler */

    REL::Relocation<uintptr_t> vtbl{RE::VTABLE_FirstPersonState[0]};
    OG::_Update = vtbl.write_vfunc(0x3, &Callback::Update);

    if (!OG::_Update.address()) {
        logger::error("FPP Update Hook Not Installed");
        return false;
    }
    return true;
}

// Callbacks

void Hooks::CameraHandler::TPP::Callback::Update(RE::ThirdPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    plugin::Diving::Update();

    OG::_Update(a_this, a_nextState);
}
void Hooks::CameraHandler::FPP::Callback::Update(RE::FirstPersonState *a_this, RE::BSTSmartPointer<RE::TESCameraState> &a_nextState) {
    plugin::Diving::Update();

    OG::_Update(a_this, a_nextState);
}