

#pragma once


#include "../Container/Swap.h"
#include "Iter.h"

namespace Urho3D
{

/// %Vector base class.
/** Note that to prevent extra memory use due to vtable pointer, %VectorBase intentionally does not declare a virtual destructor
    and therefore %VectorBase pointers should never be used.
  */
class  VectorBase
{
public:
    /// Construct.
    VectorBase() noexcept :
        size_(0),
        capacity_(0),
        buffer_(nullptr)
    {
    }

    /// Swap with another vector.
    void Swap(VectorBase& rhs)
    {
        Urho3D::Swap(size_, rhs.size_);
        Urho3D::Swap(capacity_, rhs.capacity_);
        Urho3D::Swap(buffer_, rhs.buffer_);
    }

protected:
    static unsigned char* AllocateBuffer(int size);

    /// Size of vector.
    int size_;
    /// Buffer capacity.
    int capacity_;
    /// Buffer.
    unsigned char* buffer_;
};

}
