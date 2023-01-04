#pragma once
#include <cstddef>

namespace Urho3D
{

    struct AllocatorNode;

    /// %Allocator memory block.
    struct AllocatorBlock
    {
        /// Size of a node.
        int nodeSize_;

        /// Number of nodes in this block.
        int capacity_;

        /// First free node.
        AllocatorNode* free_;

        /// Next allocator block.
        AllocatorBlock* next_;

        // Nodes follow
    };

    /// %Allocator node.
    struct AllocatorNode
    {
        /// Next free node.
        AllocatorNode* next_;

        // Data follows
    };

    /// Initialize a fixed-size allocator with the node size and initial capacity.
     AllocatorBlock* AllocatorInitialize(int nodeSize, int initialCapacity = 1);

    /// Uninitialize a fixed-size allocator. Frees all blocks in the chain.
     void AllocatorUninitialize(AllocatorBlock* allocator);

    /// Reserve a node. Creates a new block if necessary.
     void* AllocatorReserve(AllocatorBlock* allocator);

    /// Free a node. Does not free any blocks.
     void AllocatorFree(AllocatorBlock* allocator, void* ptr);

    /// %Allocator template class. Allocates objects of a specific class.
    template <class T> class Allocator
    {
    private:
        /// Allocator block.
        AllocatorBlock* allocator_;

    public:
        /// Construct.
        explicit Allocator(int initialCapacity = 0)
            : allocator_(nullptr)
        {
            if (initialCapacity)
                allocator_ = AllocatorInitialize((int)sizeof(T), initialCapacity);
        }

        /// Destruct.
        ~Allocator()
        {
            AllocatorUninitialize(allocator_);
        }

        /// Prevent copy construction.
        Allocator(const Allocator<T>& rhs) = delete;

        /// Prevent assignment.
        Allocator<T>& operator =(const Allocator<T>& rhs) = delete;

        /// Reserve and default-construct an object.
        T* Reserve()
        {
            if (!allocator_)
                allocator_ = AllocatorInitialize((int)sizeof(T));

            T* newObject = static_cast<T*>(AllocatorReserve(allocator_));
            new(newObject) T();

            return newObject;
        }

        /// Reserve and copy-construct an object.
        T* Reserve(const T& object)
        {
            if (!allocator_)
                allocator_ = AllocatorInitialize((int)sizeof(T));

            T* newObject = static_cast<T*>(AllocatorReserve(allocator_));
            new(newObject) T(object);

            return newObject;
        }

        /// Destruct and free an object.
        void Free(T* object)
        {
            (object)->~T();
            AllocatorFree(allocator_, object);
        }
    };

}
