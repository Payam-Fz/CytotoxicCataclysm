#include "menu_system.hpp"

MenuSystem::MenuSystem(const vec2& mouse)
    : curr_mouse(mouse) {
    assets_drawn = false;
    // Initialize with an impossible coordinate
    recent_click_coord = {1.0e9f, 1.0e9f};
    is_muted = false;
    prev_limit_fov = false;
}

MenuSystem::~MenuSystem(){
    destroy_menu();
}

MENU_OPTION MenuSystem::poll_start_menu() {
    if (!assets_drawn) {
        draw_start_menu();
        assets_drawn = true;
    }

    MENU_OPTION selected_option = MENU_OPTION::NONE;
    Entity entity;
    for (uint i = 0; i < registry.menuButtons.size(); i++) {
        entity = registry.menuButtons.entities[i];
        if (check_button_click(entity)) {
            selected_option = registry.menuButtons.get(entity).option;
            break;
        }
    }

    
    if (selected_option != MENU_OPTION::NONE 
        && selected_option != MENU_OPTION::REGULAR_GAME_MODE 
        && selected_option != MENU_OPTION::EASY_GAME_MODE) {// destroy all menu elements after a choice is made by the user
        destroy_menu();
    } else if (selected_option == MENU_OPTION::REGULAR_GAME_MODE) {
        registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::MENU_REGULAR_MODE_SELECTED;
        for (uint i = 0; i < registry.menuButtons.size(); i++) {
            Entity other_entity = registry.menuButtons.entities[i];
            if (registry.menuButtons.get(other_entity).option == MENU_OPTION::EASY_GAME_MODE) {
                registry.renderRequests.get(other_entity).used_texture = TEXTURE_ASSET_ID::MENU_EASY_MODE_UNSELECTED;
            }
        }
    } else if (selected_option == MENU_OPTION::EASY_GAME_MODE) {
        registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::MENU_EASY_MODE_SELECTED;
        for (uint i = 0; i < registry.menuButtons.size(); i++) {
            Entity other_entity = registry.menuButtons.entities[i];
            if (registry.menuButtons.get(other_entity).option == MENU_OPTION::REGULAR_GAME_MODE) {
                registry.renderRequests.get(other_entity).used_texture = TEXTURE_ASSET_ID::MENU_REGULAR_MODE_UNSELECTED;
            }
        }
    }

    return selected_option;
}

MENU_OPTION MenuSystem::poll_pause_menu() {
    if (!assets_drawn) {
        draw_pause_menu();
        assets_drawn = true;
    }
    
    MENU_OPTION selected_option = MENU_OPTION::NONE;
    Entity entity;
    for (uint i = 0; i < registry.menuButtons.size(); i++) {
        entity = registry.menuButtons.entities[i];
        if (check_button_click(entity)) {
            selected_option = registry.menuButtons.get(entity).option;
            break;
        }
    }

    // Destroy all menu elements if the user choose to resume or exit
    // Keep displaying the menu of the user choose to save or mute/unmute sound
    if (selected_option == MENU_OPTION::RESUME_GAME) {
        if (prev_limit_fov) {
            registry.screenStates.components[0].limit_fov = true;
            prev_limit_fov = false;
        }
        destroy_menu();
    } else if (selected_option == MENU_OPTION::EXIT_CURR_PLAY) {
        destroy_menu();
    } else if (selected_option == MENU_OPTION::MUTE_SOUND) {
        // If unmute/mute is selected, change the menu button to the opposite
        is_muted = true;
        registry.menuButtons.get(entity).option = MENU_OPTION::UNMUTE_SOUND;
        registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::MENU_UNMUTE;
    } else if (selected_option == MENU_OPTION::UNMUTE_SOUND) {
        is_muted = false;
        registry.menuButtons.get(entity).option = MENU_OPTION::MUTE_SOUND;
        registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::MENU_MUTE;
    }

    return selected_option;
}

Entity MenuSystem::create_menu_button(vec2 pos, TEXTURE_ASSET_ID texture, MENU_OPTION option) {
    auto entity = Entity();
    Transform& transform = registry.transforms.emplace(entity);
    transform.position = pos;
    transform.scale = MENU_BUTTON_TEXTURE_SIZE * 0.5f;
    transform.is_screen_coord = true;

    registry.renderRequests.insert(
		entity,
		{ texture,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::MENU });

    registry.menuElems.emplace(entity);
    MenuButton buttom_comp = {option, false};
    registry.menuButtons.insert(entity, buttom_comp);
    return entity;
}

Entity MenuSystem::create_menu_bg() {
    auto bg_entity = Entity();
    Transform& bg_transform = registry.transforms.emplace(bg_entity);
    bg_transform.position = {0.f, 0.f};
    bg_transform.scale = {CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX};
    bg_transform.is_screen_coord = true;

    // Randomly select a bg to use for menu
    std::srand(std::time(nullptr));
    TEXTURE_ASSET_ID random_bg = static_cast<TEXTURE_ASSET_ID>(static_cast<int>(TEXTURE_ASSET_ID::NERVOUS_BG) + std::rand() % 6);

    registry.renderRequests.insert(
		bg_entity,
		{ random_bg,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::MENU_BACKGROUND,
		  RENDER_ORDER::MENU });

    registry.menuElems.emplace(bg_entity);
    return bg_entity;
}

void MenuSystem::draw_start_menu() {
    // Create background
    create_menu_bg();

    // Create title
    auto title_entity = Entity();
    Transform& title_transform = registry.transforms.emplace(title_entity);
    title_transform.position = {0.f, 320.f};
    title_transform.scale = START_TITLE_TEXTURE_SIZE * 0.4f;
    title_transform.is_screen_coord = true;

    registry.renderRequests.insert(
		title_entity,
		{ TEXTURE_ASSET_ID::START_MENU_TITLE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::MENU });

    registry.menuElems.emplace(title_entity);

    // Create buttons
    create_menu_button({0.f, 50.f}, TEXTURE_ASSET_ID::MENU_START, MENU_OPTION::START_GAME);
    create_menu_button({0.f, -150.f}, TEXTURE_ASSET_ID::MENU_LOAD, MENU_OPTION::LOAD_GAME);
    create_menu_button({0.f, -350.f}, TEXTURE_ASSET_ID::MENU_EXIT, MENU_OPTION::EXIT_GAME);
    if (registry.gameMode.components.back().id == GAME_MODE_ID::EASY_MODE) {
        create_menu_button({ -550.f, -150.f }, TEXTURE_ASSET_ID::MENU_EASY_MODE_SELECTED, MENU_OPTION::EASY_GAME_MODE);
        create_menu_button({ -550.f, -350.f }, TEXTURE_ASSET_ID::MENU_REGULAR_MODE_UNSELECTED, MENU_OPTION::REGULAR_GAME_MODE);
    } else {
        create_menu_button({ -550.f, -150.f }, TEXTURE_ASSET_ID::MENU_EASY_MODE_UNSELECTED, MENU_OPTION::EASY_GAME_MODE);
        create_menu_button({ -550.f, -350.f }, TEXTURE_ASSET_ID::MENU_REGULAR_MODE_SELECTED, MENU_OPTION::REGULAR_GAME_MODE);
    }
}

void MenuSystem::draw_pause_menu() {
    // If the fov is limited at the time of pausing, revert that temporarily to show the menu normally
    if (registry.screenStates.components[0].limit_fov == true) {
        prev_limit_fov = true;
        registry.screenStates.components[0].limit_fov = false;
    }

    // Create background
    create_menu_bg();

    // Create title (Game Paused)
    auto title_entity = Entity();
    Transform& title_transform = registry.transforms.emplace(title_entity);
    title_transform.position = {0.f, 400.f};
    title_transform.scale = PAUSE_TITLE_TEXTURE_SIZE;
    title_transform.is_screen_coord = true;

    registry.renderRequests.insert(
		title_entity,
		{ TEXTURE_ASSET_ID::PAUSE_MENU_TITLE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::MENU });

    registry.menuElems.emplace(title_entity);

    // Create buttons
    create_menu_button({0.f, 200.f}, TEXTURE_ASSET_ID::MENU_RESUME, MENU_OPTION::RESUME_GAME);
    create_menu_button({0.f, 0.f}, TEXTURE_ASSET_ID::MENU_SAVE, MENU_OPTION::SAVE_GAME);
    if (is_muted) {
        create_menu_button({0.f, -200.f}, TEXTURE_ASSET_ID::MENU_UNMUTE, MENU_OPTION::UNMUTE_SOUND);
    } else {
        create_menu_button({0.f, -200.f}, TEXTURE_ASSET_ID::MENU_MUTE, MENU_OPTION::MUTE_SOUND);
    }
    create_menu_button({0.f, -400.f}, TEXTURE_ASSET_ID::MENU_EXIT, MENU_OPTION::EXIT_CURR_PLAY);
}

bool MenuSystem::check_button_click(Entity entity) {
    Transform transform = registry.transforms.get(entity);
    vec2 bottom_right = transform.position + abs(transform.scale) / 2.f;
    vec2 top_left = transform.position - abs(transform.scale) / 2.f;
    if (curr_mouse.x > top_left.x && curr_mouse.x < bottom_right.x &&
        curr_mouse.y > top_left.y && curr_mouse.y < bottom_right.y) {
        registry.menuButtons.get(entity).highlight = true;
        if (recent_click_coord.x > top_left.x && recent_click_coord.x < bottom_right.x &&
                recent_click_coord.y > top_left.y && recent_click_coord.y < bottom_right.y) {
            // Set to an impossible value to avoid double-consuming
            recent_click_coord = {1.0e9f, 1.0e9f};
            return true;
        }
    } else {
        registry.menuButtons.get(entity).highlight = false;
    }
    return false;
}

void MenuSystem::destroy_menu() {
    while (registry.menuElems.entities.size() > 0)
		registry.remove_all_components_of(registry.menuElems.entities.back());
    assets_drawn = false;
}