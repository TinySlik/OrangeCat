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
#include <mutex>
#include <memory>
#define MAX_ROOT_NODE_COUNT (256)

class CLASS_DECLSPEC ParameterServer : public std::enable_shared_from_this<ParameterServer> {
  explicit ParameterServer();
  std::shared_ptr<Runnable> m_ServerThreadContext;
  std::shared_ptr<Thread> m_ServerThread;
  configuru::Config _cfgRoot;
  configuru::Config _null;
  size_t _index;
  void startServer();
  void stopServer();
  bool debug_;

 public:
  ~ParameterServer();
  configuru::Config &getCfgStatusRoot();
  configuru::Config &getCfgRoot();
  configuru::Config &getCfgCtrlRoot();
  static ParameterServer *instance();
  inline bool isDebug() {return debug_;}
  void init();
  std::shared_ptr<ParameterServer> create();
};

#endif // PARAMETERSERVER_H
