// Copyright (c) 2008-2022 the Urho3D project
// License: MIT
#include "../Container/VectorBase.h"
namespace Urho3D
{

unsigned char* VectorBase::AllocateBuffer(int size)
{
    return new unsigned char[size];
}

}
