#pragma once


#include"scene_01.h"
#include"scene_02.h"
#include"scene_03.h"
#include"scene_04.h"
#include"scene_05.h"
const char* str[] = { "01","02","03","04","05"};
int scene_cnt = 5;

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
    case 4:return new scene::Scene05("05");
        break;
    default:break;
    }
    return nullptr;
}