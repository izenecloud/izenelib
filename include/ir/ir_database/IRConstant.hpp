///
/// @file IRConstant.h
/// @brief Some constant definition for IRDocument and IRDatabase
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2009-11-23 16:15:31
/// @date Updated 2009-11-23 16:15:31
///
#ifndef __IRCONSTANT_H_
#define __IRCONSTANT_H_
#include "type_defs.h"
#include <string>
#include <vector>

NS_IZENELIB_IR_BEGIN
namespace irdb
{
    class IRConstant {
        private:
            //avoid
            IRConstant() {}
            IRConstant(const IRConstant& db) {}
            IRConstant& operator=(const IRConstant& rhs)
            {
                return *this;
            }
        public:
            
            /// @brief Return default collection id for IRDatabase.
            /// 
            /// @return The collection id.
            inline static collectionid_t kCollectionId()
            {
                return 1;
            }
            
            /// @brief Return default start field id for IRDatabase.
            /// 
            /// @return The field id.
            inline static propertyid_t kStartFieldId()
            {
                return 1;
            }
            
            /// @brief Return the default field name
            /// 
            /// @return The name.
            inline static std::string& kDefaultFieldName()
            {
                static std::string value("default");
                return value;
            }
            
            /// @brief Return the default field list.
            /// 
            /// @return The list.
            inline static std::vector<std::string>& kDefaultFields()
            {
                static std::vector<std::string> value(1, kDefaultFieldName());
                return value;
            }
            
            
    };
    
}
NS_IZENELIB_IR_END
#endif // _IRCONSTANT_H_
