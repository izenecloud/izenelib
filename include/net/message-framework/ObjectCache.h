#ifndef _OBJECT_CACHE_H_
#define _OBJECT_CACHE_H_

#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

template<typename Object>
class DefaultAlloc {

public:

  inline static Object* new_object() {
    return new Object();
  }

}

template<typename Object, typename Alloc = DefaultAlloc<Object> >
class ObjectCache : private boost::noncopyable{

public:

  inline static ObjectCache<Object>& getInstance() {
    if( instance_ == NULL )
      instance_ = new ObjectCache<Object>();
    return *instance_;
  }

  boost::shared_ptr<Object> get();

  void put(boost::shared_ptr<Object> obj);


private:

  ObjectCache();

  static ObjectCache<Object>* instance_;

  static const int CACHE_LIMIT_ = 256;

  boost::mutex object_pool_mutex_;

  std::set<boost::shared_ptr<Object> > object_pool_;
};

template<typename Object, typename Alloc>
ObjectCache<Object>* ObjectCache<Object>::instance_;

template<typename Object, typename Alloc>
ObjectCache<Object>::ObjectCache(){}

template<typename Object, typename Alloc>
boost::shared_ptr<Object> ObjectCache<Object>::get(){
  boost::mutex::scoped_lock lock(object_pool_mutex_);
  if(object_pool_.empty()) {
    return boost::shared_ptr<Object>(Alloc::new_object());
  }
  typename std::set<boost::shared_ptr<Object> >::iterator it;
  it = object_pool_.begin();
  object_pool_.erase(it);
  return *it;
}

template<typename Object, typename Alloc>
void ObjectCache<Object>::put(boost::shared_ptr<Object> obj){
   boost::mutex::scoped_lock lock(object_pool_mutex_);
   if(object_pool_.size() <= CACHE_LIMIT_)
    object_pool_.insert(obj);
}

#endif
