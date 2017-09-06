#ifndef GAME_DEBUG_SCREEN_H
#define GAME_DEBUG_SCREEN_H

extern "C" __declspec(dllexport) GAME_RENDER_FUNC(render);
extern "C" __declspec(dllexport) GAME_UPDATE_FUNC(update);
extern "C" __declspec (dllexport) GAME_INIT_FUNC(init);
extern "C" __declspec (dllexport) GAME_DEBUG_FUNC(clean_up);

void *global_active_variable = nullptr;
void *global_hot_variable  = nullptr;
int32 interacting_with_variable = 0;
member_type global_active_variable_type;

#endif GAME_DEBUG_SCREEN_H
