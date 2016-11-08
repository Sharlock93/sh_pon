#include <game_logic.h>
/* #include <game_debug_screen.h> */

extern "C" __declspec(dllexport) GAME_RENDER_FUNC(render) { 
    printf("hello world");
}

 extern "C" __declspec(dllexport) GAME_UPDATE_FUNC(update) {
    printf("hello from update");
}

extern "C" __declspec(dllexport) GAME_INIT_FUNC(init) {
    printf("init");
}
