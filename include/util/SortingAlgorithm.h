/**
 * @file  SortingAlgorithm.h
 * @brief Definions of SortingAlgorithm class which implements
 * several sorting algorithms.
 */

#ifndef SortingAlgorithm_h
#define SortingAlgorithm_h

#include <types.h>
NS_IZENELIB_UTIL_BEGIN

template <class Element> class Array;

/**
 * @brief Definions of SortingAlgorithm class which implements
 * several sorting algorithms.
 */
template <class Element>
class SortingAlgorithm {
 public:
  SortingAlgorithm() {}
  virtual ~SortingAlgorithm() {}

  virtual void sort(const Array<Element>& array, Array<int>& a) = 0;
};

bool t_Sort(SortingAlgorithm<int>& sa);

NS_IZENELIB_UTIL_END
#endif
