#pragma once

#include <string>
#include <iostream>
#include <algorithm>
#include <map>

#ifdef WITH_REALSENSE
#include "librealsense/rs.hpp"
#endif

#include "ThreadMutexObject.h"
#include "CameraInterface.h"

class RealSenseInterface : public CameraInterface
{
public:
  RealSenseInterface(int width = 640,int height = 480,int fps = 30);
  virtual ~RealSenseInterface();

  const int width,height,fps;

  bool getAutoExposure();
  bool getAutoWhiteBalance();
  virtual void setAutoExposure(bool value);
  virtual void setAutoWhiteBalance(bool value);

  virtual bool ok()
  {
    return initSuccessful;
  }

  virtual std::string error()
  {
    return errorText;
  }


private:
  void devThreadFunc();
#ifdef WITH_REALSENSE
  rs::device *dev;
  rs::context ctx;
  std::thread real_sense_thread_;

#endif

  bool initSuccessful;
  bool is_running_;
  std::string errorText;
  size_t rgb_size_in_bytes_;
  size_t depth_size_in_bytes_;
  
  ThreadMutexObject<int> latestRgbIndex;
  //std::pair<uint8_t *,int64_t> rgbBuffers[numBuffers];

  int64_t lastRgbTime;
  int64_t lastDepthTime;

};
