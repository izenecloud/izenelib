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
uint64_t CODEPACKING_POS_BEGIN = 0xEFE5AAE90; \
uint32_t CODEPACKING_LEN_BEGIN = 0xEFE1685; \
alphabeta::time::RR(CODEPACKING_POS_BEGIN, CODEPACKING_LEN_BEGIN);

#define CODEPACKING_END() \
uint64_t CODEPACKING_POS_END = 0xEFE5AAE90; \
uint32_t CODEPACKING_LEN_END = 0xEFE1685; \
alphabeta::time::HR(CODEPACKING_POS_END, CODEPACKING_LEN_END);


#endif 
