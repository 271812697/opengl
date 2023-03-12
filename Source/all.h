#pragma once
#include"../GameApp.h"
#define MAINSCENE(className,name) \
int main()\
{\
GameApp* game = new GameApp();\
scene::Scene* s = new className(name);\
game->attachScene(s);\
game->Init();\
game->Run();\
return 0;\
}

