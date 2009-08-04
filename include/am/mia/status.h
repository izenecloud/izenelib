#ifndef STATUS_H
#define STATUS_H


#include <boost/thread/mutex.hpp>

enum MIA_STATUS
  {
    NONE = 0,
    RAW_MAP = 1,
    FORWARD_INDEX = 2,
    DUPD = 4,
    TG = 8,
    QR = 16,
    SIM = 32
  }
  ;

class MiningStatus
{
  int status_;
  boost::mutex mutex_;
  typedef boost::mutex::scoped_lock scoped_lock;


 public:
  inline MiningStatus()
    :status_(NONE)
  {
  }

  inline void set_raw_map()
  {
    scoped_lock lock(mutex_);

    status_ |= RAW_MAP;
  }

  inline bool get_raw_map()
  {
    scoped_lock lock(mutex_);

    return status_ & RAW_MAP;
  }

  inline void reset_raw_map()
  {
    scoped_lock lock(mutex_);
    status_ &= RAW_MAP^0xffff;
  }

  inline void set_forward_index()
  {
    scoped_lock lock(mutex_);
    status_ |= FORWARD_INDEX;
  }

  inline bool get_forward_index()
  {
    scoped_lock lock(mutex_);
    return status_ &FORWARD_INDEX;
  }

  inline void reset_forward_index()
  {
    scoped_lock lock(mutex_);
    status_ &= FORWARD_INDEX^0xffff;
  }
  
  inline void set_dupd()
  {
    scoped_lock lock(mutex_);
    status_ |= DUPD;
  }
  
  inline bool get_dupd()
  {
    scoped_lock lock(mutex_);
    return status_ &DUPD;
  }

  inline void reset_dupd()
  {
    scoped_lock lock(mutex_);
    status_ &= DUPD^0xffff;
  }

  inline void set_tg()
  {
    scoped_lock lock(mutex_);
    status_ |= TG;
  }

  inline bool get_tg()
  {
    scoped_lock lock(mutex_);
    return status_ & TG;
  }

  inline void reset_tg()
  {
    scoped_lock lock(mutex_);
    status_ &= TG^0xffff;
  }

  inline void set_qr()
  {
    scoped_lock lock(mutex_);
    status_ |= QR;
  }

  inline bool get_qr()
  {
    scoped_lock lock(mutex_);
    return status_ & QR;
  }

  inline void reset_qr()
  {
    scoped_lock lock(mutex_);
    status_ &= QR^0xffff;
  }

  inline void set_sim()
  {
    scoped_lock lock(mutex_);
    status_ |= SIM;
  }

  inline bool get_sim()
  {
    scoped_lock lock(mutex_);
    return status_ & SIM;
  }

  inline void reset_sim()
  {
    scoped_lock lock(mutex_);
    status_ &= SIM^0xffff;
  }

}
;

#endif
