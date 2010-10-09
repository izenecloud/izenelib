/**
 * @file CBitArray.h
 * @brief CBitArray is an abstraction of bit strings with many nice features.
 * @details The class was modified to add serialization mechanism.
 *          CBitArray uses Boost serialization mechanism. We implemented
 *          separate load/save functions. For serialization we store the
 *          values of each class attribute.
 *          Modified by: Ravshan_Khamidov on August 20, 2008
 */

#ifndef CBitArray_h
#define CBitArray_h 1

#include <types.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <util/izene_serialization.h>

#include <iostream>
#include <iomanip>
#include <vector>
#include <string.h>

// used for serialization
NS_IZENELIB_UTIL_BEGIN
    
// memory allocation methods.
void *AllocPtr(int nSize);
void *ReAllocPtr(void *p, int nSize);
void FreePtr(void *p);

#define GetBit(a, b)((((uint8_t*)a)[(b)>>3] >> ((b)&7)) & 1)
#define SetBit(a, b)(((uint8_t*)a)[(b)>>3] |= (1 << ((b)&7)))
#define ResetBit(a, b)(((uint8_t*)a)[(b)>>3] &= ~(1 << ((b)&7)))
#define XOrBit(a, b)(((uint8_t*)a)[(b)>>3] ^= (1 << ((b)&7)))

enum Operation
  {
    OR= 0x00000001,
    AND= 0x00000002,
    XOR= 0x00000004,
    COPY= 0x00000008,
    SET= 0x00000001,
    RESET= 0x00000002,
    GET= 0x00000004,
    EQUAL= 0x00000001,
    GREATER = 0x00000002,
    LESS= 0x00000004,
    NOTEQUAL= 0x00000008,
  };

/**
 * @brief CBitArray provides a lof of operations on bitwise level.
 */
class CBitArray
{
 public:
  CBitArray(bool bCompressed = false);
  CBitArray(const CBitArray &src);
  CBitArray(uint8_t* pBuffer, int nLength, bool bCompressed = false);
  virtual ~CBitArray();

 public:
  /**
   * @brief Returns byte steam pointer.
   */
  uint8_t *GetBuffer() {
    return m_pBuffer;
  }
  /**
   * @brief Returns byte steam pointer. (version with const)
   */
  const uint8_t *GetBuffer() const {
    return m_pBuffer;
  }
  /**
   * @brief Returns byte steam pointer and free internal buffer.
   */
  uint8_t *Detach();
  /**
   * @brief Attachs the object with the input buffer.
   */
  void Attach(uint8_t* pBuffer, int nLength);
  /**
   * @brief Initializes the object with the input buffer.
   */
  void Init(uint8_t* pBuffer, int nLength);
  /**
   * @brief Returns byte steam length.
   */
  int GetLength() {
    return m_nLength;
  }
  /**
   * @brief Returns byte steam length. (version with const)
   */
  const int GetLength() const {
    return m_nLength;
  }
  /**
   * @brief Check if the bitarray is empty (no ones).
   */
  bool IsEmpty();

  bool IsRangeEmpty(int nStartBit, int nEndBit);
  /**
   * @brief Set all range bits to 1.
   */
  void SetRange(int nStartBit, int nEndBit);
  /**
   * @brief Reset all range bits.
   */
  void ResetRange(int nStartBit, int nEndBit);
  /**
   * @brief Reset all bits that have ones values at the input bitarray.
   */
  void ResetAt(CBitArray *pBitArray);
  /**
   * @brief Toggle bits values of the input range.
   */
  void XOrRange(int nStartBit, int nEndBit);
  /**
   * @brief Copy bits values from the input bitarray range.
   */
  void CopyRange(const CBitArray& src, int nStartBit, int nEndBit);
  /**
   * @brief Prints the contents of CBitArray as 0 and 1's.
   */
  void display(std::ostream& stream) const;

  inline bool GetAt(int nBit) {
    if (m_bCompressed)
      return GetAt(m_pBuffer, m_nLength, nBit);
    if (nBit >= m_nLength<<3)
      return false;
    return GetBit(m_pBuffer, nBit);
  }
  inline void SetAt(int nBit) {
    if (m_bCompressed) {
      SetAt(m_pBuffer, m_nLength, nBit);
      m_nAllocLength = m_nLength;
    } else {
      if (nBit >= m_nLength<<3)
        SetLength((nBit>>3)+1);
      SetBit(m_pBuffer, nBit);
    }
    SetModified();
  }
  inline void ResetAt(int nBit) {
    if (nBit < m_nLength<<3)
      ResetBit(m_pBuffer, nBit);
    SetModified();
  }
  inline void XOrAt(int nBit) {
    if (nBit >= m_nLength<<3)
      SetLength((nBit>>3)+1);
    XOrBit(m_pBuffer, nBit);
    SetModified();
  }
  inline void SetAll() {
    memset(m_pBuffer, 0xff, m_nLength);
    SetModified();
  }
  inline void ResetAll() {
    memset(m_pBuffer, 0, m_nLength);
    SetModified();
  }
  /**
   * @brief Returns number of ones in the bitarray
   */
  int GetCount();
  /**
   * @brief Returns the number of agreed bits between bitarray and src.
   */
  int NumAgreedBits(int nBitsSize, const CBitArray& src) const {
    return (nBitsSize - (*this ^ src).GetCount());
  }

  int GetRangeCount(int nStartBit, int nEndBit);
  // returns number of ones in the range of the bitarray
  void SetLength(int nLength);
  // sets bitarray byte steam length
  void FreeBuffer();
  // frees internal buffer
  int GetIndexBit(int nIndex);
  // returns the bit number of the one index
  int GetBitIndex(int nBit);
  // returns the index of the one in the input bit number
  void Delete(int nStart, int nEnd);
  // deletes the input range from bits stream
  void Insert(int nStart, int nCount, bool bSet);
  // insert values in a specified start bit

  inline bool operator[](int nBit) {
    return GetAt(nBit);
  }

  void swap(CBitArray& rhs);
  void operator=(const CBitArray& src);
  void operator|=(const CBitArray& src);
  void operator&=(const CBitArray& src);
  void operator^=(const CBitArray& src);

  bool operator==(const CBitArray& src) const;
  bool operator!=(const CBitArray& src) const;
  bool operator&&(const CBitArray& src) const;
  CBitArray operator&(const CBitArray& src) const;
  CBitArray operator|(const CBitArray& src) const;
  CBitArray operator^(const CBitArray& src) const;
  void Compress();
  void Decompress();

  static void Compress(uint8_t *src, int nSrcLen, uint8_t *&des, int &ndesLen);
  static void Decompress(uint8_t *&src, int &nSrcLen, int nMaxLen = -1);
  static int DecompressLength(uint8_t *src, int nSrcLen);
  static bool SetAt(uint8_t *&src, int &nSrcLen, int nBit);
  static bool GetAt(uint8_t *src, int nSrcLen, int nBit);
  inline static void Mem(Operation op, uint8_t *src, int srcLen, uint8_t *&des,
                         int &desLen) {
    if (op == AND) {
      desLen = std::min(desLen, srcLen);
      for (int nByte = 0; nByte < desLen; nByte++)
        des[nByte] &= src[nByte];
      while (desLen && des[desLen-1] == 0)
        desLen--;
      return;
    }
    if (desLen < srcLen) {
      if (desLen == 0 || des == NULL)
        des = (uint8_t*)AllocPtr(srcLen);
      else
        des = (uint8_t*)ReAllocPtr(des, srcLen);
      desLen = srcLen;
    }
    if (op == COPY)
      memcpy(des, src, srcLen);
    else if (op == OR)
      for (int nByte = 0; nByte < srcLen; nByte++)
        des[nByte] |= src[nByte];
    else if (op == XOR)
      for (int nByte = 0; nByte < srcLen; nByte++)
        des[nByte] ^= src[nByte];
  }
  inline static void Mem(Operation op, CBitArray &src, CBitArray &des) {
    if (op == AND)
      des &= src;
    else if (op == COPY)
      des = src;
    else if (op == OR)
      des |= src;
    else if (op == XOR)
      des ^= src;
  }

  inline static void BitsNot(uint8_t *&p, int &nLength, int nNewBitsLen) {
    int nNewLen = (nNewBitsLen+7)/8;
    if (nNewLen > nLength) {
      if (nLength == 0)
        p = (uint8_t*)AllocPtr(nNewLen);
      else
        p = (uint8_t*)ReAllocPtr(p, nNewLen);
      nLength = nNewLen;
    }
    for (int nByte = 0; nByte < nNewLen; nByte++)
      p[nByte] = ~p[nByte];
    for (int nBit = nNewBitsLen; nBit < nNewLen*8; nBit++)
      ResetBit(p, nBit);
  }

  inline static int GetBitsValue(const uint8_t *p, int nStartBit, int nBitCount) {
    int nValue = 0;
    for (int nBit = nStartBit; nBitCount--; nBit++)
      if (GetBit(p, nBit))
        SetBit(&nValue, nBit-nStartBit);
    return nValue;
  }
  
  template<typename IntType>
  inline static IntType GetBitsValue(const uint8_t *p, int nStartBit, int nBitCount) {
    IntType nValue = 0;
    for (int nBit = nStartBit; nBitCount--; nBit++)
      if (GetBit(p, nBit))
        SetBit(&nValue, nBit-nStartBit);
    return nValue;
  }
  
  
  template<typename IntType>
  inline static IntType GetBitsValue(const uint8_t *p, const std::vector<std::pair<int, int> >& nStartCount) {
    IntType nValue = 0;
    int pos = 0;
    for ( uint32_t i=0; i<nStartCount.size(); i++)
    {
      int nStartBit = nStartCount[i].first;
      int nBitCount = nStartCount[i].second;
      for (int nBit = nStartBit; nBitCount--; nBit++)
      {
        if (GetBit(p, nBit))
          SetBit(&nValue, pos);
        ++pos;
      }
    }
    return nValue;
  }

  inline static void SetAt(uint8_t *src, int nSrcStartBit, uint8_t *des,
                           int nDesStartBit, int nBitCount) {
    while (nBitCount--) {
      if (GetBit(src, nSrcStartBit))
        SetBit(des, nDesStartBit);
      else
        ResetBit(des, nDesStartBit);
      nSrcStartBit++, nDesStartBit++;
    }
  }

  inline bool IsModified() {
    return m_bModified;
  }

  inline void SetModified(bool bModified) {
    m_bModified = bModified;
  }

  inline bool IsCompressed() {
    return m_bCompressed;
  }

  inline void SetCompressed(bool bCompressed) {
    m_bCompressed = bCompressed;
  }

  void Invert(int nMaxBits);
  // inverts all bits values to the input bit number
  int GetHeadBit();
  // return first '1' bit number
  int GetTailBit();
  // return last '1' bit number
  int GetActualBit(int dwIndexBit);
  int Bmp2Array(int *&pBuffer, bool bAllocated = false);
  int Bmp2Array(std::vector<int> &nIntArray);
  void Append2Array(std::vector<int> &nIntArray);
  int Range2Array(int nStartBit, int nEndBit, std::vector<int> &nIntArray);

 protected:
  void InitValues();
  void SetModified();
  void Index();

 protected:
  uint8_t* m_pBuffer;// byte steam pointer
  int m_nLength, m_nAllocLength;
  int m_nCount;// '1's count in the bits stream
  int *m_nIndexes, m_nIndexesCount;
  int m_nBitSeg;
  bool m_bModified;
  bool m_bCompressed;

 private:
  // Serialization mechanism is implemented
  // in the following part of code
  friend class boost::serialization::access;

  /***
   * @brief Serialization function that saves the contents
   *        of CBitArray class
   */
  template<class Archive> void save(Archive & ar, const unsigned int version) const {
    ar & m_nLength;
    ar & m_nAllocLength;

    // serialize the content of byte stream
    // by writing each value

    // for(int i = 0; i < m_nLength; i++)
    //      ar & m_pBuffer[i];

    ar & boost::serialization::make_array(m_pBuffer, m_nLength);

    // note that we did not serialize
    // m_nIndexes and m_nIndexesCount
    // since there is some bug with it
    // as a result m_nIndexesCount has
    // garbage value
    // Actually we do not need this attribute
    // and its contents

    ar & m_nCount;
    ar & m_nBitSeg;
    ar & m_bModified;
    ar & m_bCompressed;
  }

  /***
   * @brief Serialization function that loads the contents
   *        into CBitArray class
   */
  template<class Archive> void load(Archive & ar, const unsigned int version) {
    ar & m_nLength;
    ar & m_nAllocLength;

    m_pBuffer = (uint8_t*)AllocPtr(m_nLength);

    ar & boost::serialization::make_array(m_pBuffer, m_nLength);

    //  for(int i = 0; i < m_nLength; i++)
    //     ar & m_pBuffer[i];

    ar & m_nCount;
    ar & m_nBitSeg;
    ar & m_bModified;
    ar & m_bCompressed;
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()

    template<class DataIO> friend
    void DataIO_saveObject(DataIO& ar, const CBitArray& x) {
    ar & x.m_nLength;
    ar & x.m_nAllocLength;

    // serialize the content of byte stream
    // by writing each value


    // for(int i = 0; i < x.m_nLength; i++)
    //  ar & x.m_pBuffer[i];

    ar.ensureWrite(x.m_pBuffer, sizeof(uint8_t) * x.m_nLength);

    // note that we did not serialize
    // m_nIndexes and m_nIndexesCount
    // since there is some bug with it
    // as a result m_nIndexesCount has
    // garbage value
    // Actually we do not need this attribute
    // and its contents

    ar & x.m_nCount;
    ar & x.m_nBitSeg;
    ar & (int)x.m_bModified;
    ar & (int)x.m_bCompressed;
  }

  template<class DataIO> friend
    void DataIO_loadObject(DataIO& ar, CBitArray& x) {
    ar & x.m_nLength;
    ar & x.m_nAllocLength;

    x.m_pBuffer = (uint8_t*)AllocPtr(x.m_nLength);

    //for(int i = 0; i < x.m_nLength; i++)
    //     ar & x.m_pBuffer[i];

    ar.ensureRead(x.m_pBuffer, sizeof(uint8_t) * x.m_nLength);

    ar & x.m_nCount;
    ar & x.m_nBitSeg;
		
    int a, b;
    ar & a;
    ar & b;
		
    x.m_bModified = (bool)a;
    x.m_bCompressed = (bool)b;
		 
    //ar & x.m_bModified;
    //ar & x.m_bCompressed;
  }

};

inline void swap(CBitArray& a, CBitArray& b)
{
  a.swap(b);
}

//DEF_DISPLAY(CBitArray);

NS_IZENELIB_UTIL_END
#endif

