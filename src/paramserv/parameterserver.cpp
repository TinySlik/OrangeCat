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
#include <fstream>
#include <iostream>
#include <string>
#include "easylogging++.h"
#include "lodepng.h"
#include <iostream>
#include <string>
#include <mutex>

#include <time.h>   // clock
#include <stdio.h>  // snprintf

#include <string.h>
#include <map>
#include <iostream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#define CONFIGURU_IMPLEMENTATION 1
#define TARGET_WEB_DIR_NAME "../res/web_root"
#define CONFIGURU_JSON_PARSE_ERROR_LOG ""
#define CACHE_MAX_SIZE (128*1024)
#define STATUS_DISPLAY_TIME_INTERVAL 100
#define DEBUG_PARAM_SERV
#define CONFIG_HIDEN_PARAM
#define WITH_HTTP_PAGE
INITIALIZE_EASYLOGGINGPP

#include "configuru.hpp"
using namespace configuru;
static char s_http_port[] = "8099";
static struct mg_serve_http_opts s_http_server_opts;
static char cache[CACHE_MAX_SIZE];
bool _thread_stop = false;

unsigned char *pixels = NULL;
unsigned int width, height;
int instance_or_mutil = 0;

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

unsigned int iMemClock, iCurClock, iLoops;
char aFPS[12];

static void handle_get_device_usage(struct mg_connection *nc) {
  // Use chunked encoding in order to avoid calculating Content-Length
  mg_printf(nc, "%s", "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");

  auto cfg = ParameterServer::instance()->getCfgRoot();
  // uint64_t mem = 0, vmem = 0, r = 0, w = 0;
  if (cfg.has_key("dev_status") &&
    cfg["dev_status"].is_object()) {
  } else {
    cfg["dev_status"]= Config::object();
  }
  auto dev_status = cfg["dev_status"];
  dev_status["fps"] = aFPS;
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
  auto dev_ctrl = ParameterServer::instance()->getCfgCtrlRoot();

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

  auto dev_ctrl = ParameterServer::instance()->getCfgCtrlRoot();
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

  auto dev_ctrl = ParameterServer::instance()->getCfgCtrlRoot();
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

static int is_websocket(const struct mg_connection *nc) {
  return nc->flags & MG_F_IS_WEBSOCKET;
}

std::vector<struct mg_connection *> ncs = {nullptr};

static void broadcast(struct mg_connection *nc, const struct mg_str msg) {
  struct mg_connection *c;
  char buf[500];
  char addr[32];
  mg_sock_addr_to_str(&nc->sa, addr, sizeof(addr),
                      MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);

  snprintf(buf, sizeof(buf), "%s %.*s", addr, (int) msg.len, msg.p);
  printf("%s\n", buf); /* Local echo. */
  for (c = mg_next(nc->mgr, NULL); c != NULL; c = mg_next(nc->mgr, c)) {
    if (c == nc) continue; /* Don't send to the sender. */
    mg_send_websocket_frame(c, WEBSOCKET_OP_TEXT, buf, strlen(buf));
  }
}

static void broadcast(const char *buf, size_t len) {
   /* Local echo. */
  for (size_t i = 1; i < ncs.size(); i++) {
    mg_send_websocket_frame(ncs[i], WEBSOCKET_OP_BINARY, buf, len);
  }
}

static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;
  switch (ev) {
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
      /* New websocket connection. Tell everybody. */
      ncs.push_back(nc);
      break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
      // struct websocket_message *wm = (struct websocket_message *) ev_data;
      /* New websocket message. Tell everybody. */
      // struct mg_str d = {(char *) wm->data, wm->size};
      // broadcast(nc, d);
      break;
    }
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
    case MG_EV_CLOSE: {
      /* Disconnect. Tell everybody. */
      if (is_websocket(nc)) {
        for (size_t i = 0; i < ncs.size(); i++) {
          if(ncs[i] == nc) {
            ncs.erase(ncs.begin() + i);
          }
        }
        broadcast(nc, mg_mk_str("-- left"));
      }
      break;
    }
    default:
      break;
  }
}

typedef enum {
    INIT,
    RUNNING,
    STOP
} ThreadState;

class ParameterServerImp {
  struct mg_mgr mgr;
  struct mg_connection *nc;
  cs_stat_t st;
  ParameterServerImp() :
  _index(0), 
  m_image(nullptr),
  data_type('\0')
  {
    #ifdef WITH_HTTP_PAGE
    debug_ = true;
#else
    debug_ = false;
#endif
  }
  ~ParameterServerImp(){}
  ThreadState requestedState=INIT;
  ThreadState currentState=INIT;
  
  std::shared_ptr<std::vector<unsigned char>> m_image;
  configuru::Config _cfgRoot;
  configuru::Config _null;
  unsigned w, h;
  std::mutex data_lock_;
  size_t _index;
  bool debug_;
  char data_type;
  void startServer();
  void stopServer();
  int sampleImgData(std::shared_ptr<std::vector<unsigned char>> data);
  
  std::thread m_serverThread;
  bool isDebug() {
    return debug_;
  }

  friend ParameterServer;
};

int ParameterServerImp::sampleImgData(std::shared_ptr<std::vector<unsigned char>> data) {
  data_lock_.lock();
  auto sz = data -> size();
  // w = (*data)[sz - 1];
  // h = (*data)[sz - 2];
  data_type = (*data)[sz - 1];
  short *hw_ptr = (short *)(data->data() + sz - 5);
  w = hw_ptr[0];
  h = hw_ptr[1];
  m_image = data;
  LOG(INFO) << data_type << "||"<< w << "||" << h;
  data_lock_.unlock();
  return 0;
}

//returns 0 if all went ok, non-0 if error
//output image is always given in RGBA (with alpha channel), even if it's a BMP without alpha channel
unsigned decodeBMP(std::vector<unsigned char>& image, unsigned& w, unsigned& h, const std::vector<unsigned char>& bmp) {
  static const unsigned MINHEADER = 54; //minimum BMP header size

  if(bmp.size() < MINHEADER) return -1;
  if(bmp[0] != 'B' || bmp[1] != 'M') return 1; //It's not a BMP file if it doesn't start with marker 'BM'
  unsigned pixeloffset = bmp[10] + 256 * bmp[11]; //where the pixel data starts
  //read width and height from BMP header
  w = bmp[18] + bmp[19] * 256;
  h = bmp[22] + bmp[23] * 256;
  LOG(INFO) << w << "||" << h;
  //read number of channels from BMP header
  if(bmp[28] != 24 && bmp[28] != 32) return 2; //only 24-bit and 32-bit BMPs are supported.
  unsigned numChannels = bmp[28] / 8;

  //The amount of scanline bytes is width of image times channels, with extra bytes added if needed
  //to make it a multiple of 4 bytes.
  unsigned scanlineBytes = w * numChannels;
  if(scanlineBytes % 4 != 0) scanlineBytes = (scanlineBytes / 4) * 4 + 4;

  unsigned dataSize = scanlineBytes * h;
  if(bmp.size() < dataSize + pixeloffset) return 3; //BMP file too small to contain all pixels

  image.resize(w * h * 4);

  /*
  There are 3 differences between BMP and the raw image buffer for LodePNG:
  -it's upside down
  -it's in BGR instead of RGB format (or BRGA instead of RGBA)
  -each scanline has padding bytes to make it a multiple of 4 if needed
  The 2D for loop below does all these 3 conversions at once.
  */
  for(unsigned y = 0; y < h; y++)
  for(unsigned x = 0; x < w; x++) {
    //pixel start byte position in the BMP
    unsigned bmpos = pixeloffset + (h - y - 1) * scanlineBytes + numChannels * x;
    //pixel start byte position in the new raw image
    unsigned newpos = 4 * y * w + 4 * x;
    if(numChannels == 3) {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      image[newpos + 3] = 255;            //A
    } else {
      image[newpos + 0] = bmp[bmpos + 2]; //R
      image[newpos + 1] = bmp[bmpos + 1]; //G
      image[newpos + 2] = bmp[bmpos + 0]; //B
      image[newpos + 3] = bmp[bmpos + 3]; //A
    }
  }
  return 0;
}

void ParameterServerImp::startServer() {
  m_serverThread = std::thread([this]() {
    currentState=RUNNING;
#ifdef WITH_HTTP_PAGE
    mg_mgr_init(&mgr, NULL);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    
    while (nc == NULL && s_http_port[3] != '0') {
      LOG(WARNING) << "Cannot bind to " << s_http_port << std::endl;
      s_http_port[3]--;
      LOG(WARNING) << "Try " << s_http_port << std::endl;
      nc = mg_bind(&(mgr), s_http_port, ev_handler);
    }
    if (s_http_port[3] == '0') {
#ifdef DEBUG_PARAM_SERV
	    LOG(ERROR) << "failed";
#endif
      exit(1);
    }

    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = TARGET_WEB_DIR_NAME;
    s_http_server_opts.enable_directory_listing = "yes";

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
    data_lock_.lock();
    if (!m_image) {
      w = 500;
      h = 500;
      m_image = std::make_shared<std::vector<unsigned char>>(w * h * 4);
    }
    std::vector<unsigned char> png;
    lodepng::encode(png, *m_image, w, h);
    data_lock_.unlock();
    while (requestedState!=STOP) {
      if (ncs.size() > 1) {
        if (iMemClock > (iCurClock = clock()))
          iLoops++;
        else {
          snprintf(aFPS, sizeof(aFPS),"FPS: %d",iLoops);
          iMemClock = iCurClock + CLOCKS_PER_SEC;
          iLoops = 0;
        }
        // for (size_t i = 0; i < image.size() ; i++) image[i] += 1;
        // data_lock_.lock();
        png.clear();
        lodepng::encode(png, *m_image, w, h);
        // data_lock_.unlock();
        broadcast((const char *)png.data(), png.size());
      }
      
      mg_mgr_poll(&mgr, STATUS_DISPLAY_TIME_INTERVAL);
    }
    currentState=STOP;
    mg_mgr_free(&mgr);
    return;
#endif
  });
}

void ParameterServerImp::stopServer() {
  requestedState=STOP;
  currentState=STOP;
  m_serverThread.join();
}

bool ParameterServer::isDebug() {
  return m_imp->isDebug();
}

configuru::Config &ParameterServer::getCfgStatusRoot() {
  return m_imp->_cfgRoot.judge_with_create_key("dev_status");
}
configuru::Config &ParameterServer::getCfgRoot() {
  return m_imp->_cfgRoot;
}
configuru::Config &ParameterServer::getCfgCtrlRoot() {
  return m_imp->_cfgRoot.judge_with_create_key("dev_ctrl");
}
ParameterServer *ParameterServer::instance() {
  if (instance_or_mutil == 0) {
    instance_or_mutil = 1;
  } else if (instance_or_mutil == 1) {
    //
  } else {
    LOG(ERROR) << "instance_or_mutil error.";
    return nullptr;
  }

  static ParameterServer *_this = nullptr;
  if (_this == nullptr) {
    _this = new ParameterServer;
    _this->init();
    _this->m_imp->startServer();
  }
  return _this;
}

ParameterServer::ParameterServer() :
m_imp(new ParameterServerImp()) {
  el::Configurations defaultConf;
  defaultConf.setToDefault();
  defaultConf.setGlobally(el::ConfigurationType::ToFile, "true");
  defaultConf.setGlobally(el::ConfigurationType::Filename, "param_server.log");
  el::Loggers::reconfigureLogger("default", defaultConf);
}

ParameterServer::~ParameterServer() {
  m_imp->stopServer();
  ncs.clear();
  free(pixels);
  delete m_imp;
}

void ParameterServer::init() {
  m_imp->_cfgRoot = Config::object();
}

ParameterServer::ParameterServer(const std::string &port) {
  LOG(INFO) << port;
}

std::shared_ptr<ParameterServer> ParameterServer::create(const std::string &port) {
  if (instance_or_mutil == 0) {
    instance_or_mutil = 2;
  } else if (instance_or_mutil == 2) {
    //
  } else {
    LOG(ERROR) << "instance_or_mutil error.";
    return nullptr;
  }
  LOG(INFO) << port;
  return std::make_shared<ParameterServer>(port);
}

int ParameterServer::sampleImgData(std::shared_ptr<std::vector<unsigned char>> data) {
  return m_imp->sampleImgData(data);
}