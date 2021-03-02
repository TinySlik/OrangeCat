#include "parameterserver.h"

#define ELPP_THREAD_SAFE
#define ELPP_FORCE_USE_STD_THREAD
#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
  START_EASYLOGGINGPP(argc, argv);
  auto ctrl = ParameterServer::instance()->GetCfgCtrlRoot();
  ctrl["status"] = "ok";
  LOG(INFO) << "Press Enter to exit." << std::endl;
  std::cin.get();
  return 0;
}