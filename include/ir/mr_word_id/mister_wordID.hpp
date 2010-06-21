/**
   @file mister_wordID.h
   @auther Kevin Hu
   @date 2010.01.09
**/

#ifndef MISTER_WORDID_HPP
#define MISTER_WORDID_HPP

#include "term_freq.hpp"
#include "id_str_table.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <util/ustring/UString.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/function.hpp>
#include <am/graph_index/dyn_array.hpp>
#include <cmath>

NS_IZENELIB_IR_BEGIN

template<
  izenelib::util::UString::EncodingType ENCODE_TYPE = izenelib::util::UString::UTF_8,
  uint32_t START_ID = 1
  >
class MisterWordID
{
  typedef IdStringTable<uint32_t> id_table_t;
  typedef TermFrequency<> term_freq_t;
  typedef TermFrequency<> str_table_t;
  typedef boost::function< std::vector<std::string> ( const std::string&) > participle_t;
  typedef izenelib::am::DynArray<double> log_weights_t;
  
  term_freq_t term_freq_;
  id_table_t id_table_;
  str_table_t str_table_;
  std::string filenm_;
  term_freq_t stop_word_dic_;
  log_weights_t log_weights_;
  
  //participle_t participle_;

  bool init_stop_word_(const char* dicname)
  {
    FILE* f = fopen(dicname, "r");
    if (f == NULL)
      return false;

    fseek(f, 0, SEEK_END);
    uint64_t fs = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fs == 0)
      return false;
    
    uint64_t pos_s = 0;
    uint64_t pos_e = 0;
    
    char* buff = new char[fs];
    IASSERT(fread(buff, fs, 1, f)==1);
    fclose(f);

    while (pos_s < fs && pos_e<=fs)
    {
      if (buff[pos_e]!='\n')
      {
        ++pos_e;
        continue;
      }

      if (pos_e - pos_s <=1)
      {
        ++pos_e;
        pos_s = pos_e;
        continue;
      }
      
      std::string line(&buff[pos_s], pos_e - pos_s);

      stop_word_dic_.increase(line);

      ++pos_e;
      pos_s = pos_e;
    }

    delete buff;
    return true;
  }

  bool init_term_freq_(const char* dicname)
  {
    FILE* f = fopen(dicname, "r");
    if (f == NULL)
    {
      std::cout<<dicname<<" is not exist!\n";
      return false;
    }

    fseek(f, 0, SEEK_END);
    uint64_t fs = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fs == 0)
    {
      std::cout<<dicname<<" is not supposed to be empty!\n";
      return false;
    }
    
    uint64_t pos_s = 0;
    uint64_t pos_e = 0;
    
    char* buff = new char[fs];
    IASSERT(fread(buff, fs, 1, f)==1);
    fclose(f);

    while (pos_s < fs && pos_e<=fs)
    {
      if (buff[pos_e]!='\n')
      {
        ++pos_e;
        continue;
      }

      if (pos_e - pos_s <=1)
      {
        ++pos_e;
        pos_s = pos_e;
        continue;
      }
      
      std::string line(&buff[pos_s], pos_e - pos_s);
      if (stop_word_dic_.find(line)==0)
        term_freq_.increase(line);

      ++pos_e;
      pos_s = pos_e;
    }

    delete buff;
    return true;
  }

  bool traversal_corpus_(const char* corpus_dir, participle_t participle)
  {
    namespace fs = boost::filesystem;
    const static uint32_t bs = 1000000;
    
    fs::path full_path(corpus_dir, fs::native);
    if (!fs::exists(full_path))
    {
      std::cout<<corpus_dir<<" is not exist!\n";
      return false;
    }

    fs::directory_iterator item_b(full_path);
    fs::directory_iterator item_e;
    char* buff = new char[bs];
    for (; item_b!=item_e; item_b++)
    {
      if (fs::is_directory(*item_b))
        continue;

      std::string nm = item_b->path().file_string();
      std::size_t dot = nm.find_last_of('.');
      if (dot!=std::string::npos &&
          (nm.substr(dot)==".gz" ||nm.substr(dot)==".tar"
           ||nm.substr(dot)==".zip" || nm.substr(dot)==".rar"
           ||nm.substr(dot)==".bmp" ||nm.substr(dot)==".png"
           ||nm.substr(dot)==".jpg" || nm.substr(dot)==".gif"
           ||nm.substr(dot)==".rmp" ||nm.substr(dot)==".pdf"
           ||nm.substr(dot)==".ps" ||nm.substr(dot)==".ico" ))
      {
        std::cout<<"Skip to scan '"<<nm<<"'\n";
        continue;
      }
      
      std::cout<<"Start to scan '"<<nm<<"'\n";
      FILE* f = fopen(nm.c_str(), "r");
      IASSERT(f!=NULL);
      
      fseek(f, 0, SEEK_END);
      uint64_t fs = ftell(f);
      fseek(f, 0, SEEK_SET);
      const uint64_t FS = fs;

      std::cout<<std::endl;
      while (fs > 0)
      {
        std::cout<<"\rScanning ... "<<(1.-(fs*1./FS))*100.0<<"%"<<std::flush;
        uint32_t rs = bs > fs? fs: bs;
        IASSERT(fread(buff, rs, 1, f)==1);
        fs -= rs;

        //std::cout<<fs<<" "<<rs<<std::endl;
        uint32_t pos_s = 0;
        uint32_t pos_e = 0;
        std::vector<std::string> terms;
        while (pos_s < rs && pos_e<rs)
        {
          if (!(fs ==0 && pos_e==rs)&& (buff[pos_e]!='\n'||buff[pos_e]!='\r')  && pos_e-pos_s != rs-1)
          {
            ++pos_e;
            continue;
          }

          if (pos_e - pos_s <=1)
          {
            ++pos_e;
            pos_s = pos_e;
            continue;
          }
      
          std::string line(&buff[pos_s], pos_e - pos_s);
          terms = participle(line);

          for (uint32_t i=0; i<terms.size(); ++i)
            if (term_freq_.find(terms[i]))
              term_freq_.increase(terms[i]);

          ++pos_e;
          pos_s = pos_e;
        }
        
        fseek(f, -1*(int)(pos_e-pos_s), SEEK_CUR);
        fs += pos_e - pos_s;
      }

      fclose(f);
    }

    delete buff;

    return true;
  }

  struct FREQ_STRUCT
  {
    uint32_t freq;
    uint32_t idx;

    uint32_t& FREQ_()
    {
      return freq;
    }

    uint32_t& IDX_()
    {
      return idx;
    }

    uint32_t FREQ()const
    {
      return freq;
    }

    uint32_t IDX()const
    {
      return idx;
    }
  
    inline FREQ_STRUCT(uint32_t i, uint32_t j)
    {
      FREQ_() = i;
      IDX_() = j;
    }
  
    inline FREQ_STRUCT(uint32_t i)
    {
      FREQ_() = i;
      IDX_() = 0;
    }
  
    inline FREQ_STRUCT()
    {
      FREQ_() = 0;
      IDX_() = -1;
    }
  
    inline FREQ_STRUCT(const FREQ_STRUCT& other)
    {
      FREQ_() = other.FREQ();
      IDX_() = other.IDX();
    }

    inline FREQ_STRUCT& operator = (const FREQ_STRUCT& other)
    {
      FREQ_() = other.FREQ();
      IDX_() = other.IDX();
      return *this;
    }

    inline bool operator == (const FREQ_STRUCT& other)const
    {
      return (FREQ() == other.FREQ());
    }

    inline bool operator != (const FREQ_STRUCT& other)const
    {
      return (FREQ() != other.FREQ());
    }

    inline bool operator < (const FREQ_STRUCT& other)const 
    {
      return (FREQ() < other.FREQ());
    }

    inline bool operator > (const FREQ_STRUCT& other)const
    {
      return (FREQ() > other.FREQ());
    }

    inline bool operator <= (const FREQ_STRUCT& other)const 
    {
      return (FREQ() <= other.FREQ());
    }

    inline bool operator >= (const FREQ_STRUCT& other)const
    {
      return (FREQ() >= other.FREQ());
    }

    inline uint32_t operator & (uint32_t e)const
    {
      return (FREQ() & e);
    }

    /**
     *This is for outputing into std::ostream, say, std::cout.
     **/
  friend std::ostream& operator << (std::ostream& os, const FREQ_STRUCT& v)
    {
      os<<"<"<<v.FREQ()<<","<<v.IDX()<<">";

      return os;
    }
  
  }
    ;

  void sort_freq_()
  {
    typedef izenelib::am::DynArray<FREQ_STRUCT> freq_t;

    if (!term_freq_.begin())
      return;

    freq_t freqs;
    std::vector<FREQ_STRUCT> vfreqs;
    std::vector<std::string> words;
    
    std::string word;
    uint32_t freq = 0;
    while(term_freq_.next(word, freq))
    {
      vfreqs.push_back(FREQ_STRUCT(freq, words.size()));
      words.push_back(word);
    }

    std::sort(vfreqs.begin(), vfreqs.end());
    freqs.assign(vfreqs);

    for (uint32_t i= freqs.length()-1; i!=(uint32_t)-1; --i)
    {
      str_table_.insert(words[freqs.at(i).IDX()], freqs.length()-1-i+START_ID);
      id_table_.insert(freqs.length()-1-i+START_ID, words[freqs.at(i).IDX()].length(),
                       words[freqs.at(i).IDX()].c_str());
    }

    FILE* f = fopen(std::string(filenm_+".dic").c_str(), "w+");
    term_freq_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".stw").c_str(), "w+");
    stop_word_dic_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".str.id").c_str(), "w+");
    str_table_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".id.str").c_str(), "w+");
    id_table_.save(f);
    fclose(f);

    f = fopen(std::string(filenm_+".over").c_str(), "w+");
    fclose(f);
  }

  bool load_()
  {
    FILE* f = fopen(std::string(filenm_+".over").c_str(), "r");
    if (f == NULL)
      return false;
    fclose(f);
    
    f = fopen(std::string(filenm_+".stw").c_str(), "r");
    IASSERT(f!=NULL);    
    stop_word_dic_.load(f);
    fclose(f);
    
    f = fopen(std::string(filenm_+".str.id").c_str(), "r");
    IASSERT(f!=NULL);    
    str_table_.load(f);
    fclose(f);

    f = fopen(std::string(filenm_+".id.str").c_str(), "r");
    IASSERT(f!=NULL);
    id_table_.load(f);
    fclose(f);

    return true;
  }

  inline void prepare_weights_()
  {
    log_weights_.reserve(num_items());
    for (uint32_t i=0; i<num_items(); ++i)
    {
      log_weights_.push_back(log10(num_items()-i+2));
    }
  }
  
public:
  MisterWordID(const char* filenm)
    :filenm_(filenm)
  {
  }

  ~MisterWordID()
  {
  }

  bool is_ready()
  {
    if(load_())
    {
      prepare_weights_();
      return true;
    }
    return false;
  }
  
  bool prepareID(const char* dicname, const char* corpus_dir, participle_t participle, const char* sw_dic="")
  {
    init_stop_word_(sw_dic);
    
    if (!init_term_freq_(dicname))
      return false;
    if (!traversal_corpus_(corpus_dir, participle))
      return false;

    sort_freq_();

    prepare_weights_();
    return true;
  }

  uint32_t get_id(const std::string& word )const
  {
    return str_table_.find(word);
  }

  inline double get_weight(uint32_t id)const
  {
    if (id>=log_weights_.length())
      id = log_weights_.length()-1;
    
    return log_weights_.at(id);
  }

  std::string get_word(uint32_t id)const
  {
    uint32_t len = 0;
    const char* p = id_table_.find(id, len);
    return std::string(p, len);
  }

  uint32_t num_items()const
  {
    return id_table_.num_items();
  }
  
}
  ;

NS_IZENELIB_IR_END

#endif//MISTER_WORDID_H
