/** Copyright 2021 Tiny Oh, Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
#include "windllsupport.h"
#define MAX_ROOT_NODE_COUNT (256)

typedef  struct config_root {
  std::string name;
  configuru::Config config;
} CFG_ROOT;

class CLASS_DECLSPEC ParameterServer {
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
  configuru::Config &GetRootOrCreate(const std::string &name,
      configuru::Config &&config);
  bool RemoveRoot(const std::string &name) ;
  bool SetCurrentRoot(const std::string &name);
  bool SetCurrentRoot(size_t index = 0);
  inline configuru::Config &GetCfgStatusRoot();
  inline configuru::Config &GetCfgRoot();
  inline configuru::Config &GetCfgCtrlRoot();
  static ParameterServer *instance();
  inline bool is_debug() {return debug_;}
  void init();
  void start_server();
  void stop_server();
  bool debug_;
};

#endif // PARAMETERSERVER_H
