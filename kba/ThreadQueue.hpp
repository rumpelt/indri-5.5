#ifndef THREADQUEUE_HPP
#define THREADQUEUE_HPP
#include <mutex>
#include <memory>
#include <condition_variable>
#include <queue>
#include <vector>

template<class T> class Threadsafe_Queue {
private:
  mutable std::mutex mut;
  std::queue<std::shared_ptr<T> > data_queue;
  std::condition_variable data_cond;
  
  /**
   * The maximum data a queue can hold. This is to avoid memory bloating
   */
  int max_size;
  /**
   * This is not thread safe
   */
  bool not_empty(); 
public:
  std::shared_ptr<T> wait_pop();
  
  void pushdata(T newValue) {
    std::shared_ptr<T> data(std::make_shared<T>(std::move(newValue)));
    std::lock_guard<std::mutex> lk(mut);
    data_queue.push(data);
    data_cond.notify_one();
  }
  
  
  void pushValues(std::vector<T> newValues) {
    std::lock_guard<std::mutex> lk(mut);
    for (size_t idx = 0; idx < newValues.size(); idx++ ) {
      T value = newValues[idx];
      std::shared_ptr<T> data(std::make_shared<T>(std::move(value)));
      data_queue.push(data); 
    }
    data_cond.notify_all();
  }

};
#endif 
