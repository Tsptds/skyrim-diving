// Conditions.h
#pragma once
#include "OpenAnimationReplacerAPI-Conditions.h"
#include "Runtime.hpp"

namespace Conditions {
    class IsDivingCondition : public CustomCondition {
        public:
            static constexpr std::string_view CONDITION_NAME = "IsDiving"sv;

            RE::BSString GetName() const override {
                return CONDITION_NAME.data();
            }
            RE::BSString GetDescription() const override {
                return "True when player is falling into water."sv.data();
            }
            constexpr REL::Version GetRequiredVersion() const override {
                return {1, 0, 0};
            }

        protected:
            bool EvaluateImpl(RE::TESObjectREFR *, RE::hkbClipGenerator *, void *) const override{
                // your external check
                return plugin::Diving::Update();
            }
    };
}  // namespace Conditions
