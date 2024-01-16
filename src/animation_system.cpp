#include <algorithm>    // std::sort

// internal
#include "render_system.hpp"
#include "common.hpp"
#include <SDL.h>
#include "tiny_ecs_registry.hpp"
#include "components.hpp"

void RenderSystem::initAnimation(GEOMETRY_BUFFER_ID gid, ANIMATION_FRAME_COUNT fcount) {
    std::vector<TexturedVertex> textured_spritesheet_vertices(4);
    //from {-1,-1} to {1,1}
    textured_spritesheet_vertices[0].position = { -1.f / 2, +1.f / 2, 0.f };
    textured_spritesheet_vertices[1].position = { +1.f / 2, +1.f / 2, 0.f };
    textured_spritesheet_vertices[2].position = { +1.f / 2, -1.f / 2, 0.f };
    textured_spritesheet_vertices[3].position = { -1.f / 2, -1.f / 2, 0.f };
    //from {0,0} to {1,1}
    float fcount_f = (int)fcount * 1.f;
    textured_spritesheet_vertices[0].texcoord = { 0.f / fcount_f, 0.f };
    textured_spritesheet_vertices[1].texcoord = { 1.f / fcount_f, 0.f };
    textured_spritesheet_vertices[2].texcoord = { 1.f / fcount_f, 1.f };
    textured_spritesheet_vertices[3].texcoord = { 0.f / fcount_f, 1.f };

    const std::vector<uint16_t> textured_spritesheet_indices = { 0, 3, 1, 1, 3, 2 };
    bindVBOandIBO(gid, textured_spritesheet_vertices, textured_spritesheet_indices);
}

void RenderSystem::initAnimation_dashing() {
    
    std::vector<TexturedVertex> textured_spritesheet_vertices(4);
    //from {-1,-1} to {1,1}
    textured_spritesheet_vertices[0].position = { - 1.f, 1.f / 4.f, 0.f };
    textured_spritesheet_vertices[1].position = { -1.f / 2.f, 1.f / 4.f, 0.f };
    textured_spritesheet_vertices[2].position = { -1.f/2.f, -1.f/4.f, 0.f };
    textured_spritesheet_vertices[3].position = { -1.f, -1.f/4.f, 0.f };
    //from {0,0} to {1,1}
    int num_frame = (int)ANIMATION_FRAME_COUNT::DASHING;

    textured_spritesheet_vertices[0].texcoord = { 0.f / num_frame, 0.f };
    textured_spritesheet_vertices[1].texcoord = { 1.f / num_frame, 0.f };
    textured_spritesheet_vertices[2].texcoord = { 1.f / num_frame, 1.f };
    textured_spritesheet_vertices[3].texcoord = { 0.f / num_frame, 1.f };


    const std::vector<uint16_t> textured_spritesheet_indices = { 0, 3, 1, 1, 3, 2 };
    bindVBOandIBO(GEOMETRY_BUFFER_ID::SPRITESHEET_DASHING, textured_spritesheet_vertices, textured_spritesheet_indices);
}

void RenderSystem::animationSys_init() {
    for (auto& spriteSheet : animation_frame_count_map) {
        RenderSystem::initAnimation(spriteSheet.first, spriteSheet.second);
    }
    RenderSystem::initAnimation_dashing();
}

void RenderSystem::animationSys_step(float elapsed_ms) {
    
    for (uint i = 0; i < registry.animations.components.size(); i++) {
        Animation& animation = registry.animations.components[i];
        
        if(!animation.pause_animation){
            animation.timer_ms += elapsed_ms;

            if (animation.loop_interval > 0 && (int)(animation.timer_ms / animation.update_period_ms) >= animation.total_frame) {
                animation.curr_frame = 0;
                if (animation.timer_ms >= animation.loop_interval) {
                    animation.timer_ms = 0.f;
                }
            }
            else {
                animation.curr_frame = (int)(animation.timer_ms / animation.update_period_ms) % animation.total_frame;
            }


        }
        else {
            animation.timer_ms = 0;
            animation.curr_frame = 0;
        }
    }
}

void RenderSystem::animationSys_switchAnimation(Entity& entity, 
    ANIMATION_FRAME_COUNT animationType,
    int update_period_ms) {

    TEXTURE_ASSET_ID tid = animation_texture_map_general.at(animationType);
    GEOMETRY_BUFFER_ID gid = animation_geo_map_general.at(animationType);
    if (tid != TEXTURE_ASSET_ID::EMPTY && gid != GEOMETRY_BUFFER_ID::EMPTY) {
        RenderRequest& rr = registry.renderRequests.get(entity);
        rr.used_geometry = gid;
        rr.used_texture = tid;
        Animation& animation = registry.animations.get(entity);
        animation.update_period_ms = update_period_ms;
        animation.total_frame = (int)animationType;
        animation.curr_frame = 0;
        animation.pause_animation = false;
        animation.timer_ms = 0.f;
    }
}