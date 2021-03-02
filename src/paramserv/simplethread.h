#ifndef SIMPLETHREAD_H
#define SIMPLETHREAD_H
#include <thread>
#include <mutex>
#include <condition_variable>
//simple automatic unlocking mutex
typedef std::unique_lock<std::mutex> Synchronized;

//simple condition variable that encapsulates the monitor
//you need to ensure that the monitor life cycle fits
//to the condition life cycle
class Condition {
 private:
  std::mutex *mutex;
  std::condition_variable cond;
  bool ownsMutex=false;
 public:
  Condition(std::mutex &m){
    mutex=&m;
  }
  Condition(){
    mutex=new std::mutex();
    ownsMutex=true;
  }
  ~Condition(){
    if (ownsMutex) delete mutex;
  }
  void wait(){
    Synchronized l(*mutex);
    cond.wait(l);
  };
  void wait(int millis){
    Synchronized l(*mutex);
    cond.wait_for(l,std::chrono::milliseconds(millis));
  }
  void wait(Synchronized &s){
    cond.wait(s);
  }
  void wait(Synchronized &s,int millis){
    cond.wait_for(s,std::chrono::milliseconds(millis));
  }
  void notify(){
    Synchronized g(*mutex);
    cond.notify_one();
  }
  void notify(Synchronized &){
    cond.notify_one();
  }
  void notifyAll(){
    Synchronized g(*mutex);
    cond.notify_all();
  }
  void notifyAll(Synchronized &){
    cond.notify_all();
  }
};

//java like runnable interface
class Runnable : public std::enable_shared_from_this<Runnable> {
 public:
  virtual void run()=0;
  virtual void stop()=0;
};

//java like thread class
class Thread : public std::enable_shared_from_this<Thread> {
 private:
  std::shared_ptr<Runnable> _runnable;
  std::thread *mthread=NULL;
  void run() {
    _runnable->run();
  }
 public:
  Thread(std::shared_ptr<Runnable> runnable) {
      this->_runnable=runnable;
  };

  ~Thread() {
    if (mthread) delete mthread;
  }
  void start() {
    if (mthread) return;
    mthread=new std::thread([this]{this->run();});
  }
  void join() {
    if (!mthread) return;
    mthread->join();
    delete mthread;
    mthread=NULL;
  };
};


#endif /* SIMPLETHREAD_H */

