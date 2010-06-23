/**
 * @file Array.h
 * @brief The header file of array class. It is the based class of the
 *         dynamic array.
 *
 * This file defines an Array class.
 */
#ifndef Array_h
#define Array_h

#include <types.h>
#include "sortCompare.h"
#include "MRandom.h"
#include "SortingAlgorithm.h"

#include <stddef.h>
#include <iostream>

NS_IZENELIB_UTIL_BEGIN
/**
 * @brief Defines a array structure represents
 *  continuous storage.
 */
template <class Element>
class Array {
 protected:   // in case we want to derive a DynamicArray from Array.
  int base, arraySize;
  void alloc_array(int base, int size);
  void copy(const Array& source); // used by copy constructor
  Element *elements;   // the name array conflicts with LEDA
 public:
  Array(const Array& source);
  Array(int base, int size);
  Array(int size);
  Array(int base, int size, const Element& initialValue);
  ~Array();
  Array& operator=(const Array& source);
  const Element *get_elements() { return elements; }
  void init_values(const Element& initalValue);
  int high () const { return base + arraySize - 1; }
  int low ()  const { return base; }
  int size () const { return arraySize; }
  void sort(); // use system version qsort.
  void sort(SortingAlgorithm<Element>& sa, Array<int>& a) const { sa.sort(*this, a); }
  int bfind(const Element& key) const;

  void permute(long seed);
  Element& index(int i);
  const Element& index(int i) const;
  Element& operator[](int i);

  const Element& front() const { return index(0); }
  Element& front() { return index(0); }
  const Element& back() const { return index(arraySize-1); }
  Element& back() { return index(arraySize-1); }

  const Element& operator[](int i) const;
  const Element& get_max(int& idx) const;  // idx is zero based index to array
  const Element& get_max() const;
  const Element& get_min(int& idx) const;  // idx is zero based index to array
  const Element& get_min() const;

  bool find(const Element& elem, int& index) const;
  bool find(const Element& elem) const {
    int dummy;
    return find(elem, dummy);
  }

  void  display(std::ostream& stream = std::cout) const;

  bool operator==(const Array<Element>& src) const;
  bool operator!=(const Array<Element>& src) const {
    return !operator==(src);
  }

  friend std::ostream& operator<<(std::ostream& stream, const Array<Element>& source) {
    source.display(stream);
    return stream;
  }
};


// MLC++ - Machine Learning Library in -*- C++ -*-
// See Descrip.txt for terms and conditions relating to use and distribution.
/*
 *
 * @brief  Array template with error checking.

 *               Can be initialized with
 a lower bound & size, or the size only, in which
 case zero is the assumed lower bound.  Optional final
 initialization argument for lower & upper bound is an
 initial value.
 Access to elements is either by operator[], or by
 index which ranges form 0 to size-1.
 Derived Class PtrArray is an array of pointers, which,
 when its destructor is called, calls delete on
 each of the pointers. The default initialization is
 to NULL, but via the 3-argument constructor, the user
 can turn off this O(n) operation.

 The header file is not included because this is a
 template file.
 As a result of the destructor for PtrArray, all of the
 pointers should be pointing to either 1) a piece of
 memory that can be freed, or 2) NULL.  Furthermore,
 to help prevent errors, the user is not allowed to
 initialize the array with anything except for NULL.
 The sorting of Arrays turns out to be complex.  We do not
 want to force classes to have operator<, and the obvious
 solution which is to create a static function to pass to
 qsort fails, because static functions are always
 instantiated.   The solution used here it to create a
 templated function sort_compare, and take its address in
 the sort member function.  A cast must be used to cast
 it to a function taking two voids.  See description in
 sort().
 Initializing an array takes O(num-elements).
 sort() is done using the system's qsort, which should
 take O(n^2) in the worst case, but O(n*logn) on
 average.
 In many cases, both min() and max() are needed.
 In such cases we could save some comparisons by
 computing them together.  Not only would this
 iterate once through the array, but we could do
 this trick of comparing pairs of elements in the
 array.  Then the higher is compared to max and the
 lower to min.  This saves .5n comparisons.
 Added read_bin and write_bin methods to save Array
 in binary format, this is why MLCStream is included.

*/


/**
 * @brief Creates space for a newly constructed array
 */
template <class Element>
void Array<Element>::alloc_array(int lowVal, int sizeVal)
{
  if (sizeVal < 0) {
    std::cerr << "Array<>::Illegal bounds requested, base: " << lowVal <<
      " high: " << lowVal + sizeVal << std::endl;
    abort();
  }

  base = lowVal;
  arraySize = sizeVal;
  elements = new Element[std::max(1,arraySize)];
}


/**
 * @brief Creates space for new array and assigns its values.
 *
 creates space for new array and assigns its values
 O(num-elements) time, assumes no need to free array

*/
template <class Element>
void Array<Element>::copy(const Array<Element>& source)
{
  alloc_array(source.base, source.arraySize);
  for (int i = 0; i < arraySize; i++)
    index(i) = source.index(i);
}


template <class Element>
Array<Element>::~Array()
{
  if (elements)
    delete [] elements;
  elements = NULL;
}


/**
 * @brief Copy constructor which takes an extra argument.
 *
 Copy constructor which takes an extra argument.
 We don't use the standard issue copy constructor because
 we want to know exactly when we call the copy constructor.
 *
 */
template <class Element>
Array<Element>::Array(const Array<Element>& source)
: base(0), arraySize(0), elements(NULL)
{
  copy(source);
}


/**
 * @brief Constructor which takes a base and size.
 *
 Constructor which takes a base and size.
 Note that because we provide this constructor, we
 can't make size and initial value constructor because
 it will conflict with this constructor when
 the template is instantiated with an integer.

 *
 */
template <class Element>
Array<Element>::Array(int aBase, int aSize)
: base(0), arraySize(0), elements(NULL)
{
  alloc_array(aBase, aSize);
}


/**
 * @brief Constructor which takes a size.

 Constructor which takes a size.
 Indexed from zero.

*/
template <class Element>
Array<Element>::Array(int size)
: base(0), arraySize(0), elements(NULL)
{
  alloc_array(0, size);
}



/**
 * @brief Constructor which takes a low value, size and initial
 value.
 *
 Constructor which takes a low value, size and initial
 value.  Initializes all elements to the initial value.
 Takes O(num-elements) time.
 *
 */
template <class Element>
Array<Element>::Array(int aBase, int aSize,
                      const Element& initialValue)
: base(0), arraySize(0), elements(NULL)
{
  alloc_array(aBase,aSize);
  init_values(initialValue);
}


/**
 * @brief Copies values from source.
 *
 Copies values from source.
 Runs in O(num-elements) time.
 Arrays must be the same size.
 *
 */
template <class Element>
Array<Element>& Array<Element>::operator=(const Array<Element>& elem)
{
  if (this != &elem) {
    for (int i = 0; i < arraySize; i++)
      index(i) = elem.index(i);
  }
  return *this;
}


/**
 * @brief Initializes all elements of an array to the initialValue
 *
 Initializes all elements of an array to the initialValue
 Runs in O(num-elements) time
 *
 */
  template <class Element>
  void Array<Element>::init_values(const Element& initialValue)
{
  // walk through the array and initialize all of the values
  for (int i = 0; i < arraySize; i++)
    index(i) = initialValue;
}


/**
 * @brief Sorts the items in the array using C qsort() function.
 *
 Sorts the items in the array using C qsort() function.
 Overridden by PtrArray, but NOT virtual.  The reason is
 that virtual functions are always instantiated, and we
 do not want to force operator< to be defined.
 We take the address of sort_compare which is a templated
 function, and then cast it to one that takes two voids.
 It is important to get the pointer to the function with
 the actual type, THEN cast, because if we get a pointer
 to a function taking two voids, it will not use the
 class's operator<.
 Any changes here must be made in PtrArray::sort().
 *
 */
template <class Element>
void Array<Element>::sort()
{
  if (size() < 2)
    return; // no need to sort;

  // Declare sortFunc as a pointer to one of the overloaded
  // sort_compare functions (template functions) which have our
  // desired type.
  int (*sortFunc)(const Element *a, const Element *b) = &sort_compare;

  // Now cast the function into one that takes two voids.
  int (*voidSortFunc)(const void *a, const void *b) =
    (int (*)(const void*, const void*))sortFunc;
  qsort(elements, size(), sizeof(*elements), voidSortFunc);
}



/**
 * @brief Finds an element using a binary search.
 *
 Finds an element using a binary search. Returns the index number
 or -1 when failed.
 The array is assumed to be sorted. The result without being sorted
 is unknown.
 This method also assumes the use of operator-();
 * @endcode
 */
template <class Element>
int Array<Element>::bfind(const Element& key) const
{
  int lo = 0;
  int hi = high();
  int mid;
  while (lo <= hi) {
    mid = (lo + hi)/2;
    int c = key - index(mid);
    if (c < 0)
      hi = mid-1;
    else if (c > 0)
      lo = mid+1;
    else
      return mid;
  }
  return -1;
}


/**
 * @brief Permutes the elements of the array.
 *	permutes the elements of the array.
 Make use of MRandom.
 *
 */
template <class Element>
void Array<Element>::permute(long seed)
{
  MRandom random(seed);

  Element t;
  for (int i = 0; i < size(); i++) {
    t = index(i);
    int si = random.integer(size());
    index(i) = index(si);
    index(si) = t;
  }
}


/**
 * @brief Return a reference to the correct element
 referenced from a zero base.

 The next two functions return a reference and a const
 reference, respectively, to the correct element
 referenced from a zero base.

 Out of bounds reference is a fatal error.

*/
template <class Element>
inline Element& Array<Element>::index(int i)
{
  return elements[i];
}

/**
 * @brief Return a const reference to the correct element
 referenced from a zero base.
*/
template <class Element>
inline const Element& Array<Element>::index(int i) const
{
  return elements[i];
}


/**
 * @brief Return a reference to the correct element referenced.
 *
 The next two functions return a reference and a const
 reference, respectively, to the correct element
 referenced.
 Out of bounds reference is a fatal error.

*/
template <class Element>
inline Element& Array<Element>::operator[](int i)
{
  return elements[i - base];
}

/**
 * @brief Return a coonst reference to the correct element referenced.
 */
template <class Element>
inline const Element& Array<Element>::operator[](int i) const
{
  return elements[i - base];
}


/**
 *
 @brief Min/max operations.
 min() and max() return a reference
 to the element in the array.
 Array must not be empty.
 Tie breaker during equality prefer earlier elements in
 the array.
 *
 */
template <class Element>
const Element& Array<Element>::get_max(int& idx) const
{
  if (arraySize == 0) {
    std::cerr << "Array<Element>::get_max() - empty array" << std::endl;
    abort();
  }

  const Element* max = &index(0);
  idx = 0;
  for (int i = 1; i < arraySize; i++)
    if (index(i) > *max) {
      max = &index(i);
      idx = i;
    }
  return *max;
}

template <class Element>
const Element& Array<Element>::get_max() const
{
  int dummy;
  return get_max(dummy);
}


template <class Element>
const Element& Array<Element>::get_min(int& idx) const
{
  if (arraySize == 0) {
    std::cerr << "Array<Element>::min() - empty array" << std::endl;
    abort();
  }


  const Element* min = &index(0);
  idx = 0;
  for (int i = 1; i < arraySize; i++)
    if (index(i) < *min) {
      min = &index(i);
      idx = i;
    }

  return *min;
}

template <class Element>
const Element& Array<Element>::get_min() const
{
  int dummy;
  return get_min(dummy);
}


/**
 * @brief Output array to an MLCStream.

 Output array to an MLCStream

*/
template <class Element>
void Array<Element>::display(std::ostream& stream) const
{
  int i;
  for (i = 0; i < size() - 1; i++)
    stream <<  index(i) << ", ";

  if (size() > 0)
    stream << index(i);  // no trailing comma.
}


/**
 * @brief Finds the given elem and stores the index to the reference
 argument index.

 Finds the given elem and stores the index to the reference
 argument index.
 Returns true iff it finds the element in the array.

*/
template <class Element>
bool Array<Element>::find(const Element& elem, int& index) const
{
  for (int i = low(); i <= high(); i++)
    if ((*this)[i] == elem) {
      index = i;
      return true;
    }

  return false;
}


/**
 * @brief Compares the two arrays element-by-element.
 *
 Compares the two arrays element-by-element
 Runs in O(num-elements) time.
 Arrays must be the same size.
 *
 */
template <class Element>
bool Array<Element>::operator==(const Array<Element>& src) const
{
  if (size() != src.size())
    return false;

  for (int i = 0; i < size(); i++)
    if (index(i) != src.index(i))
      return false;

  return true;
}


/**
 * @brief Compares the two arrays element-by-element
 *
 Compares the two arrays element-by-element
 Returns FALSE for arrays of different sizes.
 Runs in O(num-elements) time.
 *
 */
template <class Element>
bool operator==(const Array<Element>& a1, const Array<Element>& a2)
{
  if(a1.size() != a2.size())
    return false;

  for (int i = 0; i < a1.size(); i++)
    if (a1.index(i) != a2.index(i))
      return false;

  return true;
}

template <class Element>
bool operator!=(const Array<Element>& a1, const Array<Element>& a2)
{
  return !(a1 == a2);
}


NS_IZENELIB_UTIL_END
#endif
