///
/// @file IRDocument.h
/// @brief Document item used by IRDatabase
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2009
/// @date Updated 2009-11-21 00:56:04
/// @date Refined 2009-11-24 Jinglei
///
#ifndef __IRDOCUMENT_H
#define __IRDOCUMENT_H

#include <string>
#include <vector>

#include <boost/any.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>

#include "IRConstant.hpp"
#include "type_defs.h"

NS_IZENELIB_IR_BEGIN
namespace irdb
{
    
    
    ///@brief terms in document
    class IRTerm
    {
        typedef IRConstant C;
        public:
            IRTerm()
            {
            }
            IRTerm(termid_t termId, const std::string& field = C::kDefaultFieldName()):termId_(termId), field_(field), position_(0)
            {
                
            }
            
            IRTerm(termid_t termId, loc_t position):termId_(termId), field_(C::kDefaultFieldName()), position_(position)
            {
                
            }
            
            
            IRTerm(const IRTerm& rhs):termId_(rhs.termId_), field_(rhs.field_), position_(rhs.position_)
            {
            }
            
            IRTerm& operator=(const IRTerm& rhs)
            {
                termId_ = rhs.termId_;
                field_ = rhs.field_;
                position_ = rhs.position_;
                return *this;
            }
            
            bool operator < (const IRTerm& term) const
            {
                return field_ < term.field_;
            }
            
        public:
            termid_t termId_;
            std::string field_;
            loc_t position_;
    };
    
    typedef std::vector<IRTerm >::const_iterator IRTermIterator;
    
    template <typename MPL_VECTOR>
    class IRDocument
    {
        typedef typename MPL_VECTOR::type VEC_TYPE;
        public:
            IRDocument() 
            :terms_(0),dataSize_(boost::mpl::size<MPL_VECTOR>::value),data_(dataSize_),hasData_(dataSize_, false)
            {
            }
            
            IRDocument(const std::vector<IRTerm >& terms)
            :terms_(terms),dataSize_(boost::mpl::size<MPL_VECTOR>::value),data_(dataSize_),hasData_(dataSize_, false)
            {
            }
            
            IRDocument& operator=(const IRDocument& rhs)
            {
                terms_ = rhs.terms_;
                dataSize_ = rhs.dataSize_;
                data_ = rhs.data_;
                hasData_ = rhs.hasData_;
                return *this;
            }
            
            ~IRDocument() {}
            
        public:
            
            template <std::size_t POS>
            void setData(const typename boost::mpl::at_c<VEC_TYPE, POS>::type& data)
            {
                data_[POS] = data;
                hasData_[POS] = true;
            }

            template <std::size_t T>
            bool getData(typename boost::mpl::at_c<VEC_TYPE, T>::type& data)
            {
//                 if(!hasData_[POS]) return false;
//                 typedef typename boost::mpl::at_c<VEC_TYPE, POS>::type TYPE_POS;
//                 data = boost::any_cast<TYPE_POS>(data_[POS]);
                return true;
            }
            

            
            std::vector<boost::any>& getAllData()
            {
                return data_;
            }
            
            bool hasData(std::size_t pos)
            {
                if( pos >= hasData_.size() ) return false;
                return hasData_[pos];
            }

            
            void addTerm (const IRTerm& term)
            {
                terms_.push_back(term);
            }
            
            IRTermIterator termListBegin() const
            {
                return terms_.begin();
            }
            
            IRTermIterator termListEnd() const
            {
                return terms_.end();
            }
            
            void sortField()
            {
                std::sort(terms_.begin(),terms_.end());
            }
            
            uint32_t termSize() const
            {
                return terms_.size();
            }
            
        private:
            
            std::vector<IRTerm > terms_;
            std::size_t dataSize_;
            std::vector<boost::any> data_;
            std::vector<bool> hasData_;
            
    };
    
    
    class PureIRDocument
    {
        
        public:
            PureIRDocument() 
            :terms_(0)
            {
            }
            
            PureIRDocument(const std::vector<IRTerm >& terms)
            :terms_(terms)
            {
            }
            
            PureIRDocument& operator=(const PureIRDocument& rhs)
            {
                terms_ = rhs.terms_;
                return *this;
            }
            
            ~PureIRDocument() {}
            
        public:

            
            void addTerm (const IRTerm& term)
            {
                terms_.push_back(term);
            }
            
            IRTermIterator termListBegin() const
            {
                return terms_.begin();
            }
            
            IRTermIterator termListEnd() const
            {
                return terms_.end();
            }
            
            void sortField()
            {
                std::sort(terms_.begin(),terms_.end());
            }
            
            uint32_t termSize() const
            {
                return terms_.size();
            }
            
        private:
            
            std::vector<IRTerm > terms_;
            
    };
    
}
NS_IZENELIB_IR_END
#endif
