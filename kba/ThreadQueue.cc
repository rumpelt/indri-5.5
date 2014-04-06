#include "ThreadQueue.hpp"
#include <functional>


/**
 * This is not threadsafe 
 */
template<class T> bool Threadsafe_Queue<T>::not_empty() {
  //std::lock_guard<std::mutex> lk(mut);
  return !data_queue.empty();
}

template<class T> std::shared_ptr<T> Threadsafe_Queue<T>::wait_pop() {
  std::unique_lock<std::mutex> lk(mut);
  data_cond.wait(lk, std::mem_fn(&Threadsafe_Queue::not_empty));
  std::shared_ptr<T> res = data_queue.front();
  data_queue.pop();
  return res;

}

