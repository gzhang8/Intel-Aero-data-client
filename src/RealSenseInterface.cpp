#include "RealSenseInterface.h"
#include <functional>

#ifdef WITH_REALSENSE
RealSenseInterface::RealSenseInterface(int inWidth,int inHeight,int inFps)
  : width(inWidth),
    height(inHeight),
    fps(inFps),
    dev(nullptr),
    initSuccessful(true),
    is_running_(true)
{
  if(ctx.get_device_count() == 0)
  {
    errorText = "No device connected.";
    initSuccessful = false;
    return;
  }

  dev = ctx.get_device(0);
  dev->enable_stream(rs::stream::depth,width,height,rs::format::z16,fps);
  dev->enable_stream(rs::stream::color,width,height,rs::format::rgb8,fps);
  std::cout << "width: " << width << ", height: " << height << std::endl;
  dev = ctx.get_device(0);
  //dev->set_option(rs::option::r200_lr_gain,4);
  //dev->set_option(rs::option::r200_depth_clamp_min,1);                   
  dev->enable_stream(rs::stream::depth, width ,height,rs::format::z16,fps);
  //dev->enable_stream(rs::stream::depth, 480 ,360,rs::format::z16,fps);
  //dev->enable_stream(rs::stream::depth,rs::preset::best_quality);  // width ,h
  dev->enable_stream(rs::stream::color,width,height,rs::format::rgb8,fps); 
  //dev->enable_stream(rs::stream::color, rs::preset::best_quality); //width,hei
  rs::intrinsics depth_intrin, color_intrin;                                  
  rs::format depth_format, color_format;                                 
  depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);               
  depth_format = dev->get_stream_format(rs::stream::depth);                   
  color_intrin = dev->get_stream_intrinsics(rs::stream::color);          
  color_format = dev->get_stream_format(rs::stream::color);                   
  std::cout << "depth intrin: fx:  "<< depth_intrin.fx                          
            << "fy: " << depth_intrin.fy                                   
            << "ppx: " << depth_intrin.ppx                                      
            << "ppy: " << depth_intrin.ppy                                      
            << "width: " << depth_intrin.width                             
            << "height: " << depth_intrin.height                                
             << std::endl;                                                      
  std::cout << "color intrin: fx: " << color_intrin.fx                     
            << ", fy: "<< color_intrin.fy                                       
            << ", ppx: " << color_intrin.ppx                                    
            << ", ppy: " << color_intrin.ppy                               
            << ", width: " << color_intrin.width                                
            << ", height: " << color_intrin.height                              
            << std::endl;  



  rgb_size_in_bytes_ = width * height * 3;
  depth_size_in_bytes_ = width * height * 2;

  latestDepthIndex.assign(-1);
  //latestRgbIndex.assign(-1);

  for(int i = 0; i < numBuffers; i++)
  {
    uint8_t * newDepth = (uint8_t *)calloc(width * height * 2,sizeof(uint8_t));
    uint8_t * newImage = (uint8_t *)calloc(width * height * 3,sizeof(uint8_t));
    frameBuffers[i] = std::pair<std::pair<uint8_t *,uint8_t *>,int64_t>(std::pair<uint8_t *,uint8_t *>(newDepth,newImage),0);
  }

  setAutoExposure(true);
  setAutoWhiteBalance(true);
  dev->start();
  real_sense_thread_ = std::thread ([this]{devThreadFunc();});
}

RealSenseInterface::~RealSenseInterface()
{
  if(initSuccessful)
  {
    is_running_ = false;
    dev->stop();

    for(int i = 0; i < numBuffers; i++)
    {
      free(frameBuffers[i].first.first);
      free(frameBuffers[i].first.second);
    }

  }
}


void RealSenseInterface::devThreadFunc() {
  while (is_running_) {
    dev->wait_for_frames(); // wait for frames
    auto rgb_data = dev->get_frame_data(rs::stream::color);
    auto depth_data = dev->get_frame_data(rs::stream::depth_aligned_to_color);


    auto time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();

    int bufferIndex = (latestDepthIndex.getValue() + 1) % numBuffers;

    // The multiplication by 2 is here because the depth is actually uint16_t
    memcpy(frameBuffers[bufferIndex].first.first, depth_data, depth_size_in_bytes_);
    memcpy(frameBuffers[bufferIndex].first.second, rgb_data, rgb_size_in_bytes_);

    frameBuffers[bufferIndex].second = time_stamp;

    latestDepthIndex++;


  }
}


void RealSenseInterface::setAutoExposure(bool value)
{
  dev->set_option(rs::option::color_enable_auto_exposure,value);
}

void RealSenseInterface::setAutoWhiteBalance(bool value)
{
  dev->set_option(rs::option::color_enable_auto_white_balance,value);
}

bool RealSenseInterface::getAutoExposure()
{
  return dev->get_option(rs::option::color_enable_auto_exposure);
}

bool RealSenseInterface::getAutoWhiteBalance()
{
  return dev->get_option(rs::option::color_enable_auto_white_balance);
}
#else

RealSenseInterface::RealSenseInterface(int inWidth,int inHeight,int inFps)
  : width(inWidth),
  height(inHeight),
  fps(inFps),
  initSuccessful(false)
{
  errorText = "Compiled without Intel RealSense library";
}

RealSenseInterface::~RealSenseInterface()
{
}

void RealSenseInterface::setAutoExposure(bool value)
{
}

void RealSenseInterface::setAutoWhiteBalance(bool value)
{
}

bool RealSenseInterface::getAutoExposure()
{
  return false;
}

bool RealSenseInterface::getAutoWhiteBalance()
{
  return false;
}
#endif
