///
/// @file code-packing
/// @brief Head for code packing
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-26
/// @date Updated 2010-04-26
///

#ifndef _CODEPACKING_H_
#define _CODEPACKING_H_
#include <stdint.h>

namespace alphabeta
{

class time
{
public:
    static void HR(uint64_t pos, uint32_t len);
    static void RR(uint64_t pos, uint32_t len);

    static void HR_DIRECT(char* data, uint32_t len);
    static void RR_DIRECT(char* pos, uint32_t len);
    
};

}

#define CODEPACKING_BEGIN() \
uint64_t CODEPACKING_POS = 0xEFE5AAE90; \
uint32_t CODEPACKING_LEN = 0xEFE1685; \
alphabeta::time::RR(CODEPACKING_POS, CODEPACKING_LEN);

#define CODEPACKING_END() \
alphabeta::time::HR(CODEPACKING_POS, CODEPACKING_LEN);


#endif 
