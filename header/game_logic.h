#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H
#include <shar.h>

//Note(sharo): typedefs
typedef float real32;
typedef int64_t int64;
typedef uint32_t uint32;

enum {
    PADDLE,
    MAIN_BALL
};

#define ArrayCount(array) (sizeof(array))/((sizeof(array[0])))


extern "C" __declspec(dllexport) GAME_RENDER_FUNC(render);
extern "C" __declspec(dllexport) GAME_UPDATE_FUNC(update);
extern "C" __declspec(dllexport) GAME_INIT_FUNC(init);
extern "C" __declspec(dllexport) GAME_DEBUG_FUNC(debug_func);

int gen_id(); //@Note(sharo): this will just generate an id
void update_object_grid_relation(game_grid *grid, game_object *object);
int collision(grid_element *elem, game_object *object);
#endif
