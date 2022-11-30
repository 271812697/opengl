#define C2
#ifdef C2
#include <iostream>
#include"GameApp.h"
int main()
{
    openglApp* game = new GameApp();
    game->Init();
    game->Run();
    return 0;
}
#endif // C2


