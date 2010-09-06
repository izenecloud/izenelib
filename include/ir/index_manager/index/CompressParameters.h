#ifndef E_COMPRESSION_PARAMETERS_H_
#define E_COMPRESSION_PARAMETERS_H_

NS_IZENELIB_IR_BEGIN

namespace indexmanager{

// Maximum number of documents in a chunk.
#define CHUNK_SIZE 128

// Fixed size in bytes of a block
#define BLOCK_SIZE 65536

// The lower bound size of a single chunk in bytes (one integer per docID, frequency, and position).
// This is because all our coding methods use at least one word per integer encoded.
#define MIN_COMPRESSED_CHUNK_SIZE (3 * sizeof(uint32_t))

// Determines the size of the input buffer that will be compressed.
// It needs to be a multiple of the block size for when we use blockwise codings, because we need to pad the input buffer with 0s until the block size.
// If the block size is 0, we have a non-blockwise coder, and don't need an upperbound.
#define UncompressedInBufferUpperbound(buffer_size, block_size) ((((block_size) == 0) || ((buffer_size) % (block_size) == 0)) ? (buffer_size) : ((((buffer_size) / (block_size)) + 1) * (block_size)))

// Determines size of the output buffer for decompressing to.
// Normally, this wouldn't be necessary, since if you know the uncompressed length, you wouldn't need to do an upperbound.
// However, S9 and S16 codings have a quirk that requires the output buffer array (to which we decompress) to have at least 28 empty slots;
// this is because there is a case where a word will have a max of 28 integers compressed within, and we will access the output buffer
// for all the 28 integers, even if some of those 28 integers are garbage which we haven't really compressed (but this was the case used to encode them).
// So we need to round the number of compressed integers to a multiple of 28 and make sure there is at least 28 extra space at the end of the array
// to ensure there is ample room for decompression. Here, we just use 32 instead of 28 for convenience.
#define UncompressedOutBufferUpperbound(buffer_size) ((((buffer_size) >> 5) + 2) << 5)

// Determines size of output buffer for compression.
// Here we just double it, but it could really be a tighter bound.
#define CompressedOutBufferUpperbound(buffer_size) ((buffer_size) << 1)

}

NS_IZENELIB_IR_END

#endif

