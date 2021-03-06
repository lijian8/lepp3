#ifndef LEPP3_FILTERED_VIDEO_SOURCE_H__
#define LEPP3_FILTERED_VIDEO_SOURCE_H__

#include "lepp3/VideoSource.hpp"
#include "lepp3/filter/point/PointFilter.hpp"
#include "lepp3/filter/cloud/post/CloudPostFilter.hpp"
#include "lepp3/filter/cloud/pre/CloudPreFilter.hpp"
#include "lepp3/FrameData.hpp"

#include <algorithm>
#include <numeric>

#include <pcl/common/transforms.h>
#include <pcl/filters/conditional_removal.h>
#include <opencv2/core/core.hpp>

#include <boost/enable_shared_from_this.hpp>
#include <boost/circular_buffer.hpp>

#include "lepp3/Typedefs.hpp"
#include "lepp3/debug/timer.hpp"

#include "deps/easylogging++.h"

namespace lepp {

/**
 * ABC.
 * A VideoSource decorator.  It wraps a given VideoSource instance and emits
 * clouds that are filtered versions of the original raw cloud returned by the
 * wrapped `VideoSource`.
 *
 * The cloud it receives is first filtered by applying a number of point-wise
 * filters to each point. The filters that are applied to points (if any) are
 * set dynamically (`setFilter` method).
 *
 * Then, it delegates to a concrete implementation which needs to handle the
 * cloud-level filtering.
 *
 * Concrete implementations need to provide implementations for three protected
 * hook methods that handle the full-cloud filtering at various stages: before
 * the original cloud's points are transformed, after a particular point is
 * transformed, and finally after all points have been transformed.
 *
 * This ABC provides the plumbing required to turn the filtered source into a
 * `VideoSource` making it simple to implement a concrete filter by simply
 * providing implementations for the mentioned hooks.
 */
template<class PointT>
class FilteredVideoSource
    : public VideoSource<PointT>,
      public FrameDataObserver,
      public boost::enable_shared_from_this<FilteredVideoSource<PointT> > {
public:
  /**
   * Creates a new FilteredVideoSource which will perform filtering of each
   * cloud generated by the given source and emit such a filtered cloud to its
   * own observers.
   *
   * The FilteredVideoSource instance does not assume ownership of the given
   * source, but shares it.
   */
  FilteredVideoSource(boost::shared_ptr<VideoSource<PointT>> source)
      : VideoSource<PointT>(std::shared_ptr<lepp::PoseService>()),
        source_(source) {}

  /**
   * Implementation of the VideoSource interface.
   */
  virtual void open() override;

  /**
   * Implementation of the VideoSource interface.
   */
  virtual void setOptions(
      const std::map<std::string, bool>& options) override;

  /**
   * Implementation of the FrameDataObserver interface.
   */
  virtual void updateFrame(FrameDataPtr frameData) override;

  /**
   * Add a filter that will be applied to individual points before the entire
   * cloud itself is filtered.
   */
  void addFilter(boost::shared_ptr<lepp::PointFilter<PointT> > filter) {
    point_filters_.push_back(filter);
  }

  /**
   * Add a cloud filter to apply before the point filters
   */
  void setPreFilter(boost::shared_ptr<lepp::CloudPreFilter<PointT>> filter) {
    pre_filter_ = filter;
  }

  /**
   * Add a cloud filter to apply after the point filters
   */
  void setPostFilter(boost::shared_ptr<lepp::CloudPostFilter<PointT>> filter) {
    post_filter_ = filter;
  }

private:
  /**
   * The VideoSource instance that will be filtered by this instance.
   */
  boost::shared_ptr<VideoSource<PointT> > source_;
  /**
   * The filters that are applied to individual points (in the order found in
   * the vector) before passing it off to the concrete cloud filter
   * implementation.
   */
  std::vector<boost::shared_ptr<lepp::PointFilter<PointT> > > point_filters_;

  /**
   * Filters which are applied to the whole cloud
   */
  boost::shared_ptr<lepp::CloudPreFilter<PointT>> pre_filter_;
  boost::shared_ptr<lepp::CloudPostFilter<PointT>> post_filter_;

  /**
  * Remove NaN points from input cloud.
  */
  void preprocessCloud(PointCloudPtr& cloud);
};

template<class PointT>
void FilteredVideoSource<PointT>::open() {
  // Start the wrapped VideoSource and make sure that this instance is notified
  // when it emits any new clouds.
  source_->FrameDataSubject::attachObserver(this->shared_from_this());
  source_->open();
}

/**
* Remove NaN points from input cloud.
*/
template<class PointT>
void FilteredVideoSource<PointT>::preprocessCloud(PointCloudPtr& cloud) {
  // Remove NaN points from the input cloud.
  // The pcl API forces us to pass in a reference to the vector, even if we have
  // no use of it later on ourselves.
  PointCloudPtr cloud_filtered(new PointCloudT());
  std::vector<int> index;
  pcl::removeNaNFromPointCloud<PointT>(*cloud, *cloud_filtered, index);

  cloud = cloud_filtered;
}

template<class PointT>
void FilteredVideoSource<PointT>::setOptions(const std::map<std::string, bool>& options) {
  source_->setOptions(options);
}

template<class PointT>
void FilteredVideoSource<PointT>::updateFrame(
    FrameDataPtr frameData) {
  Timer t;
  t.start();

  // Prepare the point-wise filters for a new frame.
  {
    size_t sz = point_filters_.size();
    for (size_t i = 0; i < sz; ++i) {
      point_filters_[i]->prepareNext();
    }
  }

  // Process the cloud received from the wrapped instance and put the filtered
  // result in a new point cloud.
  PointCloudConstPtr source_cloud = frameData->cloud;

  if (this->pre_filter_) {
    this->pre_filter_->newFrame();
    this->pre_filter_->getFiltered(source_cloud);
  }
  if (this->post_filter_) {
    this->post_filter_->newFrame();
  }

  PointCloudPtr cloud_filtered(new PointCloudT());
  PointCloudT& filtered = *cloud_filtered;
  cloud_filtered->is_dense = true;
  cloud_filtered->sensor_origin_ = source_cloud->sensor_origin_;

  // Apply point-wise filters to each received point and then pass it to the
  // concrete implementation to figure out how to filter the entire cloud.
  for (typename PointCloudT::const_iterator it = source_cloud->begin();
       it != source_cloud->end();
       ++it) {
    PointT p = *it;
    // Filter out NaN points already, since we're already iterating through the
    // cloud.
    if (!pcl_isfinite(p.x) || !pcl_isfinite(p.y) || !pcl_isfinite(p.z)) {
      continue;
    }

    // Now apply point-wise filters.
    size_t const sz = point_filters_.size();
    bool valid = true;
    for (size_t i = 0; i < sz; ++i) {
      if (!point_filters_[i]->apply(p)) {
        valid = false;
        break;
      }
    }
    // And pass such a filtered/modified point to the cloud-level filter.
    if (valid) {
      if (this->post_filter_) {
        this->post_filter_->newPoint(p, filtered);
      } else {
        filtered.push_back(p);
      }
    }

  }

  // Now we obtain the fully filtered cloud...
  if (this->post_filter_) {
    this->post_filter_->getFiltered(filtered);
  }
  this->preprocessCloud(cloud_filtered);

  // ...and we're done!
  t.stop();

  //LTRACE << "Total included points " << cloud_filtered->size();
  //PINFO << "Filtering took " << t.duration();
  // Finally, the cloud that is emitted by this instance is the filtered cloud.
  frameData->cloud = cloud_filtered;
  this->setNextFrame(frameData);
  //cout << filtered.size() << "   " << cloud_filtered->size() << endl;
}

}
#endif
