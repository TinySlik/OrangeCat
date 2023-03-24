#define CONFIGURU_IMPLEMENTATION 1
#include <thread>
#include <chrono>
#include "parameterserver.h"

bool thr_tag_ = false;

int main(int, char**) {
  auto cfg = ParameterServer::instance()->getCfgCtrlRoot();
  cfg["status"] = "ok";
  cfg["lucky"] = "yes";
  cfg["example"]= {
      {"display_switch", 1},
      {"static_file_info", {
        {"test_file_path", "empty"},
        {"file_load_location", 0},
        {"cdf", "empty"},
        {"m_msg_decoder", "empty"},
      }},
      {"svg_background_path", "empty"},
      {"m_decoder_unsigned", 1},
      {"front_color", "#00ff00"},
      {"background_color", "#0000ff"},
      {"transform", {
        {
          "m_translate", {
            {"x", 0},
            {"y", 0},
            {"z", 0},
          }
        },
        {
          "m_rotate", {
            {"x", 0},
            {"y", 0},
            {"z", 0},
          },
        },
        {"m_angle", 0},
        {
          "m_scale", {
            {"x", 0},
            {"y", 0},
            {"z", 0},
          }
        }
      }}
    };

  auto status = ParameterServer::instance()->getCfgStatusRoot();
  status["custom"] = "something";
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Press Enter to exit." << std::endl;
  thr_tag_ = true;
  auto thr = std::thread([]() { 
    auto data = std::make_shared<std::vector<unsigned char>>(500 * 500 * 4 + 5);
    short *hw_ptr = (short *)(data->data() + 500 * 500 * 4);
    (*data)[500 * 500 * 4 + 4] = 'c';
    hw_ptr[0] = 500;
    hw_ptr[1] = 500;
    memset(data->data(), 100, 500 * 500 * 4);
    while (thr_tag_) {
      for (int i = 0; i< 500 * 500 * 4; i++) {
        (*data)[i] += 1;
      }
      ParameterServer::instance()->sampleImgData(data);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });
  
  std::cin.get();
  thr_tag_ = false;
  thr.join();
  std::cout << "Exit." << std::endl;
  return 0;
}
