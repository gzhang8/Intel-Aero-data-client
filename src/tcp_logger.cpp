#include <thread>
#include "tcp_logger.h"

TcpLogger::TcpLogger(int width, int height, int fps)
  : dropping(std::pair<bool, int64_t>(false, -1)),
    lastWritten(-1),
    //writeThread(0),
    width(width),
    height(height),
    fps(fps),
    numFrames(0),
    compressed(true)
{

  depth_compress_buf_size = width * height * sizeof(int16_t) * 4;
  depth_compress_buf = (uint8_t*)malloc(depth_compress_buf_size);

  encodedImage = 0;

  writing.assignValue(false);

  tcpBuffer = (uint8_t *)malloc(sizeof(int64_t) + 2 * sizeof(int) +
                                depth_compress_buf_size + width * height * 3);
  this->tcp = new TcpHandler(5698);


  m_device = std::shared_ptr<RealSenseInterface> (
    new  RealSenseInterface(width, height, fps));
  if(!m_device || !m_device->ok())
  {
    std::cout << "failed!" << std::endl;
    std::cout << m_device->error();

    throw m_device->error();
  }
  else
  {
    std::cout << "success!" << std::endl;

    std::cout << "Waiting for first frame"; std::cout.flush();

    int lastDepth = m_device->latestDepthIndex.getValue();

    do
    {
      std::this_thread::sleep_for(std::chrono::microseconds(33333));
      std::cout << "."; std::cout.flush();
      lastDepth = m_device->latestDepthIndex.getValue();
    } while(lastDepth == -1);

    std::cout << " got it!" << std::endl;
    }

}

TcpLogger::~TcpLogger()
{

  free(depth_compress_buf);

  assert(!writing.getValue() && "Please stop writing cleanly first");

  if(encodedImage != 0)
  {
    cvReleaseMat(&encodedImage);
  }


  delete [] tcpBuffer;
  delete tcp;

}


void TcpLogger::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data)
{
  cv::Mat3b rgb(height, width, rgb_data, width * 3);

  IplImage * img = new IplImage(rgb);

  int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

  if(encodedImage != 0)
  {
    cvReleaseMat(&encodedImage);
  }

  encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

  delete img;
}

void TcpLogger::start()
{
  //assert(!writeThread && !writing.getValue());
  assert(!writing.getValue());

  lastTimestamp = -1;
  writing.assignValue(true);

  numFrames = 0;
  //writeThread = new boost::thread(boost::bind(&TcpLogger::loggingThread,
  //                                            this));
  loggingThread();
}

void TcpLogger::stop()
{
  //assert(writeThread && writing.getValue());
  assert(writing.getValue());
  writing.assignValue(false);
  //writeThread->join();
  dropping.assignValue(std::pair<bool, int64_t>(false, -1));

  //writeThread = 0;
  numFrames = 0;
}

void TcpLogger::loggingThread()
{
  while(writing.getValueWait(1000))
  {
    int lastDepth = m_device -> latestDepthIndex.getValue();

    if(lastDepth == -1)
    {
      continue;
    }

    //int bufferIndex = lastDepth % m_device -> numBuffers;
    int bufferIndex = lastDepth % CameraInterface::numBuffers;

    if(bufferIndex == lastWritten)
    {
      continue;
    }
    if(lastTimestamp == m_device->frameBuffers[bufferIndex].second)
    {
      continue;
    }

    unsigned char * rgbData = 0;
    unsigned char * depthData = 0;
    //unsigned long depthSize = depth_compress_buf_size;
    int32_t depthSize = depth_compress_buf_size;
    int32_t rgbSize = 0;

    if(compressed)
    {
      //boost::thread_group threads;
      unsigned long depth_size_tmp_ = depthSize;
      //threads.add_thread(new boost::thread(compress2,
      std::thread depth_compress_thread(compress2,
                                        depth_compress_buf,
                                        &depth_size_tmp_,
                                        (const Bytef*)m_device -> frameBuffers[bufferIndex].first.first,
                                        width * height * sizeof(short),
                                        Z_BEST_SPEED);

    //threads.add_thread(new boost::thread(boost::bind(&TcpLogger::encodeJpeg,
    //this,
    // (cv::Vec<unsigned char, 3> *)m_device -> frameBuffers[bufferIndex].first.second)));
      auto raw_rgb_buf = (cv::Vec<unsigned char, 3> *)m_device -> frameBuffers[bufferIndex].first.second;
      std::thread rgb_compress_thread([this](cv::Vec<unsigned char, 3> * rgb_data) {encodeJpeg(rgb_data);},
        raw_rgb_buf);
      depth_compress_thread.join();
      rgb_compress_thread.join();

  //threads.join_all();


      rgbSize = encodedImage->width;
      depthSize = (int32_t)depth_size_tmp_;
      std::cout << "long depth size: " << depth_size_tmp_ << std::endl;
      depthData = (unsigned char *)depth_compress_buf;
      rgbData = (unsigned char *)encodedImage->data.ptr;
    }
    else
    {
      depthSize = width * height * sizeof(short);
      rgbSize = width * height * sizeof(unsigned char) * 3;

      depthData = (unsigned char *)m_device->frameBuffers[bufferIndex].first.first;
      rgbData = (unsigned char *)m_device->frameBuffers[bufferIndex].first.second;
    }

    if(tcp)
    {

      memcpy(tcpBuffer, &(m_device->frameBuffers[bufferIndex].second), sizeof(int64_t));
      memcpy(tcpBuffer + sizeof(int64_t), &rgbSize, sizeof(int));
      memcpy(tcpBuffer + sizeof(int64_t) + sizeof(int),
             &depthSize, sizeof(int));
      memcpy(&tcpBuffer[sizeof(int64_t) + 2 * sizeof(int)],
             rgbData, rgbSize);
      memcpy(&tcpBuffer[sizeof(int64_t) + 2 * sizeof(int) + rgbSize], depthData, depthSize);

      std::cout << "rgb size: " << rgbSize << ", depth size: " << depthSize
                << ", data len: " << sizeof(int64_t) + 2 * sizeof(int) + rgbSize + depthSize
                << std::endl;

      tcp->sendData(tcpBuffer, sizeof(int64_t) + 2 * sizeof(int) + rgbSize + depthSize);
    }

    numFrames++;

    lastWritten = bufferIndex;

    if(lastTimestamp != -1)
    {
      if(m_device -> frameBuffers[bufferIndex].second - lastTimestamp > 1000000)
      {
        dropping.assignValue(std::pair<bool, int64_t>(true, m_device -> frameBuffers[bufferIndex].second - lastTimestamp));
      }
    }

    lastTimestamp = m_device -> frameBuffers[bufferIndex].second;
  }
}


