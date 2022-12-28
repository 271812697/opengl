#pragma once


#include"scene_01.h"
#include"scene_02.h"
#include"scene_03.h"
#include"scene_04.h"
const char* str[] = { "01","02","03","04"};
int scene_cnt = 4;

scene::Scene* attach_Scene(int cur) {
    switch (cur) {
    case 0:return new scene::Scene01("01");
        break;
    case 1:return new scene::Scene02("02");
        break;
    case 2:return new scene::Scene03("03");
        break;
    case 3:return new scene::Scene04("04");
        break;
    default:break;
    }
    return nullptr;
}