
/**
 * Format is:
 * int32_t at file beginning for frame count
 *
 * For each frame:
 * int64_t: timestamp
 * int32_t: depthSize
 * int32_t: imageSize
 * depthSize * unsigned char: depth_compress_buf
 * imageSize * unsigned char: encodedImage->data.ptr
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <zlib.h>

#include <limits>
#include <cassert>
#include <iostream>
#include <memory>

#include <opencv2/opencv.hpp>

//#include <boost/format.hpp>
//#include <boost/thread.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/date_time/posix_time/posix_time_types.hpp>
//#include <boost/thread/condition_variable.hpp>

//#include <condition_variable> // std::condition_variable_any

#include "RealSenseInterface.h"

#include "ThreadMutexObject.h"

#include "TcpHandler.h"


constexpr int getBuffersNum()
{
  return 10;
}

class TcpLogger
{
    public:
        TcpLogger(int width, int height, int fps);
        virtual ~TcpLogger();

        void start();
        void stop();

        void setAutoExposure(bool enable){}
        void setAutoWhiteBalance(bool enable){}

        void setCompressed(bool value)
        {
            assert(!writing.getValue());
            compressed = value;
        }

        ThreadMutexObject<std::pair<bool, int64_t> > dropping;
        //ThreadMutexObject<int> latestDepthIndex;

    private:
        std::shared_ptr<RealSenseInterface> m_device;
        //int64_t m_lastImageTime;
        //int64_t m_lastDepthTime;
        //ThreadMutexObject<int> latestImageIndex;

        int depth_compress_buf_size;
        uint8_t * depth_compress_buf;
        CvMat * encodedImage;

        int lastWritten;
        //boost::thread * writeThread;
        ThreadMutexObject<bool> writing;
        int64_t lastTimestamp;

        int width;
        int height;
        int fps;

        TcpHandler * tcp;
        uint8_t * tcpBuffer;

        void encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data);
        void loggingThread();

        int32_t numFrames;

        bool compressed;

};

#endif 
