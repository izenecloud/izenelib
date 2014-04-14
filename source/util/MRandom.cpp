/**
 * @file MRandom.cc *
 * @brief Generates random numbers that are really random--the lsb is
 *		   usable.
 *
 * @code
 *		   Each instance of MRandom class has its own state.
 *		   Therefore two instances of MRandom, initialized
		   with the same seed, produce exactly the same sequence
		   of numbers regardless of any interleaving between
		   the calls to them. Also calls to MRandom will not
		   affect anybody else using random().

  Assumptions  : There exists a default state, so the first call to
                   initstate will return a pointer to it.
		 random() is assumed to be a good random number generator.
  Comments     :
  Complexity   :
  Enhancements : Add other tests for randomness.  Information can be
                   found in:
                   "Random Number Generators: Good Ones Are Hard To Find",
                    Park & Miller, Communications of the ACM, Oct. 1988,
                    pp. 1192-1201.  And the Technical Correspondence on this,
                    Communications of the ACM, July 1993, pp. 105-110.
                    Source code in C for this generator can be found
                    in "Data Structures and Algorithm Analysis in C" by
                    Mark Allen Weiss, copyright 1993 Benjamin/Cummings.
                    Note that this code is written so as to avoid overflow
		    due to multiplication - this problem and its solution
		    (the Schrage procedure) is discussed in the first
		    reference.                               Ronnyk
  History      : Yeogirl Yun                                   6/9/04
                   Ported to Linux
                 Svetlozar Nestorov                          12/19/93
                   Initial revision (.h,.c)
 * @endcode
 */
#include <types.h>
#include <util/MRandom.h>
#include <util/random_utils.h>

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <stdlib.h>

NS_IZENELIB_UTIL_BEGIN
  /**
   * @code
  Description : Returns a random number in the range of 0 to LONG_MAX.
  Comments    : We restore the previous state after we get a random
                  number, using the state of the object, for the
		  following reasons. First, there is a bug in
		  setstate() because when it is called with the
		  current state it does not function properly.
		  Second, we want to make sure that somebody outside
		  MRandom can use random() and will not be affected by
		  MRandom.
		Since this is a static function it does not have
		  access to the members of MRandom so the objects
		  calling it pass the state as a parameter.
   * @endcode
   */

  /**
   * @code
  Description : Constructors. The constructor with no arguments
                  initilizes the seed with an arbitrary value.
  Comments    : The arbitrary value for the seed is taken from the
                  current time.
   * @endcode
   */
  MRandom::MRandom()
  {
    init((unsigned int)time(0) + (int)getpid());
  }
  MRandom::MRandom(unsigned int seed)
  {
    init(seed);
  }

  /**
   * @code
      Description : Initializes MRandom with the new seed.
      Comments    :

   * @endcode
   */
  void MRandom::init(unsigned int seed)
  {
    srandom2(seed);
  }

  /**
   * @code
  Description : Two functions that return a random long integer and a
                  random integer in the range from low to high,
                  uniformly distributed.
  Comments    : High should be greater or equal than low.
                If high and low are equal high is returned without
		  using the state.
   * @endcode
   */
  long MRandom::long_int(long low, long high)
  {
    if (high < low) {
      std::cerr << "MRandom::long_int: The lower bound (" << low << ") is "
	"greater than the higher bound (" << high << ")"  << std::endl;
      abort();
    }
    if (high == low)
      return high;

    unsigned long result = wise_random();
    long range = high - low + 1;  // the range is the number of different
    // long integers that could be generated
    return low + result % range;
  }

  int MRandom::integer(int low, int high)
  {
    return (int)long_int(low, high);
  }

  /**
   * @code
  Description : Two functions that return a random long integer and a
                  random integer in the range from 0 to count - 1.
  Comments    : Count should be greater than 0.
   * @endcode
   */
  long MRandom::long_int(long count)
  {
    return long_int(0, count - 1);
  }
  int MRandom::integer(int count)
  {
    return (int)long_int(0, count - 1);
  }

  /**
   * @code
  Description : Returns a random real number uniformly distributed in the
                  interval [low, high).
  Comments    : High should be greater than or equal to low.
                If high and low are equal high is returned without
		  using the state.
                Note that since LONG_MAX has 31 bits the lsb of the
		  real may not be random.
   * @endcode
   */
  float MRandom::real(float low, float high)
  {
    if (high < low) {
      std::cerr << "MRandom::real: The lower bound (" << low << ") is "
	"greater or equal to the higher bound (" << high <<
	")"  << std::endl;
      abort();
    }
    if (high == low)
      return high;

    // shortcut; also (float)(LONG_MAX + 1) is negative ?!
    float denominator = (float)LONG_MAX + 1;

    float range = high - low;
    // calculate the result in two stages in order to get twice the
    //   number of bits in long int
    float result = (float)wise_random()*range/denominator +
      (((float)wise_random()/denominator)*range)/denominator;
    return low + result;
  }



  /**
   * @code
  Description : This is binary random variable with  the probability of
                   'prob'
  Comments    :
   * @endcode
   */
  int MRandom::flip(float prob)
  {
    if (prob < 0 || prob > 1) {
      std::cerr << "MRandom::flip: prob is out of range : " << prob << std::endl;
      abort();
    }

    return (real(0, 1) <= prob);
  }

NS_IZENELIB_UTIL_END
