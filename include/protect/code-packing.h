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

namespace codepacking
{

class Algorithm
{
public:
    static void HR(int64_t pos, uint32_t len);
    static void RR(int64_t pos, uint32_t len);

    
};

}

#endif 
