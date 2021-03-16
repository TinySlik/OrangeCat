#include "parameterserver.h"
#include <thread>
#include <chrono>

#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  ParameterServer::instance()->CreateNewRoot("default", {
    {"dev_ctrl", {
      { "key1", "value1" },
      { "key2", "value2" },
    }},
    {"dev_status", {
      { "key1", "value1" },
      { "key2", "value2" },
    }}
  });

  ParameterServer::instance()->CreateNewRoot("second", {
    {"dev_ctrl", {
      {"pi",     3.14},
      {"array",  configuru::Config::array({ 1, 2, 3 })},
    }},
    {"dev_status", {
      {"pi",     3.14},
    }}
  });
  

  ParameterServer::instance()->SetCurrentRoot("second");
  auto ctrl = ParameterServer::instance()->GetCfgCtrlRoot();

  ctrl["pi"].add_callback([](configuru::Config &, const configuru::Config &b)->bool{
      LOG(INFO) << b;
      return true;
    });
  std::cout << "";
  ctrl["status"] = "ok";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  ctrl["pi"] << 67.7;
  LOG(INFO) << "Press Enter to exit." << std::endl;
  std::cin.get();
  LOG(INFO) << "Exit." << std::endl;
  return 0;
}