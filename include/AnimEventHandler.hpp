#include "Runtime.hpp"
namespace Hooks {
    std::unordered_set<std::string_view> Events{"JumpFall", "JumpFallDirectional", "JumpUp", "JumpLand", "JumpLandEnd"};

    template <class T>
    class AnimationEventHook : public T {
        public:
            using Fn_t = decltype(&T::ProcessEvent);
            static inline REL::Relocation<Fn_t> _ProcessEvent;  // 01
            inline RE::BSEventNotifyControl Hook(const RE::BSAnimationGraphEvent* a_event,
                                                 RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
                if (a_event) {
                    auto actor = a_event->holder;
                    if (actor && actor->IsPlayerRef()) {
                        /* TODO: SkyParkour compat, check graph var if ledge is not -1 don't set diving true */
                        //logger::info("{}.{}", a_event->tag, a_event->payload);

                        if (Events.contains(a_event->tag)) {
                            plugin::Funcs::Update();
                        }
                    }
                }
                return _ProcessEvent(this, a_event, a_eventSource);
            }
            static void InstallAnimEventHook() {
                // Hooking the vfunc directly
                auto vtbl = REL::Relocation<std::uintptr_t>(RE::VTABLE_BSAnimationGraphManager[0]);
                constexpr std::size_t idx = 0x1;
                _ProcessEvent = vtbl.write_vfunc(idx, &Hook);
                logger::info(">> AnimEvent Hook Installed");
            }
    };
}  // namespace Hooks