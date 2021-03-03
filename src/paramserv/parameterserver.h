#ifndef PARAMETERSERVER_H
#define PARAMETERSERVER_H

#include "configuru.hpp"
#include "simplethread.h"
#include <mutex>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include <iostream>

class ParameterServer {
 private:
  explicit ParameterServer();
  ~ParameterServer() {
    stop_server();
  }

  std::shared_ptr<Runnable> m_ServerThreadContext;
  std::shared_ptr<Thread> m_ServerThread;
  configuru::Config _cfgRoot;

 public:
  inline configuru::Config &GetCfgRoot() {
    return _cfgRoot;
  }
  inline configuru::Config &GetCfgCtrlRoot() {
    _cfgRoot.judge_or_create_key("dev_ctrl");
    return _cfgRoot["dev_ctrl"];
  }
  static ParameterServer *instance() {
    static ParameterServer *_this = nullptr;
    if (_this == nullptr) {
      _this = new ParameterServer;
      _this->init();
      _this->start_server();
    }
    return _this;
  }
  inline bool is_debug() {return debug_;}
  void init();
  void start_server();
  void stop_server();
  bool debug_;
};

#endif // PARAMETERSERVER_H
