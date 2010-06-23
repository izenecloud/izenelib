/**
 * @file BoyerMoore.h
 * @brief The header file of BoyerMoore algorithm implementmentation.
 *
 * This file defines an BoyerMoore class, which implements the BoyerMoore algorithm.
 */


#ifndef BoyerMoore_h
#define BoyerMoore_h 1

#include <wiselib/YString.h>

namespace wiselib {
  /**
   * @brief Simple wrapper for BoyerMoore algorithm.
   *
   */
  class BoyerMoore {
  public:
    const char* x;
    int m;
    int *bm_gs;
    int bm_bc[256];

    BoyerMoore(const YString& pattern);
    BoyerMoore(const BoyerMoore& bm);

    ~BoyerMoore() { delete bm_gs; }

    BoyerMoore& operator=(const BoyerMoore& bm);
    int find(const char *y, int n) const;

  private:
    void pre_gs();
    void pre_bc();
    void copy(const BoyerMoore& bm);
  };
};
#endif
