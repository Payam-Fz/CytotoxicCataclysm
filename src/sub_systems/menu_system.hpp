#pragma once
#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

#include <random>

class MenuSystem {
public:
    MenuSystem (const vec2& mouse);
    MENU_OPTION poll_start_menu();
    MENU_OPTION poll_pause_menu();
    vec2 recent_click_coord;
    ~MenuSystem();

private:
    bool assets_drawn;
    const vec2& curr_mouse;
    bool is_muted;
    bool prev_limit_fov;
    void draw_start_menu();
    void draw_pause_menu();
    Entity create_menu_button(vec2 pos, TEXTURE_ASSET_ID texture, MENU_OPTION option);
    Entity create_menu_bg();
    bool check_button_click(Entity entity);
    void destroy_menu();
};