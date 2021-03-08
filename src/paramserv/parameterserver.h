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
#define MAX_ROOT_NODE_COUNT (256)

typedef  struct config_root {
  std::string name;
  configuru::Config config;
} CFG_ROOT;

class ParameterServer {
 private:
  explicit ParameterServer();
  ~ParameterServer() {
    stop_server();
  }

  std::shared_ptr<Runnable> m_ServerThreadContext;
  std::shared_ptr<Thread> m_ServerThread;
  configuru::Config _cfgRoot;
  std::vector<CFG_ROOT> _root_nodes;
  configuru::Config _null;
  size_t _index;

 public:
  bool CreateNewRoot(const std::string &name,
      configuru::Config &&config = configuru::Config::object());
  configuru::Config &GetRoot(const std::string &name) ;
  bool RemoveRoot(const std::string &name) ;
  bool SetCurrentRoot(const std::string &name);
  bool SetCurrentRoot(size_t index = 0);
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
