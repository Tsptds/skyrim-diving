#pragma once

float RayCast(RE::NiPoint3 rayStart, RE::NiPoint3 rayDir, float maxDist, RE::hkVector4 &normalOut, RE::COL_LAYER layerMask) {
    const auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if player is null
    }
    const auto cell = player->GetParentCell();
    if (!cell) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if cell is unavailable
    }
    const auto bhkWorld = cell->GetbhkWorld();
    if (!bhkWorld) {
        normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
        return maxDist;  // Return maxDist if Havok world is unavailable
    }

    RE::bhkPickData pickData;
    const auto havokWorldScale = RE::bhkWorld::GetWorldScale();

    // Set ray start and end points (scaled to Havok world)
    pickData.rayInput.from = rayStart * havokWorldScale;
    pickData.rayInput.to = (rayStart + rayDir * maxDist) * havokWorldScale;

    // Set the collision filter info to exclude the player
    uint32_t collisionFilterInfo = 0;
    player->GetCollisionFilterInfo(collisionFilterInfo);
    pickData.rayInput.filterInfo = (collisionFilterInfo & 0xFFFF0000) | static_cast<uint32_t>(layerMask);

    // Perform the raycast
    if (bhkWorld->PickObject(pickData) && pickData.rayOutput.HasHit()) {
        normalOut = pickData.rayOutput.normal;

        const uint32_t layerIndex = pickData.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F;

        if (layerIndex == 0) {
            return -1.0f;  // Invalid layer hit
        }

        // Optionally log the layer hit
        // if (logLayer) logger::info("\nLayer hit: {}", layerIndex);

        // Check for useful collision layers
        switch (static_cast<RE::COL_LAYER>(layerIndex)) {
            case RE::COL_LAYER::kStatic:
            case RE::COL_LAYER::kCollisionBox:
            case RE::COL_LAYER::kTerrain:
            case RE::COL_LAYER::kGround:
            case RE::COL_LAYER::kProps:
            case RE::COL_LAYER::kDoorDetection:
            case RE::COL_LAYER::kTrees:
            case RE::COL_LAYER::kClutterLarge:
            case RE::COL_LAYER::kAnimStatic:
            case RE::COL_LAYER::kDebrisLarge:
                // Update last hit object type
                return maxDist * pickData.rayOutput.hitFraction;

            default:
                return -1.0f;  // Ignore unwanted layers
        }
    }

    // No hit
    normalOut = RE::hkVector4(0.0f, 0.0f, 0.0f, 0.0f);
    // if (logLayer) logger::info("Nothing hit");

    return maxDist;
}

RE::NiPoint3 GetPlayerDirFlat(RE::Actor *player) {
    // Calculate player forward direction (normalized)
    const float playerYaw = player->data.angle.z;  // Player's yaw
    //_THREAD_POOL
    RE::NiPoint3 playerDirFlat{std::sin(playerYaw), std::cos(playerYaw), 0};
    const float dirMagnitude = std::hypot(playerDirFlat.x, playerDirFlat.y);
    playerDirFlat.x /= dirMagnitude;
    playerDirFlat.y /= dirMagnitude;

    return playerDirFlat;
}

bool UpdateDivingState() {
    auto *player = RE::PlayerCharacter::GetSingleton();
    if (!player) {
        //logger::info("No player error");
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
        player->SetGraphVariableInt("bSimpleDiving_IsDiving", 0);
        //logger::info("set false");
        return true;
    }

    float gap = pos.z - waterZ;
    int dive = 0;
    if (gap > 0.0f) {
        // precompute player height
        float playerH = 120.0f /** PlayerScale*/;
        RE::hkVector4 normal;
        auto facing = GetPlayerDirFlat(player);

        float hitDist = RayCast(pos + RE::NiPoint3{facing.x + 10, facing.y + 10, playerH}, RE::NiPoint3{0, 0, -1}, playerH * 10.0f, normal,
                                RE::COL_LAYER::kLOS);

        // require the ray actually hit something above the water AND that that
        // distance is greater than your gap, AND that hit surface is roughly flat
        if (hitDist > 0.0f && abs(gap + playerH + 50 /** PlayerScale*/) < abs(hitDist) && normal.quad.m128_f32[2] > 0.8f) {
            dive = 1;
        }
        //logger::info("hit {} - Water {}", hitDist, gap + playerH);
    }

    player->SetGraphVariableInt("bSimpleDiving_IsDiving", dive);
    //logger::info("set {}", dive);
    return true;
}