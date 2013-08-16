#ifndef WAND_H_GUARD
#define WAND_H_GUARD

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "heap/Heap.h"
#include "scorer/BM25.h"
#include "SegmentPool.h"

#define MIN(X, Y) (X < Y ? X : Y)
#define TERMINAL_DOCID -1

static void wand(
        SegmentPool* pool, long* headPointers, int* df, float* UB, int len,
        int* docLen, int totalDocs, float avgDocLen, int hits, bool hasTf,
        float** scores)
{
    Heap* elements = initHeap(hits);
    int origLen = len;
    uint32_t** blockDocid = (uint32_t**) calloc(len, sizeof(uint32_t*));
    uint32_t** blockTf = (uint32_t**) calloc(len, sizeof(uint32_t*));
    uint32_t* counts = (uint32_t*) calloc(len, sizeof(uint32_t));
    int* posting = (int*) calloc(len, sizeof(int));
    int* mapping = (int*) calloc(len, sizeof(int));
    float threshold = 0;

    int i, j;
    for (i = 0; i < len; i++)
    {
        blockDocid[i] = (uint32_t*) calloc(BLOCK_SIZE * 2, sizeof(uint32_t));
        blockTf[i] = hasTf ? NULL : (uint32_t*) calloc(BLOCK_SIZE * 2, sizeof(uint32_t));
        counts[i] = decompressDocidBlock(pool, blockDocid[i], headPointers[i]);
        if (!hasTf)
        {
            decompressTfBlock(pool, blockTf[i], headPointers[i]);
        }
        posting[i] = 0;
        mapping[i] = i;
        if (UB[i] <= threshold)
        {
            threshold = UB[i] - 1;
        }
    }

    for (i = 0; i < len; i++)
    {
        for (j = i + 1; j < len; j++)
        {
            if (GREATER_THAN(blockDocid[mapping[i]][posting[mapping[i]]],
                        blockDocid[mapping[j]][posting[mapping[j]]],
                        pool->reverse))
            {
                int temp = mapping[i];
                mapping[i] = mapping[j];
                mapping[j] = temp;
            }
        }
    }

    int curDoc = 0;
    int pTerm = 0;
    int pTermIdx = 0;

    while (1)
    {
        float sum = 0;
        pTerm = -1;
        pTermIdx = -1;
        for (i = 0; i < len; i++)
        {
            sum += UB[mapping[i]];
            if (sum > threshold)
            {
                pTerm = mapping[i];
                pTermIdx = i;
                if (i < len - 1)
                {
                    if (blockDocid[mapping[i]][posting[mapping[i]]] ==
                            blockDocid[mapping[i + 1]][posting[mapping[i + 1]]])
                    {
                        continue;
                    }
                }
                break;
            }
        }

        if (sum == 0 || pTerm == -1)
        {
            break;
        }

        int pivot = blockDocid[pTerm][posting[pTerm]];

        if (blockDocid[mapping[0]][posting[mapping[0]]] == pivot)
        {
            curDoc = pivot;
            if (pivot != 0)
            {
                float score = 0;
                if (!hasTf)
                {
                    for (i = 0; i <= pTermIdx; i++)
                    {
                        score += _default_bm25(blockTf[mapping[i]][posting[mapping[i]]],
                                df[mapping[i]], totalDocs, docLen[curDoc], avgDocLen);
                    }
                }
                else
                {
                    score = sum;
                }

                if (score > threshold)
                {
                    insertHeap(elements, curDoc, score);
                }

                if (isFullHeap(elements))
                {
                    threshold = minScoreHeap(elements);
                    if (hasTf && len == 1)
                    {
                        break;
                    }
                }
            }

            int atermIdx;
            for (atermIdx = 0; atermIdx < MIN(pTermIdx + 1, len); atermIdx++)
            {
                int aterm = mapping[atermIdx];

                if (posting[aterm] >= counts[aterm] - 1 &&
                        nextPointer(pool, headPointers[aterm]) == UNDEFINED_POINTER)
                {
                    int k = 0;
                    for (i = 0; i < len; i++)
                    {
                        if (i != atermIdx)
                        {
                            mapping[k++] = mapping[i];
                        }
                    }
                    --len;
                    --atermIdx;
                    continue;
                }

                while (LESS_THAN_EQUAL(blockDocid[aterm][posting[aterm]], pivot, pool->reverse))
                {
                    posting[aterm]++;
                    if (posting[aterm] > counts[aterm] - 1)
                    {
                        headPointers[aterm] = nextPointer(pool, headPointers[aterm]);
                        if (headPointers[aterm] == UNDEFINED_POINTER)
                        {
                            break;
                        }
                        else
                        {
                            counts[aterm] = decompressDocidBlock(pool, blockDocid[aterm], headPointers[aterm]);
                            if (!hasTf)
                            {
                                decompressTfBlock(pool, blockTf[aterm], headPointers[aterm]);
                            }
                            posting[aterm] = 0;
                        }
                    }
                }
            }
        }
        else
        {
            int aterm = mapping[0];
            int atermIdx;
            for (atermIdx = 0; atermIdx < MIN(pTermIdx + 1, len); atermIdx++)
            {
                if (df[mapping[atermIdx]] <= df[aterm] &&
                        LESS_THAN(blockDocid[mapping[atermIdx]][posting[mapping[atermIdx]]], pivot, pool->reverse))
                {
                    int atermTemp = mapping[atermIdx];

                    if (posting[atermTemp] >= counts[atermTemp] - 1 &&
                            nextPointer(pool, headPointers[atermTemp]) == UNDEFINED_POINTER)
                    {
                        int k = 0;
                        for (i = 0; i < len; i++)
                        {
                            if (i != atermIdx)
                            {
                                mapping[k++] = mapping[i];
                            }
                        }
                        --len;
                        --atermIdx;
                        continue;
                    }
                    aterm = atermTemp;
                }
            }

            while (LESS_THAN(blockDocid[aterm][posting[aterm]], pivot, pool->reverse))
            {
                posting[aterm]++;
                if (posting[aterm] > counts[aterm] - 1)
                {
                    headPointers[aterm] = nextPointer(pool, headPointers[aterm]);
                    if (headPointers[aterm] == UNDEFINED_POINTER)
                    {
                        break;
                    } else {
                        counts[aterm] = decompressDocidBlock(pool, blockDocid[aterm], headPointers[aterm]);
                        if (!hasTf)
                        {
                            decompressTfBlock(pool, blockTf[aterm], headPointers[aterm]);
                        }
                        posting[aterm] = 0;
                    }
                }
            }
        }

        for (i = 0; i < len; i++)
        {
            for (j = i + 1; j < len; j++)
            {
                if (GREATER_THAN(blockDocid[mapping[i]][posting[mapping[i]]],
                            blockDocid[mapping[j]][posting[mapping[j]]],
                            pool->reverse))
                {
                    int temp = mapping[i];
                    mapping[i] = mapping[j];
                    mapping[j] = temp;
                }
            }
        }
    }

    // Free the allocated memory
    free(posting);
    free(mapping);
    for (i = 0; i < origLen; i++)
    {
        free(blockDocid[i]);
        if (blockTf[i])
        {
            free(blockTf[i]);
        }
    }
    free(blockDocid);
    free(blockTf);
    free(counts);

    int* set = (int*) calloc(elements->index + 1, sizeof(int));
    memcpy(set, &elements->docid[1], elements->index * sizeof(int));
    memcpy(*scores, &elements->score[1], elements->index * sizeof(float));
    if (!isFullHeap(elements))
    {
        set[elements->index] = TERMINAL_DOCID;
    }
    destroyHeap(elements);
    return set;
}

#endif
