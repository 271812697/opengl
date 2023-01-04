// Copyright (c) 2008-2022 the Urho3D project
// License: MIT
#include "../Container/HashMap.h"
#include "../Container/HashSet.h"
#include "../Container/Sort.h"
#include "../Container/Str.h"
#include<assert.h>
namespace Urho3D
{

void HashBase::AllocateBuckets(int size, int numBuckets)
{
    assert(size >= 0 && numBuckets > 0);

    delete[] ptrs_;

    HashNodeBase** ptrs = new HashNodeBase* [numBuckets + 2];
    int* data = reinterpret_cast<int*>(ptrs);
    data[0] = size;
    data[1] = numBuckets;
    ptrs_ = ptrs;

    ResetPtrs();
}

void HashBase::ResetPtrs()
{
    // Reset bucket pointers
    if (!ptrs_)
        return;

    int numBuckets = NumBuckets();
    HashNodeBase** ptrs = Ptrs();
    for (int i = 0; i < numBuckets; ++i)
        ptrs[i] = nullptr;
}

void HashBase::SetSize(int size)
{
    assert(size >= 0);

    if (ptrs_)
        (reinterpret_cast<int*>(ptrs_))[0] = size;
}

}
