#ifndef MRLITE_MEMORY_ALLOCATOR_H_
#define MRLITE_MEMORY_ALLOCATOR_H_

#include "memory_piece.h"

namespace net{namespace mrlite{

class MemoryPiece;

class NaiveMemoryAllocator
{
public:
    explicit NaiveMemoryAllocator(const int pool_size);
    ~NaiveMemoryAllocator();

    // Returns false for insufficiency memory.
    bool Allocate(PieceSize size, MemoryPiece* piece);
    // Check if there is sufficient memory to hold a string.
    bool Have(PieceSize length) const;
    // Check if there is sufficient memory to hold two strings.
    bool Have(PieceSize key_length, PieceSize value_length);
    // Reclaims all allocated blocks for the next round of allocations.
    void Reset();

    const char* Pool()
    {
        return pool_;    // For test only.
    }
    size_t PoolSize() const
    {
        return pool_size_;
    }
    size_t AllocatedSize() const
    {
        return allocated_size_;
    }
    bool IsInitialized() const
    {
        return pool_ != NULL;
    }

private:
    char* pool_;
    size_t pool_size_;
    size_t allocated_size_;

    DISALLOW_COPY_AND_ASSIGN(NaiveMemoryAllocator);
};

}}  // namespace mrlite

#endif  // MRLITE_MEMORY_ALLOCATOR_H_
