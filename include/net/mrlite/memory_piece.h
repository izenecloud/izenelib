#ifndef MRLITE_MEMORY_PIECE_H_
#define MRLITE_MEMORY_PIECE_H_

#include <stdio.h>
#include <functional>
#include <string>

#include "common.h"

namespace net{namespace mrlite{

typedef uint32 PieceSize;


// Represent either a piece of memory, which is prepended by a
// PieceSize, or a std::string object.
class MemoryPiece
{
    friend std::ostream& operator<< (std::ostream&, const MemoryPiece& p);

public:
    MemoryPiece() : piece_(NULL), string_(NULL) {}
    MemoryPiece(char* piece, PieceSize size)
    {
        Set(piece, size);
    }
    explicit MemoryPiece(std::string* string)
    {
        Set(string);
    }

    void Set(char* piece, PieceSize size)
    {
        piece_ = piece;
        *reinterpret_cast<PieceSize*>(piece_) = size;
        string_ = NULL;
    }

    void Set(std::string* string)
    {
        string_ = string;
        piece_ = NULL;
    }

    void Clear()
    {
        piece_ = NULL;
        string_ = NULL;
    }

    bool IsSet() const
    {
        return IsString() || IsPiece();
    }
    bool IsString() const
    {
        return string_ != NULL;
    }
    bool IsPiece() const
    {
        return piece_ != NULL;
    }

    const char* Piece() const
    {
        return piece_;
    }

    char* Data()
    {
        return IsPiece() ? piece_ + sizeof(PieceSize) :
               (IsString() ? const_cast<char*>(string_->data()) : NULL);
    }

    const char* Data() const
    {
        return IsPiece() ? piece_ + sizeof(PieceSize) :
               (IsString() ? string_->data() : NULL);
    }

    size_t Size() const
    {
        return IsPiece() ? *reinterpret_cast<PieceSize*>(piece_) :
               (IsString() ? string_->size() : 0);
    }

private:
    char* piece_;
    std::string* string_;
};


// Compare two MemoryPiece objects in lexical order.
struct MemoryPieceLessThan : 
    public std::binary_function<const MemoryPiece&,const MemoryPiece&,bool>
{
    bool operator() (const MemoryPiece& x, const MemoryPiece& y) const;
};

bool MemoryPieceEqual(const MemoryPiece& x, const MemoryPiece& y);

bool WriteMemoryPiece(FILE* output, const MemoryPiece& piece);
bool ReadMemoryPiece(FILE* input, std::string* piece);

}}  // namespace mrlite

#endif  // MRLITE_MEMORY_PIECE_H_

