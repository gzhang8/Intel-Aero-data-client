
#ifndef THREADMUTEXOBJECT_H_
#define THREADMUTEXOBJECT_H_

#include <chrono>
#include <thread>             // std::thread, std::this_thread::yield
#include <mutex>              // std::mutex
#include <condition_variable> // std::condition_variable_any


template <class T>
class ThreadMutexObject
{
public:
  ThreadMutexObject()
  {}

ThreadMutexObject(T initialValue)
  : object(initialValue),
    lastCopy(initialValue)
    {}

  void assignValue(T newValue)
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    object = lastCopy = newValue;

    lock.unlock();
  }

  void assign(T newValue)
  {
    assignValue(newValue);
  }

  std::mutex & getMutex()
  {
    return mutex;
  }

  T & getReference()
  {
    return object;
  }

  void assignAndNotifyAll(T newValue)
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    object = newValue;

    signal.notify_all();

    lock.unlock();
  }
        
  void notifyAll()
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    signal.notify_all();

    lock.unlock();
  }

  T getValue()
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    lastCopy = object;

    lock.unlock();

    return lastCopy;
  }

  T waitForSignal()
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    signal.wait(mutex);

    lastCopy = object;

    lock.unlock();

    return lastCopy;
  }

  T getValueWait(int wait = 33000)
  {
    //boost::this_thread::sleep(boost::posix_time::microseconds(wait));
    std::this_thread::sleep_for(std::chrono::microseconds(wait));

    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    lastCopy = object;

    lock.unlock();

    return lastCopy;
  }

  T & getReferenceWait(int wait = 33000)
  {
    //boost::this_thread::sleep(boost::posix_time::microseconds(wait));
    std::this_thread::sleep_for(std::chrono::microseconds(wait));

    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    lastCopy = object;

    lock.unlock();

    return lastCopy;
  }

  void operator++(int)
  {
    //boost::mutex::scoped_lock lock(mutex);
    std::unique_lock<std::mutex> lock(mutex);

    object++;

    lock.unlock();
  }

private:
  T object;
  T lastCopy;
  std::mutex mutex;
  //boost::condition_variable_any signal;
  std::condition_variable_any signal;
};

#endif /* THREADMUTEXOBJECT_H_ */
