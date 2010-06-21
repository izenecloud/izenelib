/**
 * @file random.h
 * @brief Random numbers or strings generation helper.
 */
#ifndef _WISE_RANDOM_H_
#  define _WISE_RANDOM_H_


#  ifdef __cplusplus
extern "C" {
#  endif /* __cplusplus */

void srandom2(unsigned int);
long wise_random(void);

#  ifdef __cplusplus
}
#  endif /* __cplusplus */

#endif /* _WISE_RANDOM_H_ */
