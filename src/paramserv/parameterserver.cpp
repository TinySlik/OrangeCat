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

#include "parameterserver.h"
#include "mongoose.h"
#include "urldecode.h"
#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD
#include <thread>
#include <chrono>
#include "easylogging++.h"
#include "udp_discovery_peer.hpp"
#include <iostream>
#include <string>


#include <string.h>
#include <map>
#include <iostream>
#include "udp_discovery_peer.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

const int kPort = 12021;
const uint64_t kApplicationId = 7681412;
const unsigned int kMulticastAddress = (236 << 24) + (255 << 16) + (255 << 8) + 123; // 236.255.255.123

#define CONFIGURU_IMPLEMENTATION 1
#define TARGET_WEB_DIR_NAME "../res/web_root"
#define CONFIGURU_JSON_PARSE_ERROR_LOG ""
#define CACHE_MAX_SIZE (128*1024)
#define STATUS_DISPLAY_TIME_INTERVAL 1000
#define DEBUG_PARAM_SERV
#define CONFIG_HIDEN_PARAM
#define WITH_HTTP_PAGE
INITIALIZE_EASYLOGGINGPP

#include "configuru.hpp"
using namespace configuru;
static char s_http_port[] = "8099";
static struct mg_serve_http_opts s_http_server_opts;
static char cache[CACHE_MAX_SIZE];

// EASY README
/* exp ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  using namespace configuru;
 *  Config cfg = Config::object();
    cfg["pi"]     = 3.14;
    cfg["array"]  = Config::array{ 1, 2, 3 };
    cfg["object"] = Config::object({
            { "key1", "value1" },
            { "key2", "value2" },
    });
 *
    Config cfg{
      {"pi",     3.14},
      {"array",  Config::array({ 1, 2, 3 })},
      {"object", {
              { "key1", "value1" },
              { "key2", "value2" },
      }},
      { "fps",   fps_usage }
    };
    -------------------------------------------------

 *  if you want to update a existed parm in code

    cfg["target_parm_index"] << your_new_param_val;

    -------------------------------------------------

 *  if you want to add your custom param in manager

    step 1:
    cfg["target_parm_index"] = your_initial_val;

    step 2:
    cfg["target_parm_index"].add_callback([this](configuru::Config &a, const configuru::Config &b)->bool{
      if (b == "a") {
        // your logic code
        return true;-----------------------
      } else if (b == "b") {              |
        // your logic code                |
        return true;------------------    |
      } else if (b == "c") {         |    |
        // your logic code           |    |
      }                              |    |
      return false;--------------    |    |
                               \|/  \|/  \|/
      // !!!!!!!!!!important!!!!!!!!!!!!
      // if you judge the new config change already work, and the config tree should update with the new parm b,then return true;
      // if not, return false will refuse the param changed with new parms b;
    });

 *  as I said, if you want to add a const value just show to the params server user.
    cfg_ctrl["target_parm_index"] = your_const_val;
    cfg_ctrl["target_parm_index"].add_callback([](configuru::Config &, const configuru::Config &)->bool{
      return false;
    });
    so you next time update this value in code should use '=' instead of '<<';
    and '<<' operate will always refuse the change when you set.

    -------------------------------------------------
 *  if you want to load the config from a file, you can use it to init your own module.
 *  read:
    cfg["target_parm_index"] = configuru::parse_file("your/path/to/config.json", configuru::JSON / configuru::CFG);
 *  write:
    configuru::dump_file("your/path/to/out_put_config.json", cfg["target_parm_index"], configuru::JSON / configuru::CFG);
    -------------------------------------------------
 *  the second level config node such as
    cfg["first_level"]["second_level"] = "your_value" will be crush when the cfg["first_level"] is not exist as a object;
    to solve this problem, you should use cfg.judge_or_create_key("first_level") to make sure the first level object is not exist or you will create it;

 *  sometimes we want to use some config but not show on the web server, then we should use hiden
    cfg["target_parm_index"].set_hiden(true);
    you also can use the index in code as useral. but on http web , wen can only set the value by add object and value.
    #define CONFIG_HIDEN_PARAM is the feature enable switch.

 *  forbidden code like this
     cfg_ctrl["a"].add_callback([] {             auto cfg_ctrl = ParamsServer::instance()-> getCtrlRoot();    cfg_ctrl["b"] << ...;            })
     cfg_ctrl["b"].add_callback([] {             auto cfg_ctrl = ParamsServer::instance()-> getCtrlRoot();    cfg_ctrl["a"] << ...;            })
     a block loop will occur when you do the logic like this.

*exp ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

configuru::Config &ParameterServer::GetCfgStatusRoot() {
  return _cfgRoot.judge_with_create_key("dev_status");
}
configuru::Config &ParameterServer::GetCfgRoot() {
  return _cfgRoot;
}
configuru::Config &ParameterServer::GetCfgCtrlRoot() {
  return _cfgRoot.judge_with_create_key("dev_ctrl");
}
ParameterServer *ParameterServer::instance() {
  static ParameterServer *_this = nullptr;
  if (_this == nullptr) {
    _this = new ParameterServer;
    _this->init();
    _this->start_server();
  }
  return _this;
}

static void handle_get_device_usage(struct mg_connection *nc) {
  // Use chunked encoding in order to avoid calculating Content-Length
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

  auto cfg = ParameterServer::instance()->GetCfgRoot();
  uint64_t mem = 0, vmem = 0, r = 0, w = 0;

  //  cpu = get_cpu_usage();
  //  get_memory_usage(&mem, &vmem);
  //  get_io_bytes(&r, &w);

  if (cfg.has_key("dev_status") &&
    cfg["dev_status"].is_object()) {
  } else {
    cfg["dev_status"]= Config::object();
  }
  auto dev_status = cfg["dev_status"];
  //  dev_status["mem"] = mem;
  //  dev_status["vmem"] = vmem;
  //  dev_status["io_r"] = r;
  //  dev_status["io_w"] = w;

  mg_printf_http_chunk(nc, dump_string(dev_status, JSON).c_str());

  // Send empty chunk, the end of response
  mg_send_http_chunk(nc, "", 0);
}

static void handle_set_dev_ctrl(struct mg_connection *nc, struct http_message *hm) {
  // Use chunked encoding in order to avoid calculating Content-Length
  char * res = urlDecode(hm->message.p);
  char *custom_head = strstr(res, "code_res=");
  char *end =  strstr(res, "HTTP/1.1");

  if (!(custom_head && end)) {
    LOG(ERROR) << __FUNCTION__ << "error";
    free(res);
    mg_http_send_error(nc, 403, nullptr);
    return;
  }

  memset(cache, 0, CACHE_MAX_SIZE);
  memcpy(cache, custom_head + 9, end - custom_head - 9);
  free(res);

  auto config_in = parse_string(cache, JSON, CONFIGURU_JSON_PARSE_ERROR_LOG);
  auto dev_ctrl = ParameterServer::instance()->GetCfgCtrlRoot();

  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
  if (Config::deep_async(dev_ctrl, config_in)) {
#ifdef DEBUG_PARAM_SERV
    LOG(INFO) << "Nothing in config changed." << std::endl;
#endif
  }
#ifdef CONFIG_HIDEN_PARAM
  mg_printf_http_chunk(nc, dump_string_with_hiden(dev_ctrl, JSON).c_str());
#else
  mg_printf_http_chunk(nc, dump_string(dev_ctrl, JSON).c_str());
#endif

  // Send empty chunk, the end of response
  mg_send_http_chunk(nc, "", 0);
}

static void handle_get_dev_ctrl(struct mg_connection *nc) {
  // Use chunked encoding in order to avoid calculating Content-Length
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

  auto dev_ctrl = ParameterServer::instance()->GetCfgCtrlRoot();
#ifdef CONFIG_HIDEN_PARAM
  mg_printf_http_chunk(nc, dump_string_with_hiden(dev_ctrl, JSON).c_str());
#else
  mg_printf_http_chunk(nc, dump_string(dev_ctrl, JSON).c_str());
#endif

  // Send empty chunk, the end of response
  mg_send_http_chunk(nc, "", 0);
}

#ifdef WITH_HTTP_PAGE
static void handle_jsonp(struct mg_connection *nc, struct http_message *hm) {
  // Use chunked encoding in order to avoid calculating Content-Length
  char *res = urlDecode(hm->message.p);
  char *custom_head = strstr(res, "jsonp");
  char *end =  strstr(res, "HTTP/1.1");
  if (!(custom_head && end)) {
    LOG(ERROR) << __FUNCTION__ << "error";
    free(res);
    mg_http_send_error(nc, 403, nullptr);
    return;
  }

  memset(cache, 0, CACHE_MAX_SIZE);
  memcpy(cache, custom_head + 6, end - custom_head - 6);
  free(res);

  LOG(INFO) << cache;

  // Use chunked encoding in order to avoid calculating Content-Length
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

  auto dev_ctrl = ParameterServer::instance()->GetCfgCtrlRoot();
  std::string og = "fn(";
#ifdef CONFIG_HIDEN_PARAM
  mg_printf_http_chunk(nc, ( og + dump_string_with_hiden(dev_ctrl, JSON) + ")") .c_str());
#else
  mg_printf_http_chunk(nc, ( og + dump_string(dev_ctrl, JSON) + ")").c_str());
#endif

  // Send empty chunk, the end of response
  mg_send_http_chunk(nc, "", 0);
}
#endif

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  switch (ev) {
    case MG_EV_HTTP_REQUEST:
      if (mg_vcmp(&hm->uri, "/get_dev_status") == 0) {
        handle_get_device_usage(nc);
      } else if (mg_vcmp(&hm->uri, "/get_dev_ctrl") == 0) {
        handle_get_dev_ctrl(nc);
      } else if (mg_vcmp(&hm->uri, "/set_dev_ctrl") == 0) {
        handle_set_dev_ctrl(nc, hm);
#ifdef WITH_HTTP_PAGE
      } else if (mg_vcmp(&hm->uri, "/jsonp") == 0) {
        handle_jsonp(nc, hm);
#endif
      } else {
        mg_serve_http(nc, hm, s_http_server_opts);  // Serve static content
      }
      break;
    default:
      break;
  }
}

typedef enum {
    INIT,
    RUNNING,
    STOP
} ThreadState;

class ServerThread : public Runnable {
private:
  std::mutex mmutex;
  Condition *condition;
  ThreadState requestedState=INIT;
  ThreadState currentState=INIT;
  int id;
  struct mg_mgr mgr;
  struct mg_connection *nc;
  cs_stat_t st;
public:
  ServerThread(int id) {
    this->id=id;
    condition=new Condition(mmutex);
#ifdef WITH_HTTP_PAGE
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    while (nc == NULL && s_http_port[3] != '0') {
      LOG(WARNING) << "Cannot bind to " << s_http_port << std::endl;
      s_http_port[3]--;
      LOG(WARNING) << "Try " << s_http_port << std::endl;
      nc = mg_bind(&mgr, s_http_port, ev_handler);
    }
    if (s_http_port[3] == '0') {
#ifdef DEBUG_PARAM_SERV
	    LOG(ERROR) << "failed";
#endif
      exit(1);
    }
#endif
  }
  ~ServerThread() {
    delete condition;
  }
  int getId() {
    return id;
  }
  void setState(ThreadState nState) {
    {
    Synchronized x(mmutex);
    requestedState=nState;
    condition->notifyAll(x);
    }
  };
  ThreadState getState() {
    Synchronized x(mmutex);
    return currentState;
  };

  virtual void run() {
    {
      Synchronized x(mmutex);
      currentState=RUNNING;
    }
#ifdef WITH_HTTP_PAGE
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = TARGET_WEB_DIR_NAME;

    if (mg_stat(s_http_server_opts.document_root, &st) != 0) {
#ifdef DEBUG_PARAM_SERV
      LOG(ERROR) << "Cannot find web_root directory, continue without params server.\n" << std::endl;
#endif
      while (requestedState!=STOP) {
        // waiting for close;
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      return;
    }
#ifdef DEBUG_PARAM_SERV
    LOG(INFO) << "Starting web server on port " << s_http_port << std::endl;
#endif
    while (requestedState!=STOP) {
      mg_mgr_poll(&mgr, STATUS_DISPLAY_TIME_INTERVAL);
    }
    currentState=STOP;
    mg_mgr_free(&mgr);
    return;
#endif
  }

  virtual void stop() {
    requestedState=STOP;
    currentState=STOP;
  }
};

class MutilCastThread : public Runnable {
private:
  std::mutex mmutex;
  Condition *condition;
  ThreadState requestedState=INIT;
  ThreadState currentState=INIT;
  int id;

public:
  MutilCastThread(int id) {
    this->id=id;
    condition=new Condition(mmutex);
  }
  ~MutilCastThread() {
    delete condition;
  }
  int getId() {
    return id;
  }
  void setState(ThreadState nState) {
    {
    Synchronized x(mmutex);
    requestedState=nState;
    condition->notifyAll(x);
    }
  };
  ThreadState getState() {
    Synchronized x(mmutex);
    return currentState;
  };

  virtual void run() {
    {
      Synchronized x(mmutex);
      currentState=RUNNING;
    }

#ifdef WITH_HTTP_PAGE
    std::string user_data = "error";
    udpdiscovery::PeerParameters parameters;
    parameters.set_multicast_group_address(kMulticastAddress);
    // parameters.set_can_use_broadcast(true);
    parameters.set_can_use_multicast(true);
    parameters.set_can_discover(true);
    parameters.set_can_be_discovered(true);
    if (parameters.can_be_discovered()) {
      user_data = s_http_port;
    }
    parameters.set_port(kPort);
    parameters.set_application_id(kApplicationId);

    udpdiscovery::Peer peer;
    if (!peer.Start(parameters, user_data)) {
#ifdef DEBUG_PARAM_SERV
      LOG(ERROR) << "MutilCast initial failed.";
#endif
      return;
    }
    std::list<udpdiscovery::DiscoveredPeer> discovered_peers;
    std::map<udpdiscovery::IpPort, std::string> last_seen_user_datas;

    while (true) {
      if (parameters.can_discover()) {
        std::list<udpdiscovery::DiscoveredPeer> new_discovered_peers = peer.ListDiscovered();
        if (!udpdiscovery::Same(parameters.same_peer_mode(), discovered_peers, new_discovered_peers)) {
          discovered_peers = new_discovered_peers;

          last_seen_user_datas.clear();
          for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
            last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));
          }

          LOG(INFO) << "Discovered peers: " << discovered_peers.size();
          for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
            LOG(INFO) << udpdiscovery::IpPortToString((*it).ip_port()) << ", " << (*it).user_data();
          }
        } else {
          bool same_user_datas = true;
          for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = new_discovered_peers.begin(); it != new_discovered_peers.end(); ++it) {
            std::map<udpdiscovery::IpPort, std::string>::const_iterator find_it = last_seen_user_datas.find((*it).ip_port());
            if (find_it != last_seen_user_datas.end()) {
              if ((*find_it).second != (*it).user_data()) {
                same_user_datas = false;
                break;
              }
            } else {
              same_user_datas = false;
              break;
            }
          }

          if (!same_user_datas) {
            discovered_peers = new_discovered_peers;

            last_seen_user_datas.clear();
            for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
              last_seen_user_datas.insert(std::make_pair((*it).ip_port(), (*it).user_data()));
            }

            LOG(INFO) << "Discovered peers: " << discovered_peers.size();
            for (std::list<udpdiscovery::DiscoveredPeer>::const_iterator it = discovered_peers.begin(); it != discovered_peers.end(); ++it) {
              LOG(INFO) << udpdiscovery::IpPortToString((*it).ip_port()) << ", " << (*it).user_data();
            }
          }
        }
#if defined(_WIN32)
        Sleep(500);
#else
        usleep(500000);
#endif
      }
    }
#endif
  }

  virtual void stop() {
    requestedState=STOP;
    currentState=STOP;
  }
};

ParameterServer::ParameterServer() :
m_ServerThreadContext(nullptr),
m_ServerThread(nullptr),
m_MutilCastThreadContext(nullptr),
m_MutilCastThread(nullptr),
_index(0) {
  el::Configurations defaultConf;
  defaultConf.setToDefault();

  defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
  defaultConf.setGlobally(el::ConfigurationType::Filename, "param_server.log");
  // defaultConf.setGlobally(el::ConfigurationType::ToStandardOutput, "false");
  // default logger uses default configurations
  el::Loggers::reconfigureLogger("default", defaultConf);
  m_ServerThreadContext = std::make_shared<ServerThread>(0);
  m_ServerThread = std::make_shared<Thread>(m_ServerThreadContext);

  m_MutilCastThreadContext = std::make_shared<MutilCastThread>(0);
  m_MutilCastThread = std::make_shared<Thread>(m_MutilCastThreadContext);
#ifdef WITH_HTTP_PAGE
  debug_ = true;
#else
  debug_ = false;
#endif
}

void ParameterServer::stop_server() {
  m_ServerThreadContext->stop();
  m_MutilCastThreadContext->stop();
}

void ParameterServer::start_server() {
  m_ServerThread->start();
  m_MutilCastThread->start();
}

void ParameterServer::init() {
  _cfgRoot = Config::object();
}
