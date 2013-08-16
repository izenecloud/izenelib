#ifndef SVS_H_GUARD
#define SVS_H_GUARD

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SegmentPool.h"

#define MIN(X, Y) (X < Y ? X : Y)
#define TERMINAL_DOCID -1

// Gallop to >= docid
inline int gallopSearch(SegmentPool* pool, int* data, int* count,
                        int* index, long* pointer, int docid) {
  while(1) {
    if(*index == *count) {
      (*pointer) = nextPointer(pool, *pointer);
      if(*pointer == UNDEFINED_POINTER) {
        return 0;
      }
      memset(data, 0, BLOCK_SIZE * 2 * sizeof(unsigned int));
      (*count) = decompressDocidBlock(pool, data, *pointer);
      (*index) = 0;
    }
    if(data[*count - 1] == docid) {
      (*index) = *count - 1;
      return 1;
    } else if(LESS_THAN(data[*count - 1], docid, pool->reverse)) {
      (*index) = *count;
      continue;
    }

    if(GREATER_THAN_EQUAL(data[*index], docid, pool->reverse)) {
      return 1;
    }

    int beginIndex = *index;
    int hop = 0;
    int tempIndex = beginIndex;
    while(tempIndex < *count) {
      if(LESS_THAN(data[tempIndex], docid, pool->reverse)) {
        beginIndex = tempIndex;
        hop = hop == 0 ? 1 : hop * 2;
        tempIndex += hop;
      } else {
        break;
      }
    }
    if(data[beginIndex] == docid) {
      (*index) = beginIndex;
      return 1;
    }

    int endIndex = *count - 1;
    hop = 0;
    tempIndex = endIndex;
    while(tempIndex >= 0) {
      if(GREATER_THAN(data[tempIndex], docid, pool->reverse)) {
        endIndex = tempIndex;
        hop = hop == 0 ? 1 : hop * 2;
        tempIndex -= hop;
      } else {
        break;
      }
    }
    if(data[endIndex] == docid) {
      (*index) = endIndex;
      return 1;
    }

    // Binary search between begin and end indexes
    int mid = beginIndex;
    while(beginIndex < endIndex) {
      mid = beginIndex + ((endIndex - beginIndex) / 2);

      if(LESS_THAN(docid, data[mid], pool->reverse)) {
        endIndex = mid - 1;
      } else if(GREATER_THAN(docid, data[mid], pool->reverse)) {
        beginIndex = mid + 1;
      } else {
        (*index) = mid;
        return 1;
      }
    }
    (*index) = endIndex;
    if(*index < 0) {
      (*index) = 0;
    } else if(*index >= *count) {
      (*index) = *count;
      continue;
    }
    return 1;
  }
}

int* intersectPostingsLists_SvS(SegmentPool* pool, long a, long b, int minDf) {
  int* set = (int*) calloc(minDf, sizeof(int));
  unsigned int* dataA = (unsigned int*) calloc(BLOCK_SIZE * 2, sizeof(unsigned int));
  unsigned int* dataB = (unsigned int*) calloc(BLOCK_SIZE * 2, sizeof(unsigned int));

  int cA = decompressDocidBlock(pool, dataA, a);
  int cB = decompressDocidBlock(pool, dataB, b);
  int iSet = 0, iA = 0, iB = 0;

  while(a != UNDEFINED_POINTER && b != UNDEFINED_POINTER && iSet < minDf) {
    if(dataB[iB] == dataA[iA]) {
      set[iSet++] = dataA[iA];
      iA++;
      iB++;
      if(iA == cA) {
        a = nextPointer(pool, a);
        if(a == UNDEFINED_POINTER) {
          break;
        }
        memset(dataA, 0, BLOCK_SIZE * 2 * sizeof(unsigned int));
        cA = decompressDocidBlock(pool, dataA, a);
        iA = 0;
      }
      if(iB == cB) {
        b = nextPointer(pool, b);
        if(b == UNDEFINED_POINTER) {
          break;
        }
        memset(dataB, 0, BLOCK_SIZE * 2 * sizeof(unsigned int));
        cB = decompressDocidBlock(pool, dataB, b);
        iB = 0;
      }
    }

    if(LESS_THAN(dataA[iA], dataB[iB], pool->reverse)) {
      iA++;
      if(!gallopSearch(pool, dataA, &cA, &iA, &a, dataB[iB])) {
        break;
      }
    } else if(LESS_THAN(dataB[iB], dataA[iA], pool->reverse)) {
      iB++;
      if(!gallopSearch(pool, dataB, &cB, &iB, &b, dataA[iA])) {
        break;
      }
    }
  }

  if(iSet < minDf) {
    set[iSet] = TERMINAL_DOCID;
  }

  free(dataA);
  free(dataB);

  return set;
}

int intersectSetPostingsList_SvS(SegmentPool* pool, long a, int* currentSet, int len) {
  unsigned int* data = (unsigned int*) calloc(BLOCK_SIZE * 2, sizeof(unsigned int));
  int c = decompressDocidBlock(pool, data, a);
  int iSet = 0, iCurrent = 0, i = 0;

  while(a != UNDEFINED_POINTER && iCurrent < len) {
    if(currentSet[iCurrent] == TERMINAL_DOCID) {
      break;
    }
    if(data[i] == currentSet[iCurrent]) {
      currentSet[iSet++] = currentSet[iCurrent];
      iCurrent++;
      i++;

      if(i == c) {
        a = nextPointer(pool, a);
        if(a == UNDEFINED_POINTER) {
          break;
        }
        memset(data, 0, BLOCK_SIZE * 2 * sizeof(unsigned int));
        c = decompressDocidBlock(pool, data, a);
        i = 0;
      }
      if(iCurrent == len) {
        break;
      }
      if(currentSet[iCurrent] == TERMINAL_DOCID) {
        break;
      }
    }

    if(LESS_THAN(data[i], currentSet[iCurrent], pool->reverse)) {
      i++;
      if(!gallopSearch(pool, data, &c, &i, &a, currentSet[iCurrent])) {
        break;
      }
    } else {
      while(LESS_THAN(currentSet[iCurrent], data[i], pool->reverse)) {
        iCurrent++;
        if(iCurrent == len) {
          break;
        }
        if(currentSet[iCurrent] == TERMINAL_DOCID) {
          break;
        }
      }
    }
  }

  if(iSet < len) {
    currentSet[iSet] = TERMINAL_DOCID;
  }

  free(data);
  return iSet;
}

int* intersectSvS(SegmentPool* pool, long* headPointers, int len, int minDf, int hits) {
  if(len < 2) {
    unsigned int* block = (unsigned int*) calloc(BLOCK_SIZE * 2, sizeof(unsigned int));
    int length = MIN(minDf, hits);
    int* set = (int*) calloc(length, sizeof(int));
    int iSet = 0;
    long t = headPointers[0];
    while(t != UNDEFINED_POINTER && iSet < length) {
      memset(block, 0, BLOCK_SIZE * 2 * sizeof(unsigned int));
      int c = decompressDocidBlock(pool, block, t);
      int r = iSet + c <= length ? c : length - iSet;
      memcpy(&set[iSet], block, r * sizeof(int));
      iSet += r;
      t = nextPointer(pool, t);
    }
    free(block);
    return set;
  } else if(len == 2) {
    return intersectPostingsLists_SvS(pool, headPointers[0], headPointers[1], MIN(minDf, hits));
  }

  int* set = intersectPostingsLists_SvS(pool, headPointers[0], headPointers[1], minDf);
  int i;
  for(i = 2; i < len; i++) {
    if(set[0] == TERMINAL_DOCID) {
      break;
    }
    intersectSetPostingsList_SvS(pool, headPointers[i], set, minDf);
  }
  return set;
}

#endif
