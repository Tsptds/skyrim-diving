#pragma once

struct RayCastResult {
        float distance = -1.0f;
        RE::COL_LAYER layer = RE::COL_LAYER::kUnidentified;
        RE::hkVector4 normalOut = RE::hkVector4(0, 0, 0, 0);
        bool didHit = false;

        RayCastResult() = default;

        RayCastResult(float d, RE::COL_LAYER l, const RE::hkVector4 &n, bool h)
            : distance(d), layer(l), normalOut(n), didHit(h) {}
};

RayCastResult RayCast(RE::Actor *actor, RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::COL_LAYER layerMask) {
    const auto &player = actor;

    RayCastResult result{};
    result.distance = maxDist;

    if (!player) {
        return result;
    }
    const auto cell = player->GetParentCell();
    if (!cell) {
        return result;
    }
    const auto bhkWorld = cell->GetbhkWorld();
    if (!bhkWorld) {
        return result;
    }

    RE::bhkPickData pickData;
    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    /* hkpCollidable.h, lower 4 bits: CollidesWith, higher 4 bits: BelongsTo */

    //static_cast<uint32_t>(COL_LAYER::kAnimStatic) & ~static_cast<uint32_t>(COL_LAYER::kDoorDetection)

    RE::CFilter cFilter;
    player->GetCollisionFilterInfo(cFilter);
    cFilter.SetCollisionLayer(static_cast<RE::COL_LAYER>(layerMask));
    pickData.rayInput.filterInfo = cFilter;
    // static_cast<RE::CFilter>(cFilter.filter | static_cast<uint32_t>(layerMask));

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        result.didHit = true;
        result.distance = maxDist * pickData.rayOutput.hitFraction;
        result.normalOut = pickData.rayOutput.normal;

        const RE::COL_LAYER layer = static_cast<RE::COL_LAYER>(pickData.rayOutput.rootCollidable->GetCollisionLayer());

        result.layer = layer;
    }

    return result;
}

bool UpdateDivingState(RE::Actor *player) {
    if (!player) {
        return false;
    }

    auto pos = player->GetPosition();
    float waterZ = -RE::NI_INFINITY;
    if (pos; auto *cell = player->GetParentCell()) {
        cell->GetWaterHeight(pos, waterZ);
    }
    else {
        //logger::info("Water pos error");
        return false;
    }

    //auto PlayerScale = (player->GetCharController()->actorHeight * 0.0084f); /* 1/120 */
    //if (!PlayerScale) {
    //    logger::info("No player scale");
    //    return false;
    //}

    if (waterZ == -RE::NI_INFINITY) {
        // no water here → definitely not diving
        //logger::info("set false");
        return false;
    }

    float gap = pos.z - waterZ;
    if (gap > 0.0f) {
        // precompute player height
        constexpr float playerH = 120.0f /** PlayerScale*/;
        constexpr float maxCheck = playerH * 2000.0f;
        constexpr RE::NiPoint3 rayDir{0, 0, -1};

        RayCastResult ray = RayCast(player, pos + RE::NiPoint3{0, 0, playerH}, rayDir, maxCheck, RE::COL_LAYER::kLOS);

        // require the ray actually hit something above the water AND that that
        // distance is greater than your gap, AND that hit surface is roughly flat
        if (ray.didHit && ray.distance > 0.0f && abs(gap + playerH + 50 /** PlayerScale*/) < abs(ray.distance) &&
            ray.normalOut.quad.m128_f32[2] > 0.8f) {
            return true;
        }
        //logger::info("hit {} - Water {}", hitDist, gap + playerH);
    }

    return false;
}
