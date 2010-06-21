/**
 * @file  MRandom.h
 * @brief Defines the function for manipulating random strings or numbers.
 */

#ifndef _MRandom_h
#define _MRandom_h 1

#include <types.h>

#include <limits.h>

NS_IZENELIB_UTIL_BEGIN
/**
 * @brief The wrapper of MRandom class.
 *
 * It will be used for manipulating random strings or numbers.
 */
class MRandom {
 public:
  MRandom();
  MRandom(unsigned int seed);
  ~MRandom() {}      // nothing to deallocated
  void init(unsigned int seed);

  /**
   *  @brief Generates a random number in [0, count-1].
   */
  long long_int(long count);
  /**
   *  @brief Generates a random number in [low, high].
   */
  long long_int(long low, long high);
  /**
   *  @brief Generates a random number in [0, count-1].
   */
  int integer(int count);
  int operator()(int count) { return integer(count); } // Unary functor.
  int integer() { return integer(INT_MAX); }
  /**
   *  @brief Generates a integer in [low, high].
   */
  int integer(int low, int high); //
  /**
   *  @brief Generates a integer in [low, high).
   */
  float real(float low, float high);

  /**
   * @brief Returns true with the probability of prob.
   */
  int flip(float prob);
};

/** The following macro is meant to be used in class declarataions in the
 * private part.  It will define a member randNumGen which is the random
 * number generator, and provide public functions for operations on it.
 */
#define RAND_OPTIONS \
  public: \
   MRandom& rand_num_gen() {return randNumGen;} \
   void init_rand_num_gen(unsigned seed) {randNumGen.init(seed);} \
private: \
   MRandom randNumGen

NS_IZENELIB_UTIL_END
#endif
