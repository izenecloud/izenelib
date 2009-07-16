#ifndef FingerPrinter_h
#define FingerPrinter_h 1

#include <types.h>
#include <util/mrandom.h>

NS_IZENELIB_IR_BEGIN
/**
 * @brief generate hash value of a string, use Finger printer
 */
class FingerPrinter {
private:
    /**
     * @brief seed when generate the hash
     */
    uint64_t seed;
  /**
   * @brief to generate a default seed
   */
    static izenelib::util::MRandom rand;
  enum {RAND_INIT = 47u};
public:
  /**
   * @brief default constructor
   */
  inline FingerPrinter();
  inline FingerPrinter(uint64_t s):seed(s){}
  /**
   * @brief deconstructor
   */
  inline ~FingerPrinter() {}
  /**
   * @brief set a new seed
   *
   * @param s seed to set
   */
  inline void set_seed(uint64_t s) {
      seed = s;
  }

  inline static void reset_rand(unsigned int init_seed = RAND_INIT) {
      rand.init(init_seed);
  }

  inline void set_seed();
  /**
   * @brief generate the string's hash value
   *
   * @param data source string
   * @param len source string length
   *
   * @return hash value
   */
  uint64_t fp(const char* data, int len) const;
};

NS_IZENELIB_IR_END
#endif
