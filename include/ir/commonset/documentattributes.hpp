#ifndef _IZENELIB_IR_COMMONSET_DOCUMENTATTRIBUTES_HPP_
#define _IZENELIB_IR_COMMONSET_DOCUMENTATTRIBUTES_HPP_

// attribute nbits per document is constant = max attribute nbits
// structure is determined by knowing the document category, containined in the first few bits
// doc id -> category id is in separate bitmap
// category id -> ordered list of attribute ids and offset in bits
// attribute id -> length in bits
//
// filter algorithm
//   input: docid -> single valued: sorted attribute ids : attribute values
//                -> multi valued: sorted attribute ids : bit strings
//
// check that category is the same
//
// if category is not set, work out which categories have the same attribute set
//
// check that attributes in request are all present
//
// check attribute values are the same
//
// single-valued attributes
//
// multi-valued attributes
//   docid -> get attribute data
//    get category id
//    get vector of sorted attribute ids
//    if vector is different from input, return false
//    else
//      convert/compress values to attribute data
//      && data with input filter


namespace izenelib
{
namespace ir
{
namespace commonset
{

class DocumentAttributes
{
public:

    DocumentAttributes
    (
        unsigned int ncategories,
        const std::vector<unsigned int>& category_by_document,
        const std::vector<std::vector<unsigned int> >& attribute_values_by_document,
        const std::vector<std::vector<unsigned int> >& attributes_by_category,
        const std::vector<unsigned int>& nvalues_by_attribute
    )
    ndocuments_( attribute_values_by_documents ),
                 nbits_nattributes_( getNBits(nattributes) ),
                 sizeof_longlong_( sizeof(unsigned long long) ),
                 attribute_present_bitmap_( nullptr )
    {
        ndocuments_ = category_by_document.size();

        nbits_category_ = getNBits( ncategories );
        unsigned int nvalues = 1 + nbits_category_ * ndocuments_ / sizeof_longlong_;
        categories_ new unsigned long long[nvalues];
        memset( categories_, 0, nvalues );

        unsigned int nvalues = 1 + nbits_nattributes_ * ndocuments_ / sizeof_longlong_;
        attribute_present_bitmap_ = new unsigned long long[nvalues];
        memset( attribute_present_bitmap_, 0, nvalues );

        for( unsigned int iattribute = 0 ; iattribute != nattribute_values.size() ; ++iattribute )
        {
            nattribute_values_[iattribute] = nattribute_values[iattribute];
            nbits_attribute_[iattribute] = getNBits( nattribute_values_[iattribute] );
            nvalues = 1 + nbits_attribute[iattribute]_ * ndocuments_ / sizeof_longlong_;
            attribute_value_bitmap_[iattribute] = new unsigned long long[nvalues];
            memset( attribute_value_bitmap_[iattribute], 0, nvalues );
        }
    }

    ~DocumentAttributes()
    {
    }

    template<typename DocID,typename Category>
    bool isACategoryMatch( const DocID& docid, const Category& category )
    {
        return category & == category;
    }

    template<typename DocID,typename Attribute>
    bool isAnAttributesMatch( const DocID& docid, const Attribute& attribute )
    {
        return false;
    }

private:

    unsigned int nbits_[nattributes_];

    unsigned long long* attribute_value_bitmap_[nattributes_];

    unsigned int ndocuments_;

    unsigned int nbits_nattributes_;

    unsigned int sizeof_longlong_;

    unsigned long long* attribute_present_bitmap_;

    unsigned int getNBits( unsigned int nvalues )
    {
        return 1 + (unsigned int)std::log(nvalues)/log(2.0);
    }

    bool setAttributeValue( unsigned int idoc, unsigned int attribute, unsigned int value )
    {
        if( idoc >= ndocuments_ || attribute >= nattributes_ || value >= nattribute_values_[attribute] ) return false;

        attribute_present_bitmap_ &= ;

        attribute_value_bitmap_[attribute_] &= ;

        return true;
    }

    bool hasAttribute( unsigned idoc, unsigned int attribute )
    {
        if( idoc >= ndocuments_ || attribute >= nattributes_ ) return false;
        return attribute_present_bitmap_[idoc]
    }

    bool hasAttributeValue( unsigned idoc, unsigned int attribute, unsigned int value )
    {
        if( !hasAttribute( idoc, attribute ) || value >= nattribute_values_[attribute] ) return false;
        return attribute_value_bitmap_[attribute] && 1<<(value-idoc%sizeof_longlong_) ? true : false;
    }

};

}
}
}
#endif
