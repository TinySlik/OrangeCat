#define CONFIGURU_IMPLEMENTATION 1
#include <thread>
#include <chrono>
#include "parameterserver.h"

int main(int, char**) {
  auto ctrl = ParameterServer::instance()->getCfgCtrlRoot();
  ctrl["status"] = "ok";

  auto status = ParameterServer::instance()->getCfgStatusRoot();
  status["bingo"] = "kkl";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Press Enter to exit." << std::endl;
  std::cin.get();
  std::cout << "Exit." << std::endl;
  return 0;
}
