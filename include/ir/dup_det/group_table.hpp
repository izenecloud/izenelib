/**
   @file group_table.hpp
   @author Kevin Hu
   @date 2009.11.25
 */
#ifndef GROUP_TABLE_HPP
#define GROUP_TABLE_HPP

#include <types.h>
#include <ir/dup_det/integer_dyn_array.hpp>
#include <ir/dup_det/prime_gen.hpp>

NS_IZENELIB_IR_BEGIN

/**
   @class GroupTable
   @brief A table to provide a fast lookup to docids that within the same group.
 */
template <
  uint32_t ENTRY_SIZE = 1000000,
  class  UNIT_TYPE = uint64_t
  >
class GroupTable
{
public:
  typedef izenelib::am::IntegerDynArray<uint32_t> Vector32;
  typedef izenelib::am::IntegerDynArray<Vector32*> Vector32Ptr;
  typedef Vector32::size_t size_t;
  typedef GroupTable<ENTRY_SIZE, UNIT_TYPE> SelfT;
  
protected:
  Vector32Ptr gids_;//!< store group pointer
  Vector32Ptr doc_hash_;//!< docid hash table entry
  FILE* f_;
  Vector32    empty_q_;//!< store empty group that's ready for using.
  
  /**
     @brief set group id of the doc with ID as docid
   */
  inline void set_gid(uint32_t docid, size_t gid)
  {
    Vector32* p = doc_hash_.at(docid%ENTRY_SIZE);
    IASSERT (p != NULL);
    
    for (size_t i=0; i<p->length(); i++)
    {
      if (p->at(i++) == docid)
      {
        (*p)[i] = gid;
        return;
      } 
    }
  }

  inline void insert(uint32_t docid, size_t gid)
  {
    uint32_t h = docid%ENTRY_SIZE;
    Vector32* p = doc_hash_.at(h);
    if (p == NULL)
      doc_hash_[h] = new Vector32();

    doc_hash_.at(h)->push_back(docid);
    doc_hash_.at(h)->push_back(gid);
    
    if (gids_.at(gid) == NULL)
      gids_[gid] = new Vector32();
    
    gids_.at(gid)->push_back(docid);
    
  }

  /**
     @brief merge 2 groups that share same documents.
   */
  inline void merge(size_t gid1, size_t gid2)
  {
    Vector32* p1 = gids_.at(gid1);
    Vector32* p2 = gids_.at(gid2);
    IASSERT(p1 != NULL);
    IASSERT(p2 != NULL);

    //swap p1 p2, small one is merged into big one. p1<p2. p1->p2
    if (p1->length()>p2->length())
    {
      Vector32* p = p1;
      p1 = p2;
      p2 = p;
      size_t g = gid1;
      gid1 = gid2;
      gid2 = g;
    }

    size_t p2_len = p2->length();
    memcpy(p2->array(p2_len+p1->length())+p2_len, p1->data(), p1->size());
    for (size_t i=0; i<p1->length(); i++)
      set_gid(p1->at(i), gid2);

    delete p1;
    gids_[gid1] = NULL;
    empty_q_.push_back(gid1);
  }

  /**
     @brief get some available group.
     @return group ID.
   */
  inline size_t get_empty_gid()
  {
    if (empty_q_.length()>0)
      return empty_q_.pop_back();

    gids_.push_back(NULL);
    return gids_.length()-1;
  }
  
public:
  inline GroupTable(const char* filenm)
  {
    doc_hash_.reserve(ENTRY_SIZE);
    for (uint32_t i=0; i<ENTRY_SIZE; i++)
      doc_hash_.add_tail(NULL);

    f_ = fopen(filenm, "r+");
    if (f_ == NULL)
    {
      f_ = fopen(filenm, "w+");
      if (f_ == NULL)
      {
        std::cout<<"Can't create file: "<<filenm<<std::endl;
        return;
      }
    }
    else
    {
      std::cout<<"Loading....\n";
      load();
    }
  }

  inline ~GroupTable()
  {
    fclose(f_);
    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;
      delete v;
      doc_hash_[i] = NULL;
    }

    for (size_t i=0; i<gids_.length(); i++)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;
      delete v;
      gids_[i] = NULL;
    }
  }

  /**
     @brief get group id of the doc with ID as docid
     @return group ID
   */
  inline size_t get_gid(uint32_t docid)const
  {
    Vector32* p = doc_hash_.at(docid%ENTRY_SIZE);
    if (p == NULL)
      return -1;
    
    for (size_t i=0; i<p->length(); i++)
    {
      if (p->at(i++) == docid)
        return p->at(i);
    }

    return -1;
  }

  inline void reset(const char* filenm)
  {
    fclose(f_);
    f_ = fopen(filenm, "w+");
    if (f_ == NULL)
    {
      std::cout<<"Can't create file: "<<filenm<<std::endl;
      return;
    }

    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;
      delete v;
      doc_hash_[i] = NULL;
    }
    doc_hash_.reset();

    for (size_t i=0; i<gids_.length(); i++)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;
      delete v;
      gids_[i] = NULL;
    }
    gids_.reset();
  }

  inline void assign(const SelfT& other)
  {
    memcpy(doc_hash_.array(other.doc_hash_.length()),
           other.doc_hash_.data(), other.doc_hash_.size());
    
    memcpy(gids_.array(other.gids_.length()),
           other.gids_.data(), other.gids_.size());
    
    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;
      
      doc_hash_[i] = new Vector32();
      memcpy(doc_hash_.at(i)->array(v->length()),
             v->data(), v->size());
    }

    for (size_t i=0; i<gids_.length(); i++)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;
      
      gids_[i] = new Vector32();
      memcpy(gids_.at(i)->array(v->length()),
             v->data(), v->size());
    }
  }
  
  inline void load()
  {
    size_t index = 0;
    fseek(f_, 0, SEEK_SET);
    if (fread(&index, sizeof(size_t), 1, f_)!=1)
      return;
    //std::cout<<index<<std::endl;
    
    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      if (index != i)
        continue;

      size_t len = 0;
      fread(&len, sizeof(size_t), 1, f_);
      doc_hash_[i] = new Vector32();
      fread(doc_hash_.at(i)->array(len), len*sizeof(uint32_t), 1, f_);
      fread(&index, sizeof(size_t), 1, f_);
      
      //std::cout<<index<<" "<<len<<std::endl;
    }

    fread(&index, sizeof(size_t), 1, f_);
    gids_.reserve(index);
    for (size_t i=0; i<index; i++)
      gids_.add_tail(NULL);

    fread(&index, sizeof(size_t), 1, f_);
    for (size_t i=0; i<gids_.length(); i++)
    {
      if (index != i)
        continue;

      size_t len = 0;
      fread(&len, sizeof(size_t), 1, f_);
      gids_[i] = new Vector32();
      fread(gids_.at(i)->array(len), len*sizeof(uint32_t), 1, f_);
      fread(&index, sizeof(size_t), 1, f_);
    }
  }

  inline void flush()
  {
    if (gids_.length()==0)
      return;
    
    fseek(f_, 0, SEEK_SET);

    size_t i=0;
    for (; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;

      fwrite(&i, sizeof(size_t), 1, f_);
      size_t len = v->length();
      fwrite(&len, sizeof(size_t), 1, f_);
      fwrite(v->data(), v->size(), 1, f_);
      //std::cout<<i<<" "<<len<<std::endl;
    }
    
    i = -1;
    fwrite(&i, sizeof(size_t), 1, f_);
    i = gids_.length();
    fwrite(&i, sizeof(size_t), 1, f_);

    for (i=0; i<gids_.length(); i++)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;

      fwrite(&i, sizeof(size_t), 1, f_);
      size_t len = v->length();
      fwrite(&len, sizeof(size_t), 1, f_);
      fwrite(v->data(), v->size(), 1, f_);
    }
    i = -1;
    fwrite(&i, sizeof(size_t), 1, f_);
    
    fflush(f_);
  }

  inline void add_doc(uint32_t docID1, uint32_t docID2)
  {
//	  std::cout<<"doc1: "<<docID1<<" , doc2: "<<docID2<<std::endl;
    size_t t1 = get_gid(docID1);
    size_t t2 = get_gid(docID2);

    if (t1 == (size_t )-1 && t2 != (size_t )-1 )
    {
      insert(docID1, t2);
      return;
    }
    
    if (t2 == (size_t )-1 && t1 != (size_t )-1 )
    {
      insert(docID2, t1);
      return;
    }

    if (t1 == (size_t )-1 && t2 == (size_t )-1 )
    {
      size_t g = get_empty_gid();
      insert(docID1, g);
      insert(docID2, g);
      return;
    }

    if(t1 == t2)
      return;
    
    merge(t1, t2);
  }

  /**
     @brief to assign this doc a new group
   */
  inline void add_doc(uint32_t docid)
  {
    if (get_gid(docid) != (size_t)-1)
      return;

    insert(docid, get_empty_gid());
  }
  
  inline bool exist(uint32_t docid)const
  {
    size_t gid = get_gid(docid);
    if (gid == (size_t )-1)
      return false;
    if (gid >= gids_.length() || gids_.at(gid)== NULL)
      return false;
    return true;
  }

  /**
     @return a reference of vector of docids that are in the same group.
   */
  inline const Vector32& find(uint32_t docid)const
  {
    static Vector32 v;
    size_t gid = get_gid(docid);
    if (gid == (size_t)-1)
      return v;

    Vector32* p = gids_.at(gid);
    if (p== NULL)
      return v;
    
    return *p;
  }

  inline void compact()
  {
    gids_.compact();
    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;
      v->compact();
    }

    for (size_t i=0; i<gids_.length(); i++)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;
      v->compact();
      std::cout<<"group: "<<i<<" "<<*v<<std::endl;
    }
    
  }

  inline bool same_group(uint32_t docid1, uint32_t docid2)const
  {
    size_t t1 = get_gid(docid1);
    size_t t2 = get_gid(docid2);

    return t1 == t2 && t1 != (size_t)-1;
  }

  inline size_t group_length()const
  {
    return gids_.length();
  }

  /**
     @brief reset docids by FpList
   */
  template<
    class FpList
    >
  void set_docid(FpList& fp)
  {
    for (size_t i=0; i<doc_hash_.length(); i++)
    {
      Vector32* v = doc_hash_.at(i);
      if (v==NULL)
        continue;
      delete v;
      doc_hash_[i] = NULL;
    }

    for (size_t i=0; i<gids_.length(); ++i)
    {
      Vector32* v = gids_.at(i);
      if (v==NULL)
        continue;

      for(size_t j=0; j<v->length(); ++j)
      {
        (*v)[j] = fp[(*v)[j]];
        
        uint32_t h =  (*v)[j]%ENTRY_SIZE;
        Vector32* p = doc_hash_.at(h);
        if (p == NULL)
          doc_hash_[h] = new Vector32();
        
        doc_hash_.at(h)->push_back((*v)[j]);
        doc_hash_.at(h)->push_back(i);
      }
    }

    compact();
  }
  
  /**
   *This is for outputing into std::ostream, say, std::cout.
   **/
friend std::ostream& operator << (std::ostream& os, const SelfT& v)
  {
    os<< "\n-------- Doc hash table -------\n";
    
    for (size_t i =0; i<v.doc_hash_.length(); i++)
    {
      Vector32* p = v.doc_hash_.at(i);
      if (p==NULL)
        continue;
      
      os<<(*p)<<"\n";
    }

    os<< "\n-------- Group table -------\n";
    
    for (size_t i =0; i<v.gids_.length(); i++)
    {
      Vector32* p = v.gids_.at(i);
      if (p==NULL)
        continue;
      
      os<<(*p)<<"\n";
    }


    return os;
  }

  
}
  ;

NS_IZENELIB_IR_END
#endif
