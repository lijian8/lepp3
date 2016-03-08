#ifndef BASE_VIDEO_SOURCE_H_
#define BASE_VIDEO_SOURCE_H_

#include <vector>
#include <map>
#include <initializer_list>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/io/openni_grabber.h>
#include <pcl/visualization/cloud_viewer.h>

#include <opencv2/core/core.hpp>

#include "VideoObserver.hpp"

namespace lepp {

typedef pcl::PointXYZ SimplePoint;
typedef pcl::PointCloud<SimplePoint> SimplePointCloud;
typedef pcl::PointXYZRGBA ColoredPoint;
typedef pcl::PointCloud<ColoredPoint> ColoredPointCloud;


/**
 * The abstract base class for all classes that wish to be sources of point
 * cloud video information.
 *
 * The VideoSource is an "Observable" object to which interested observers
 * (implementing the VideoObserver interface) can attach.
 *
 * This class provides the common functionality for all VideoSource
 * implementations. Most importantly, it implements all logic regarding tracking
 * and notifying observers of the source when a new point cloud is received by
 * the source.
 */
template<class PointT>
class VideoSource {
public:
  /**
   * Convenience type for defining observers for a particular VideoSource.
   */
  typedef VideoObserver<PointT> ObserverType;
  /**
   * Convenience type for referring to the type of the pcl::PointCloud that is
   * handled by a particular VideoSource.
   */
  typedef pcl::PointCloud<PointT> PointCloudType;

  VideoSource() : frame_counter_(0) {}
  virtual ~VideoSource();

  /**
   * Starts the video source.
   */
  virtual void open() = 0;
  /**
   * Attaches a new VideoObserver to the VideoSource instance.
   * Each observer will get notified once the VideoSource has received a new
   * frame and converted it into a point cloud.
   */
  virtual void attachObserver(boost::shared_ptr<ObserverType> observer);
  /**
   * Convenience method for subclasses to set any option on the members or
   * methods of the class.
   */
  virtual void setOptions(
    std::map<std::string, bool> options) = 0;

protected:
  /**
   * Convenience method for subclasses to indicate that a new cloud has been
   * received.  It is enough to invoke this method in order to register a new
   * point cloud as the most recent one, as well as to notify all source
   * observers.
   */
  virtual void setNextFrame(const typename PointCloudType::ConstPtr& cloud);
  /**
   * Convenience method for subclasses to indicate that a new RGB image has been
   * received.  It is enough to invoke this method in order to register a new
   * RGB image as the most recent one, as well as to notify all source
   * observers.
   */
  virtual void setNextFrame(const typename boost::shared_ptr<openni_wrapper::Image>& rgb);
  virtual void setNextFrame(const cv::Mat& rgb);
private:
  /**
   * Private helper method.  Notifies all known observers that a new point cloud
   * has been received.
   */
  void notifyObservers(
      int idx,
      const typename PointCloudType::ConstPtr& cloud) const;
  /**
   * Private helper method.  Notifies all known observers that a new RGB image
   * has been received.
   */
  void notifyObservers(
      int idx,
      const typename boost::shared_ptr<openni_wrapper::Image>& rgb) const;
  /**
   * Private helper method.  Notifies all known observers that a new offline
   * RGB image has been received.
   */
  void notifyObservers(
      int idx,
      const cv::Mat& rgb) const;
  /**
   * Keeps track of all observers that are attached to the source.
   */
  std::vector<boost::shared_ptr<ObserverType> > observers_;
  /**
   * Counts how many frames have been seen by the video source.
   */
  int frame_counter_;
};

template<class PointT>
VideoSource<PointT>::~VideoSource() {
  // Empty!
}

template<class PointT>
void VideoSource<PointT>::notifyObservers(
    int idx,
    const typename pcl::PointCloud<PointT>::ConstPtr& cloud) const {
  size_t const sz = observers_.size();
  for (size_t i = 0; i < sz; ++i) {
    observers_[i]->notifyNewFrame(idx, cloud);
  }
}

template<class PointT>
void VideoSource<PointT>::notifyObservers(
    int idx,
    const typename boost::shared_ptr<openni_wrapper::Image>& rgb) const {
  size_t const sz = observers_.size();
  for (size_t i = 0; i < sz; ++i) {
    observers_[i]->notifyNewFrame(idx, rgb);
  }
}

template<class PointT>
void VideoSource<PointT>::notifyObservers(
    int idx,
    const cv::Mat& rgb) const {

  std::cout << "entered BaseVideoSource::notifyObservers" << std::endl;
  size_t const sz = observers_.size();
  for (size_t i = 0; i < sz; ++i) {
    observers_[i]->notifyNewFrame(idx, rgb);
  }
}

template<class PointT>
void VideoSource<PointT>::setNextFrame(
    const typename PointCloudType::ConstPtr& cloud) {
  ++frame_counter_;
  notifyObservers(frame_counter_, cloud);
}

template<class PointT>
void VideoSource<PointT>::setNextFrame(
    const typename boost::shared_ptr<openni_wrapper::Image>& rgb) {
  // ++frame_counter_;
  notifyObservers(frame_counter_, rgb);
}

template<class PointT>
void VideoSource<PointT>::setNextFrame(const cv::Mat& rgb) {
  std::cout << "entered BaseVideoSource::setNextFrame" << std::endl;
  notifyObservers(frame_counter_, rgb);
}

template<class PointT>
void VideoSource<PointT>::attachObserver(
    boost::shared_ptr<ObserverType> observer) {
  observers_.push_back(observer);
}

/**
 * Convenience typedefs referring to VideoSource classes based on the template
 * parameter.
 */
typedef VideoSource<ColoredPoint> ColoredVideoSource;
typedef VideoSource<SimplePoint> SimpleVideoSource;

}  // namespace lepp

#endif
