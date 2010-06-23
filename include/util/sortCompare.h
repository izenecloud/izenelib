/**
 * @file  sortCompare.h
 * @brief Provides the template compare functions for qsort.
 */

#ifndef _sortCompare_h
#define _sortCompare_h 1

NS_IZENELIB_UTIL_BEGIN
  template <class Element> int sort_compare(const Element* a, const Element* b);
  template <class Element> int sort_ptr_compare(const Element** a, const Element** b);

  // MLC++ - Machine Learning Library in -*- C++ -*-
  // See Descrip.txt for terms and conditions relating to use and distribution.
  /**
   * @code
  Description  : Template functions for qsort.
  Assumptions  : Class must have operator< and operator==
  Comments     : Apparently, qsort must get a zero back when the elements
                   are equal.  Our first version used only operator<, and
                   returned -1 for equality or greater, and qsort was buggy
                   (see appended example).
  Complexity   : Depends on the class operator< and operator==
  Enhancements :
  History      : Ronny Kohavi                                       5/17/94
                   Initial revision (.h,.c)
                 Yeogirl Yun                                        8/31/99
                   Modifed it to be SortCompare template class with two
                   static members, to make it compile on NT.
   * @endcode
   */




  template <class Element> int sort_compare(const Element* a, const Element* b)
    {
      // since we are using qsort...no need to check equality.
      if (*a < *b)
	return -1;
      else
	return 1;
    }

  template <class Element>
    int sort_ptr_compare(const Element** a, const Element** b)
    {
      if (**a < **b)
	return -1;
      else
	return 1;
    }



  /** The following shows the importance of operator== returning 0 in the
     comparison function.

     compile this with CC -g and it will fail!

     Ronnyk



     #include <iostream.h>
     #include <stdlib.h>

     #define NUM 50

     static int compare_function(const void *item1, const void *item2)
     {
     if (*(int *)item1 < *(int *)item2)
     return -1;
     else
     return 1;
     }


     main()
     {
     int bar[4];
     int foo[NUM];
     int bar2[4];

     bar[0] = bar[1] = bar[2] = bar[3] = 9999;
     bar2[0] = bar2[1] = bar2[2] = bar2[3] = 8888;
     for (int i = 0; i < NUM; i++)
     foo[i] = i % 10;

     qsort(foo, NUM, sizeof(int), compare_function);

     for (i = 0; i < NUM; i++)
     if (foo[i] > 10)
     cout << "Bug.  Seeing " << foo[i] << " at " << i << endl;

     return 0;
     }
  */

NS_IZENELIB_UTIL_END
#endif
