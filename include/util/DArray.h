/**
 * @file DArray.h
 * @brief The header file of dynamic array implementation.
 *
 * This file defines an DynamicArray class, which can enlarge and shrink
 *  as we do insertions and deletions.
 */

#ifndef DynamicArray_h
#define DynamicArray_h

#include <types.h>
#include <util/Array.h>

NS_IZENELIB_UTIL_BEGIN

const int DEFAULT_INCR_VAL = -1;
const int defaultIncrVal = 32;

/**
 *  @brief The wrapper for dynamic array class.
 */
template <class Element>
class DynamicArray 
: public Array<Element>
{
 protected:
  Element initialValue;
  int sizeAllocated; // physically allocated size. Note that this
  // size is different from arraySize which is
  // the size actually ever been accessed.
  int wasInitialized;
  int incrementSize;

  void increase_size();

 public:

  // normal copy constructor.
  DynamicArray(const DynamicArray& source);
  DynamicArray(int base, int size, const Element& initialVal);
  DynamicArray(int size, const Element& initialVal);
  DynamicArray(int size);
  DynamicArray();
  DynamicArray& operator=(const DynamicArray& source);
  DynamicArray& operator=(const Array<Element>& source);

  ~DynamicArray();

  void set_init_value(const Element& init) { initialValue = init; }
  void set_increment_size(int incrSize);
  int get_increment_size(void) const {return incrementSize;}

  void extend_array(int sizeRequested);
  Element& index(int i);
  Element& operator[](int i);
  // These two methods do not have dynamic feature, because they
  // are const functions. So just call the base methods.
  const Element& index(int i) const {
    return Array<Element>::index(i);
  }
  const Element& operator[](int i) const {
    return Array<Element>::operator[](i);
  }

  void push_back(const Element& elem) { index(Array<Element>::size()) = elem; }
  void append(const Array<Element>& other);
  void truncate(int size);

  // List functionalities.
  void insert(const Element& elem, int pos);
  void remove(int pos);
  void find_insert(const Element& elem);

  int check_range(int index) const {
    return (index >= Array<Element>::low() && index <= Array<Element>::high()); }

  bool operator==(const DynamicArray<Element>& src) const {
    return Array<Element>::operator==(src);
  }

  bool operator!=(const DynamicArray<Element>& src) const {
    return !operator==(src);
  }

  //DUMMY_COMPARISON_SUB(DynamicArray<Element>);
};

// MLC++ - Machine Learning Library in -*- C++ -*-
// See Descrip.txt for terms and conditions relating to use and distribution.

/*
 * @brief  Implements dynamic array derived from Array<>.

 DynamicArray may grow when using operator=(),
 operator[], and index() for non-const object.
 DynamicArray may shrink to the size of the source
 array when using operator=() for non const object.
 DynamicArray may not grow or shrink for const object.
 An attempt to try to read beyond the current
 allocated range  by using operator[] or index() is
 regarded as a fatal error.
 The reason for this is that by allowing these operators to
 extend the array, we are allowing a const object to be
 changed, which contradicts conceptual constness (not to
 mention bitwise constness).  For example, the size of the
 array could change even if we call a function taking a
 const array.
 size() returns the largest index, ever been
 accessed either for read or for write, plus one.
 Users are advised to try to extend array as large as
 possible at his/her first try, which will reduce
 additional unnecessary operations otherwise incurred.

*/


/**
 *
 @brief  Increase sizeAllocated  according to incrementSize
 data member.

 This is a protected member function.
 Increase the size of allocated memory very fast up to
 1024 elements and afterwards increase by the unit.
 This is to increase the performance and avoid memory
 allocation error.

*/
template <class Element>
void DynamicArray<Element>::increase_size()
{
  if( incrementSize == -1 )
    sizeAllocated += defaultIncrVal;
  else
    sizeAllocated += incrementSize;
}


/**
 *
 @brief Extends the current array to be larger than
 indexRequested which is the index requested by
 index() or operator[] methods.

 This is a proctected member function.
 *
 */
template <class Element>
void DynamicArray<Element>::extend_array(int indexRequested)
{
  // increase sizeAllocated member until it gets
  // larger than indexRequested.
  while (sizeAllocated <= indexRequested)
    increase_size();

  Element* tempBuf = new Element[std::max(1,sizeAllocated)];
  int i;
  for (i = 0; i < Array<Element>::arraySize; i++)
    tempBuf[i] = Array<Element>::elements[i];
  // initialize up to indexRequested if wasInitialized is on
  if( wasInitialized )
    for(; i <= indexRequested; i++ )
      tempBuf[i] = initialValue;

  // set the current array size
  Array<Element>::arraySize = indexRequested + 1;
  delete [] Array<Element>::elements;
  Array<Element>::elements = tempBuf;
}

/**
 * @brief Copy constructor

 */
template <class Element>
DynamicArray<Element>::DynamicArray(const DynamicArray<Element>& source)
: Array<Element>(source)
{
  incrementSize = DEFAULT_INCR_VAL;
  wasInitialized = source.wasInitialized;
  sizeAllocated = source.size();

  if(wasInitialized)
    initialValue = source.initialValue;
}




/**
 *
 * @brief Constructor which tkaes a base, a size, and an
 initial value arguments.

*/
template <class Element>
DynamicArray<Element>::DynamicArray(int aBase, int aSize,
                                    const Element& initialVal)
: Array<Element>(aBase,aSize,initialVal)
{
  incrementSize = DEFAULT_INCR_VAL;
  wasInitialized = true;
  initialValue = initialVal;
  sizeAllocated = aSize;
}

/**
 *
 * @brief  Constructor which takes a size and an initial value arguments.
 Base is assumed to be zero.
*/
template <class Element>
DynamicArray<Element>::DynamicArray(int size, const Element& initialVal)
: Array<Element>(0,size,initialVal)
{
  initialValue = initialVal;
  wasInitialized = true;
  incrementSize = DEFAULT_INCR_VAL;
  sizeAllocated = size;
}


/**
 *
 * @brief  Constructor which takes a size argument.
 Base is assumed to be zero.
*/
template <class Element>
DynamicArray<Element>::DynamicArray(int size)
: Array<Element>(size)
{
  incrementSize = DEFAULT_INCR_VAL;
  sizeAllocated = size;
  wasInitialized = false;
}


/**
 *
 *@brief  Becuase of GNU, i have to implicitly define this.
 */
template <class Element>
DynamicArray<Element>::DynamicArray()
: Array<Element>(0)
{
  incrementSize = DEFAULT_INCR_VAL;
  sizeAllocated = 0;
  wasInitialized = false;
}


/**
 *
 * @brief  Set the increment size for the dynamic array to grow.
 -1 means double every time, which has good properties if you
 amortize costs.  For example, a series of sequential
 inserts will have O(1) time complexity per insert, because
 each extension means n/2 new elements have been inserted,
 and charging 2 extra operations for each, means we have
 deposited money for n operations (to move everything when we
 extend the array).

*/

template <class Element>
void DynamicArray<Element>::set_increment_size(int incrSize)
{
  if( incrSize <= 0 && incrSize != -1 ) {
    std::cerr << "DynamicArray<>::set_increment_size : illegal"
      " increment size argument : " << incrSize << std::endl;
    abort();
  }

  incrementSize = incrSize;
}


/**
 *
 * @brief  Returns a reference to the element indexed from the
 base as if it is 0. When index is bigger than the
 current upper bound, the current array is extended
 according to incrementSize data member.

*/
template <class Element>
Element& DynamicArray<Element>::index(int i)
{
  if( i < Array<Element>::arraySize )
    return Array<Element>::index(i);

  if( i <= sizeAllocated - 1) {
    // need to update arraySize and do initialization
    if(wasInitialized)
      for( int j = Array<Element>::arraySize; j <= i; j++ )
        Array<Element>::elements[j] = initialValue;
    Array<Element>::arraySize = i + 1;
  }
  else
    extend_array(i);

  return Array<Element>::index(i);
}



/**
 *
 * @brief  Returns a reference to the element index from the base.

 When index is bigger than the current upper bound,
 the current array is extended according to
 incrementSize data member.
 Please be very careful when using this method as a rhs operand.
 For example, if array is n and the following statement will
 put a garabage into the array element.
 @code

 DynamicArray<int> d(10);
 d[8] = 1;
 d[20] = d[8];

 d[20] will have a garabage instead of 1 because at the
 time of d[20], the reference to d[8] is no longer
 available since it was deleted at extend_array() call!

 correct usage is
 d.extend(20); // first extend.
 d[20] = d[8];
 @endcode
 *
 */
template <class Element>
Element& DynamicArray<Element>::operator[](int i)
{
  if( i - Array<Element>::base < Array<Element>::arraySize )
    return Array<Element>::operator[](i);

  if( i - Array<Element>::base <= sizeAllocated - 1) {
    // need to update arraySize and do initialization
    if(wasInitialized)
      for( int j = Array<Element>::arraySize - Array<Element>::base; j <= i - Array<Element>::base; j++ )
        Array<Element>::elements[j] = initialValue;
    Array<Element>::arraySize = i - Array<Element>::base + 1;
  }
  else
    extend_array(i - Array<Element>::base);

  return Array<Element>::operator[](i);
}



/**
 *
 * @brief  Appends elements from the specified array onto the end
 of this dynamic array.

 *
 */
template <class Element>
void DynamicArray<Element>::append(const Array<Element>& other)
{
  for(int otherElem = 0; otherElem < other.size(); otherElem++)
    index(Array<Element>::size()) = other.index(otherElem);
}


/**
 * @code
 Description : Truncates the size of the DynamicArray as if it has grown
 to the truncated size.
 Comments    :
 * @endcode
 */
template <class Element>
void DynamicArray<Element>::truncate(int sizeVal)
{
  if (sizeVal < 0 || sizeVal > Array<Element>::arraySize) {
    std::cerr << "DynamicArray<>::truncate: Illegal truncated size : " << sizeVal
              << ". The current size is " << Array<Element>::arraySize << std::endl;
    abort();
  }

  Array<Element>::arraySize = sizeVal;
}


/**
 *
 * @brief Insert the given element in the given position. Previous
 elements at or below the given position is shifted to the
 right by one.
 This takes O(n). Must be used with caution.
 *
 */
template <class Element>
void DynamicArray<Element>::insert(const Element& elem, int pos)
{
  if (pos < Array<Element>::low()) {
    std::cerr << "DynamicArray<>::insert : the position : " << pos <<
      " is invalid" << std::endl;
    abort();
  }

  for (int i = Array<Element>::high(); i >= pos; i--) {
    Element t1 = operator[](i); // keep the value to the temporary
    // this is necessary.. don't know why
    operator[](i+1) = t1;
  }


  operator[](pos) = elem;
}


/**
 *
 * @brief  Remove the element in the given position. All the elements
 above the removed elemnt is shifted to the left by one.
 This takes O(n). Must be used with caution due to being an
 expensive operation.
 *
 */
template <class Element>
void DynamicArray<Element>::remove(int pos)
{
  if (pos < Array<Element>::low() || pos > Array<Element>::high()) {
    std::cerr << "DynamicArray<>::remove : the position : " << pos <<
      " is invalid" << std::endl;
    abort();
  }

  for (int i = pos; i < Array<Element>::high(); i++)
    operator[](i) = operator[](i+1);
  truncate(Array<Element>::size() - 1);
}


template <class Element>
DynamicArray<Element>::~DynamicArray()
{
}


template <class Element>
DynamicArray<Element>& DynamicArray<Element>::
operator=(const DynamicArray<Element>& src)
{
  if (this != &src) {
    truncate(0);
    for (int i = src.high(); i >= 0; i--)
      index(i) = src.index(i);
  }
  return *this;
}



  template <class Element>
  DynamicArray<Element>& DynamicArray<Element>::
  operator=(const Array<Element>& src)
{
  truncate(0);
  for (int i = src.high(); i >= 0; i--)
    index(i) = src.index(i);
  return *this;
}




  /**
   *
   * @brief  Inserts an elem into a matching position if already exists,
   otherwise at the end
  */
    template <class Element>
    void DynamicArray<Element>::find_insert(const Element& elem)
{
  int i = -1;
  if (find(elem, i))
    index(i) = elem;
  else
    index(Array<Element>::size()) = elem;
}

NS_IZENELIB_UTIL_END
#endif

