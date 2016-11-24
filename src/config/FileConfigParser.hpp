#ifndef LEPP3_CONFIG_FILE_PARSER_H_
#define LEPP3_CONFIG_FILE_PARSER_H_

#include <iostream>
#include <sstream>
#include <stdexcept>

#include "Parser.h"
#include "lepp3/SurfaceDetector.hpp"
#include "lepp3/SurfaceTracker.hpp"
#include "lepp3/GMMObstacleTracker.hpp"
#include "lepp3/GMMObstacleTrackerState.hpp"

#include "deps/toml.h"

#include "lepp3/ObstacleEvaluator.hpp"
#include "lepp3/util/FileManager.hpp"
#include "lepp3/util/OfflineVideoSource.hpp"

/**
 * A `Parser` implementation that reads the configuration from a config file
 * (given as a parameter at construct time).
 * `tinytoml` is responsible for reading the TOML configuration file.
 */
template<class PointT>
class FileConfigParser : public Parser<PointT> {
public:
  /**
   * Create a new `FileConfigParser` that will read its configuration from the
   * given config file.
   *
   * If the file cannot be opened, there is an error parsing it, or one of the
   * components is misconfigured, the constructor will throw.
   *
   * Uses tiny tinytoml to create a tree out of the available entities in the
   * file. Parser extracts information out of this tree.
   */
  FileConfigParser(std::string const& file_name)
      : file_name_(file_name),
        parser_(toml::parse(file_name)),
        toml_tree_(parser_.value),
        ground_removal_(false),
        surfaceDetectorActive(false),
        obstacleDetectorActive(false) {

    if (!toml_tree_.valid()) {
      throw std::runtime_error("Config parsing error: " + parser_.errorReason);
    }

    this->init();
  }

protected:
  /**
   * Main parsing pipeline. Major parsing steps happen here, with each of them
   * having sub-steps.
   */
  virtual void init() override {
    // The pose service is optional.
    // Compatibility for offline use.
    if (toml_tree_.find("PoseService"))
      initPoseService();
    else
      std::cout << "No pose service info found in config file."
                << "Assuming no robot pose information will be available."
                << std::endl;
    // Existence of a Lola is optional.
    // Compatibility for offline use.
    if (toml_tree_.find("Robot"))
      initRobot();
    else
      std::cout << "No robot info found in config file." << std::endl;

    // Now get the video source ready...
    initRawSource();
    // ...along with any possible filters. Method Parser::buildFilteredSource
    // from the base class is called.
    this->buildFilteredSource();
    // attach any available observers
    addObservers();
    // ...and additional observer processors.
    addAggregators();

    std::cout << "==== Finished parsing the config file. ====" << std::endl;
  }
  void initRobot() {
    double bubble_size = toml_tree_.find("Robot.bubble_size")->as<double>();
    this->robot_.reset(new Robot(*this->pose_service(), bubble_size));
  }

  void addAggregators() {
      std::cout << "entered addAggregators" << std::endl;
      const toml::Value* available = toml_tree_.find("aggregators");
      if (!available)
        return;

      const toml::Array& agg_array = available->as<toml::Array>();
      std::cout << "# aggregators : ";
      std::cout << agg_array.size() << std::endl;
      for (const toml::Value& v : agg_array)
      {
        this->detector_->attachObserver(getAggregator(v));
      }
  }

  // constructs a RobotService from the parameters specified by t
  boost::shared_ptr<AsyncRobotService> getRobotService(const toml::Value& t)
  {
      std::cout << "Constructing new RobotService" << std::endl;

      const std::string target = t.find("target")->as<std::string>();
      std::cout << "Target connection: " << target << std::endl;

      std::string ip = t.find("ip")->as<std::string>();
      std::cout << "\tip:    " << ip << std::endl;

      int port = t.find("port")->as<int>();
      std::cout << "\tPort:  " << port << std::endl;

      int delay = t.find("delay")->as<int>();
      std::cout << "\tDelay: " << delay << std::endl;

      boost::shared_ptr<AsyncRobotService> async_robot_service(new AsyncRobotService(ip, target, port, delay));
      async_robot_service->start();
      return async_robot_service;
  }

  /// Implementations of initialization of various parts of the pipeline.
  void initRawSource() {
    std::cout << "entered initRawSource" << std::endl;
    if (!toml_tree_.find("VideoSource.type"))
      throw "error: no video source found in the config file.";
    const std::string type = toml_tree_.find(
          "VideoSource.type")->as<std::string>();
    bool enable_rgb = false;
    if (toml_tree_.find("VideoSource.enable_rgb"))
      enable_rgb = toml_tree_.find("VideoSource.enable_rgb")->as<bool>();

    if (type == "stream") {
      this->raw_source_ = boost::shared_ptr<VideoSource<PointT> >(
          new LiveStreamSource<PointT>(enable_rgb));
    } else if (type == "pcd") {
      std::string file_path = toml_tree_.find(
            "VideoSource.file_path")->as<std::string>();
      boost::shared_ptr<pcl::Grabber> interface(new pcl::PCDGrabber<PointT>(
            file_path,
            20.,
            true));
      this->raw_source_ = boost::shared_ptr<VideoSource<PointT> >(
          new GeneralGrabberVideoSource<PointT>(interface));
    } else if (type == "oni") {
      std::string file_path = toml_tree_.find(
            "VideoSource.file_path")->as<std::string>();
      std::cout << "oni file path: " << file_path << std::endl;
      boost::shared_ptr<pcl::Grabber> interface(new pcl::io::OpenNI2Grabber(
            file_path,
            pcl::io::OpenNI2Grabber::OpenNI_Default_Mode,
            pcl::io::OpenNI2Grabber::OpenNI_Default_Mode));
      this->raw_source_ = boost::shared_ptr<VideoSource<PointT> >(
          new GeneralGrabberVideoSource<PointT>(interface));
    } else if (type == "am_offline") {
      std::string dir_path = toml_tree_.find(
            "VideoSource.dir_path")->as<std::string>();
      std::cout << "oni file path: " << dir_path << std::endl;
      FileManager fm(dir_path);
      // Prepare the pcd file names required by PCDGrabber
      const std::vector<std::string> file_names = fm.getFileNames(".pcd");
      boost::shared_ptr<pcl::Grabber> pcd_interface(new pcl::PCDGrabber<PointT>(
        file_names,
        30, // frame rate
        true)); // video loop
      // Prepare the filename sequence for cv::VideoCapture
      std::stringstream ss;
      ss << dir_path << "image_%04d.jpg";
      std::cout << "image dir: " << ss.str() << std::endl;
      const boost::shared_ptr<cv::VideoCapture> img_interface(
          new cv::VideoCapture(ss.str()));

      this->raw_source_ = boost::shared_ptr<OfflineVideoSource<PointT> >(
          new OfflineVideoSource<PointT>(pcd_interface, img_interface));
    } else {
      throw "Invalid VideoSource configuration";
    }
  }

  /**
   * Creates and instance of `FilteredVideoSource` based on the previously
   * acquired `VideoSource` object instance.
   */
  virtual void initFilteredVideoSource() override {
    const std::string type = toml_tree_.find(
          "FilteredVideoSource.type")->as<std::string>();
    if (type == "simple") {
      this->filtered_source_.reset(
          new SimpleFilteredVideoSource<PointT>(this->raw_source_));
    } else if (type == "prob") {
      this->filtered_source_.reset(
          new ProbFilteredVideoSource<PointT>(this->raw_source_));
    } else if (type == "pt1") {
      this->filtered_source_.reset(
          new Pt1FilteredVideoSource<PointT>(this->raw_source_));
    } else {
      throw "Invalid FilteredVideoSource configuration";
    }
  }

  /**
   * Reads and sets up the available filters for the `FilteredVideoSource`
   * instance. Filter initialization for each entry is done by calling
   * `getPointFilter`.
   */
  virtual void addFilters() override {
    const toml::Value* value = toml_tree_.find("FilteredVideoSource.filters");
    if (value == nullptr)
      return;

    const toml::Array& filter_array = value->as<toml::Array>();
    for (const toml::Value& v : filter_array) {
      this->filtered_source_->addFilter(getPointFilter(v));
    }
  }

  void initPoseService() {
    std::string ip = toml_tree_.find("PoseService.ip")->as<std::string>();
    int port = toml_tree_.find("PoseService.port")->as<int>();

    this->pose_service_.reset(new PoseService(ip, port));
    this->pose_service_->start();
  }

  void addObservers()
  {
    std::cout << "entered addObservers" << std::endl;
    const toml::Value* available = toml_tree_.find("observers");
    if (!available)
      return;
    const toml::Array& obs_arr = available->as<toml::Array>();
    // Iterate though all the available observer entries in the config file and
    // set them up accordingly.
    for (const toml::Value& v : obs_arr)
    {
      std::string const type = v.find("type")->as<std::string>();
      std::cout << "observer type: " << type << std::endl;
      if (type == "ObstacleDetector")
      {
        obstacleDetectorActive = true;
        // Set up the basic surface detector for ground removal, if not already
        // initialized.
        if (!ground_removal_)
          initSurfaceFinder();
        std::string const method = v.find("method")->as<std::string>();
        initObstacleDetector(method);
      }
      else if (type == "SurfaceDetector")
      {
        surfaceDetectorActive = true;
        // Set up the basic surface detector for ground removal, if not already
        // initialized.
        if (!ground_removal_)
          initSurfaceFinder();
        initSurfaceDetector();
      }
      else if (type == "Recorder")
      {
        initRecorder();
      }
      else if (type == "CameraCalibrator")
      {
        initCamCalibrator();
      }
      else if (type == "ARVisualizer")
      {
        toml::Value const* visualizer = v.find("visualizer");
        if (!visualizer)
        {
          std::cout << "[[observers.visualizer]] not found!" << std::endl;
          std::cout << "Cannot create a Visualizer without specifying the parameters!" << std::endl;
          exit(1);
        }
        toml::Array const& array = visualizer->as<toml::Array>();
        /*
         * TODO add sanity check for the case where there's more than one visualizer
         * instance inside one definition (=illegal basewd on definition).
         */
        auto vis = array[0];
        this->visualizers_.push_back(getVisualizer(vis));
        std::cout << "visualizer added" << std::endl;
      }
    }
    // Initialize obstacle and surface detector if necessary
    if (surfaceDetectorActive || obstacleDetectorActive) {
      initSurfaceDetector();
    }
  }

  virtual boost::shared_ptr<SplitStrategy<PointT> > buildSplitStrategy() override {
    std::cout << "entered buildSplitStrategy" << std::endl;
    boost::shared_ptr<CompositeSplitStrategy<PointT> > split_strat(
        new CompositeSplitStrategy<PointT>);

    // First find the axis on which the splits should be made
    std::string axis_id = toml_tree_.find(
          "SplitStrategy.split_axis")->as<std::string>();
    if (axis_id == "largest") {
      split_strat->set_split_axis(SplitStrategy<PointT>::Largest);
    } else if (axis_id == "middle") {
      split_strat->set_split_axis(SplitStrategy<PointT>::Middle);
    } else if (axis_id == "smallest") {
      split_strat->set_split_axis(SplitStrategy<PointT>::Smallest);
    } else {
      throw "Invalid axis identifier";
    }

    // Now add all conditions
    const toml::Array& cond_array = toml_tree_.find(
          "SplitStrategy.conditions")->as<toml::Array>();
    std::cout << "# conditions: " << cond_array.size() << std::endl;
    for (const toml::Value &v : cond_array) {
      const std::string type = v.find("type")->as<std::string>();
      std::cout << "splitcondition: " << type << std::endl;
      if (type == "SizeLimit") {
        double size = v.find("size")->as<double>();
        split_strat->addSplitCondition(boost::shared_ptr<SplitCondition<PointT> >(
                new SizeLimitSplitCondition<PointT>(size)));
      } else if (type == "DepthLimit") {
        int depth = v.find("depth")->as<int>();
        split_strat->addSplitCondition(boost::shared_ptr<SplitCondition<PointT> >(
                new DepthLimitSplitCondition<PointT>(depth)));
      } else if (type == "DistanceThreshold") {
        int distance = v.find("distance_threshold")->as<int>();
        split_strat->addSplitCondition(boost::shared_ptr<SplitCondition<PointT> >(
                new DistanceThresholdSplitCondition<PointT>(distance, *this->robot())));
      } else if (type == "ShapeCondition") {
        double sphere1 = v.find("sphere1")->as<double>();
        double sphere2 = v.find("sphere2")->as<double>();
        double cylinder = v.find("cylinder")->as<double>();
        split_strat->addSplitCondition(boost::shared_ptr<SplitCondition<PointT> >(
                new ShapeSplitCondition<PointT>(sphere1, sphere2, cylinder)));
      } else {
        throw "Unknown split condition given.";
      }
    }
    std::cout << "returning current split_strat" << std::endl;
    return split_strat;
  }

  void loadSurfaceFinderParameters(std::vector<double> &surfFinderParameters)
  {
    surfFinderParameters.push_back(toml_tree_.find("RANSAC.maxIterations")->as<int>());
    surfFinderParameters.push_back(toml_tree_.find("RANSAC.distanceThreshold")->as<double>());
    surfFinderParameters.push_back(toml_tree_.find("RANSAC.minFilterPercentage")->as<double>());
    surfFinderParameters.push_back(toml_tree_.find("Classification.deviationAngle")->as<double>());
  }

  void loadPlaneInlierFinderParameters(std::vector<double> &planeInlierFinderParameters)
  {
    planeInlierFinderParameters.push_back(toml_tree_.find("InlierFinder.minDistToPlane")->as<double>());
  }


  void loadSurfaceClustererParameters(std::vector<double> &surfaceClusterParameters)
  {
    surfaceClusterParameters.push_back(toml_tree_.find("Clustering.clusterTolerance")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Clustering.minClusterSize")->as<int>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.coarseVoxelSize_X")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.coarseVoxelSize_Y")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.coarseVoxelSize_Z")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.fineVoxelSize_X")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.fineVoxelSize_Y")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.fineVoxelSize_Z")->as<double>());
    surfaceClusterParameters.push_back(toml_tree_.find("Downsampling.coarseFineLimit")->as<int>());
  }


  void loadSurfaceTrackerParameters(std::vector<double> &surfaceTrackerParameters)
  {
    surfaceTrackerParameters.push_back(toml_tree_.find("SurfaceTracking.lostLimit")->as<int>());
    surfaceTrackerParameters.push_back(toml_tree_.find("SurfaceTracking.foundLimit")->as<int>());
    surfaceTrackerParameters.push_back(toml_tree_.find("SurfaceTracking.maxCenterDistance")->as<double>());
    surfaceTrackerParameters.push_back(toml_tree_.find("SurfaceTracking.maxRadiusDeviationPercentage")->as<double>());
  }

  void loadConvexHullParameters(std::vector<double> &convexHullParameters)
  {
    convexHullParameters.push_back(toml_tree_.find("ConvexHullApproximation.numHullPoints")->as<int>());
    convexHullParameters.push_back(toml_tree_.find("ConvexHullApproximation.mergeUpdatePercentage")->as<double>());
  }


  /**
   * Initiailizes the obstacle detector.
   */
  virtual void initObstacleDetector(std::string const& method) {

    // Setup plane inlier finder
    // TODO: [Sahand] There's an ambiguity here. There's the `initSurfaceFinder`
    //       which is initialized regardless of having a `SurfaceDetector` or
    //       `ObstacleDetector`, and then there is the following 4 lines. Inspect
    //       why they had this plane inlier finder right before the obstacle
    //       detector. Does it mean the `PlaneInlierFinder` tries to remove all
    //       those surfaces found by the `initSurfaceFinder` to end of with only
    //       obstacles? Or is it a redundant step?
    std::vector<double> planeInlierFinderParameters;
    loadPlaneInlierFinderParameters(planeInlierFinderParameters);
    inlier_finder_.reset(new PlaneInlierFinder<PointT>(planeInlierFinderParameters));
    surface_detector_->FrameDataSubject::attachObserver(inlier_finder_);

    if (method == "EuclideanClustering") {
      // Prepare the approximator that the detector is to use.
      // First, the simple approximator...
      boost::shared_ptr<ObjectApproximator<PointT> > simple_approx(
          this->getApproximator());
      // ...then the split strategy
      boost::shared_ptr<SplitStrategy<PointT> > splitter(
          this->buildSplitStrategy());
      // ...finally, wrap those into a `SplitObjectApproximator` that is given
      // to the detector.
      boost::shared_ptr<ObjectApproximator<PointT> > approx(
          new SplitObjectApproximator<PointT>(simple_approx, splitter));
      // Prepare the base detector...
      base_obstacle_detector_.reset(
          new ObstacleDetector<PointT>(approx, surfaceDetectorActive));
      this->source()->FrameDataSubject::attachObserver(base_obstacle_detector_);
      // Smooth out the basic detector by applying a smooth detector to it
      boost::shared_ptr<LowPassObstacleTracker> low_pass_obstacle_tracker(
          new LowPassObstacleTracker);
      base_obstacle_detector_->FrameDataSubject::attachObserver(low_pass_obstacle_tracker);
      // Now the detector that is exposed via the context is a smoothed-out
      // base detector.
      this->detector_ = low_pass_obstacle_tracker;
    } else if (method == "GMM") {
      // parse [ObstacleTracking] parameters
      GMM::ObstacleTrackerParams params = readGMMObstacleTrackerParams();

      boost::shared_ptr<GMMObstacleTracker> obstacle_tracker(new GMMObstacleTracker(params));
      this->detector_ = obstacle_tracker;
    #ifndef ENABLE_RECORDER
      inlier_finder_->FrameDataSubject::attachObserver(obstacle_tracker);
    #endif

      std::cout << "initialized obstacle tracker" << std::endl;
    } else {
      std::cerr << "Unknown obstacle detector method`" << method << "`" << std::endl;
      throw "Unknown obstacle detector method";
    }
  }

  virtual void initSurfaceDetector() override {
    // initialize surface clusterer
    std::vector<double> surfaceClusterParameters;
  loadSurfaceClustererParameters(surfaceClusterParameters);
    surface_clusterer_.reset(new SurfaceClusterer<PointT>(surfaceClusterParameters));
    surface_detector_->SurfaceDataSubject::attachObserver(surface_clusterer_);
    // initialize tracking of surfaces
    std::vector<double> surfaceTrackerParameters;
    loadSurfaceTrackerParameters(surfaceTrackerParameters);
    surface_tracker_.reset(new SurfaceTracker<PointT>(surfaceTrackerParameters));
    surface_clusterer_->SurfaceDataSubject::attachObserver(surface_tracker_);
    // initialize convex hull detector
    std::vector<double> convexHullParameters;
    loadConvexHullParameters(convexHullParameters);
    convex_hull_detector_.reset(new ConvexHullDetector(convexHullParameters));
    surface_tracker_->SurfaceDataSubject::attachObserver(convex_hull_detector_);
    // connect convex hull detector back to surfaceDetector (close the loop)
    convex_hull_detector_->SurfaceDataSubject::attachObserver(surface_detector_);
  }


  virtual void initRecorder() override {
    std::cout << "entered initRecorder" << std::endl;
    this->recorder_.reset(new VideoRecorder<PointT>());
    std::cout << "recorder got reset!" << std::endl;
    const bool rec_cloud = toml_tree_.find(
          "RecordingOptions.cloud")->as<bool>();
    const bool rec_rgb = toml_tree_.find(
          "RecordingOptions.rgb")->as<bool>();
    const bool rec_pose = toml_tree_.find(
          "RecordingOptions.pose")->as<bool>();
    std::cout << "\trec_cloud: " << rec_cloud
              << "\n\trec_rgb: " << rec_rgb
              << "\n\trec_pose: " << rec_pose << std::endl;

    // set the VideoGrabber options (whether to subscribe to cloud/rgb) ...
    std::map<std::string, bool> video_options;
    video_options.insert(
          std::pair<std::string, bool>("subscribe_cloud", rec_cloud));
    video_options.insert(
          std::pair<std::string, bool>("subscribe_image", rec_rgb));

    this->source()->setOptions(video_options);

    // ... and set the VideoRecorder options accordingly.
    this->recorder_->setMode(rec_cloud, rec_rgb, rec_pose);

    if (rec_cloud)
      this->source()->FrameDataSubject::attachObserver(this->recorder());
    if (rec_rgb)
      this->raw_source()->RGBDataSubject::attachObserver(this->recorder());
    if (rec_pose)
      this->pose_service_->attachObserver(this->recorder());
  }

//  void loadARVisualizerParams(double position[],double up[],double forward[])
//  {
//    std::string type= toml_tree_.find("ARVisualizer.frame")->as<std::string>();
//    if (type == "pcl") {
//      position[0]=toml_tree_.find("ARVisualizer.pcl_positionx")->as<int>();
//      position[1]=toml_tree_.find("ARVisualizer.pcl_positiony")->as<int>();
//      position[2]=toml_tree_.find("ARVisualizer.pcl_positionz")->as<int>();
//      up[0]=toml_tree_.find("ARVisualizer.pcl_upx")->as<int>();
//      up[1]=toml_tree_.find("ARVisualizer.pcl_upy")->as<int>();
//      up[2]=toml_tree_.find("ARVisualizer.pcl_upz")->as<int>();
//      forward[0]=toml_tree_.find("ARVisualizer.pcl_forwardx")->as<int>();
//      forward[1]=toml_tree_.find("ARVisualizer.pcl_forwardy")->as<int>();
//      forward[2]=toml_tree_.find("ARVisualizer.pcl_forwardz")->as<int>();
//    }
//    else if (type == "lola") {
//      position[0]=toml_tree_.find("ARVisualizer.lola_positionx")->as<int>();
//      position[1]=toml_tree_.find("ARVisualizer.lola_positiony")->as<int>();
//      position[2]=toml_tree_.find("ARVisualizer.lola_positionz")->as<int>();
//      up[0]=toml_tree_.find("ARVisualizer.lola_upx")->as<int>();
//      up[1]=toml_tree_.find("ARVisualizer.lola_upy")->as<int>();
//      up[2]=toml_tree_.find("ARVisualizer.lola_upz")->as<int>();
//      forward[0]=toml_tree_.find("ARVisualizer.lola_forwardx")->as<int>();
//      forward[1]=toml_tree_.find("ARVisualizer.lola_forwardy")->as<int>();
//      forward[2]=toml_tree_.find("ARVisualizer.lola_forwardz")->as<int>();
//    } else {
//      throw "Unknown AR frame condition given.";
//    }
//  }
//
//  void initVisualizer()
//  {
//    double position[3]={0};
//    double forward[3]={0};
//    double up[3]={0};
//
//    loadARVisualizerParams(position,up,forward);
//
//    if (surfaceDetectorActive && !obstacleDetectorActive)
//    {
//      ar_visualizer_.reset(new ARVisualizer(surfaceDetectorActive, obstacleDetectorActive, position,forward,up));
//      surface_detector_->FrameDataSubject::attachObserver(ar_visualizer_);
//    }
//    else if (obstacleDetectorActive)
//    {
//      ar_visualizer_.reset(new ARVisualizer(surfaceDetectorActive, obstacleDetectorActive,position,forward,up));
//      this->detector_->FrameDataSubject::attachObserver(ar_visualizer_);
//    }
//
//    bool viz_cloud = toml_tree_.find("Visualization.cloud")->as<bool>();
//    if (viz_cloud) {
//      // TODO add relevant stuff for cloud visualization and any necessary
//      // observer
//    }

  virtual void initCamCalibrator() override {
    std::cout << "entered initCamCalibrator" << std::endl;
    this->cam_calibrator_.reset(new CameraCalibrator<PointT>);
    this->source()->FrameDataSubject::attachObserver(this->cam_calibrator());
  }


private:
  /// Helper functions for constructing parts of the pipeline.
  /**
   * A helper function that reads all the parameters that are required by the
   * `GMMObstacleTracker`.
   */
  GMM::DebugGUIParams readGMMDebugGuiParams(toml::Value const* v) {
    bool const enableTracker = v->find("enable_tracker")->as<bool>();
    bool const enableTightFit = v->find("enable_tight_fit")->as<bool>();
    bool const drawGaussians = v->find("draw_gaussians")->as<bool>();
    bool const drawSSVs = v->find("draw_ssv")->as<bool>();
    bool const drawTrajectories = v->find("draw_trajectories")->as<bool>();
    bool const drawVelocities = v->find("draw_velocities")->as<bool>();
    bool const drawDebugValues = v->find("draw_debug_values")->as<bool>();
    bool const drawVoxels = v->find("draw_voxels")->as<bool>();

    int const trajectoryLength = v->find("trajectory_length")->as<int>();
    // TODO decide how to read ar::color info from config file
    // ar::Color gaussianColor;
    // ar::Color ssvColor;
    float const downsampleResolution = static_cast<float>(v->find("downsample_res")->as<double>());

    GMM::ColorMode colorMode;
    if (v->find("color_mode")) {
      std::string const m = v->find("color_mode")->as<std::string>();

      if (m == "NONE") {
        colorMode = GMM::ColorMode::NONE;
      } else if (m == "SOFT_ASSIGNMENT") {
        colorMode = GMM::ColorMode::SOFT_ASSIGNMENT;
      } else if (m == "HARD_ASSIGNMENT") {
        colorMode = GMM::ColorMode::HARD_ASSIGNMENT;
      } // TODO set the last missing value in GMM::ColorMode
    }

    GMM::DebugGUIParams params;
    params.enableTracker = enableTracker;
    params.enableTightFit = enableTightFit;
    params.drawGaussians = drawGaussians;
    params.drawSSVs = drawSSVs;
    params.drawTrajectories = drawTrajectories;
    params.drawVelocities = drawVelocities;
    params.drawDebugValues = drawDebugValues;
    params.drawVoxels = drawVoxels;
    params.trajectoryLength = trajectoryLength;
    params.downsampleResolution = downsampleResolution;
    params.colorMode = colorMode;
    // TODO add missing values gassuanColor, ssvColor

    return params;
  }

  GMM::ObstacleTrackerParams readGMMObstacleTrackerParams() {
    toml::Value const* v = toml_tree_.find("GMMObstacleDetectorParams");
    GMM::ObstacleTrackerParams params;
    params.enableTightFit = v->find("enable_tight_fit")->as<bool>();
    params.filterSSVPositions = v->find("filter_ssv_position")->as<bool>();
    params.voxelGridResolution = v->find("voxel_grid_resolution")->as<double>();
    params.kalman_SystemNoisePosition = v->find("kalman_system_noise_position")->as<double>();
    params.kalman_SystemNoiseVelocity = v->find("kalman_system_noise_velocity")->as<double>();
    params.kalman_MeasurementNoise = v->find("kalman_measurement_noise")->as<double>();

    return params;

  }

  void initSurfaceFinder() {
    // A basic surfaceDetector is ALWAYS active because ground is always removed
    std::vector<double> surfFinderParameters;
    loadSurfaceFinderParameters(surfFinderParameters);
    surface_detector_.reset(new SurfaceDetector<PointT>(surfaceDetectorActive,surfFinderParameters));
    this->source()->FrameDataSubject::attachObserver(surface_detector_);

    ground_removal_ = true;
  }
  /**
   * A helper function that constructs the next `PointFilter` instance,
   * as defined in the following lines of the config file.
   * If the lines are invalid, an exception is thrown.
   */
  boost::shared_ptr<PointFilter<PointT> > getPointFilter(toml::Value const& v) {
    std::string const type = v.find("type")->as<std::string>();
    if (type == "SensorCalibrationFilter") {
      double a = v.find("a")->as<double>();
      double b = v.find("b")->as<double>();
      return boost::shared_ptr<PointFilter<PointT> >(
          new SensorCalibrationFilter<PointT>(a, b));
    } else if (type == "RobotOdoTransformer") {
      return boost::shared_ptr<PointFilter<PointT> >(
          new RobotOdoTransformer<PointT>(this->pose_service_));
    } else if (type == "FileOdoTransformer") {
      std::string const file_name = toml_tree_.find(
            "file_path")->as<std::string>();
      return boost::shared_ptr<PointFilter<PointT> >(
          new FileOdoTransformer<PointT>(file_name));
    } else if (type == "TruncateFilter") {
      int decimals = v.find("decimal_points")->as<int>();
      return boost::shared_ptr<PointFilter<PointT> >(
          new TruncateFilter<PointT>(decimals));
    } else if (type == "CropFilter") {
      double xmax = v.find("xmax")->as<double>();
      double xmin = v.find("xmin")->as<double>();
      double ymax = v.find("ymax")->as<double>();
      double ymin = v.find("ymin")->as<double>();
      return boost::shared_ptr<PointFilter<PointT> >(
          new CropFilter<PointT>(xmax, xmin, ymax, ymin));
    } else {
      std::cerr << "Unknown filter type `" << type << "`" << std::endl;
      throw "Unknown filter type";
    }
  }

  boost::shared_ptr<BaseVisualizer> getVisualizer(toml::Value const& v) {
    std::cout << "Entered getVisualizer" << std::endl;

    std::string const name = v.find("name")->as<std::string>();
    int const height = v.find("height")->as<int>();
    int const width = v.find("width")->as<int>();

    std::string const type = v.find("type")->as<std::string>();
    std::cout << "visualizer.type: " << type << std::endl;
    if (type == "CameraCalibrator") {
      boost::shared_ptr<CalibratorVisualizer<PointT> > calib_visualizer(
          new CalibratorVisualizer<PointT>(name, width, height));
      this->source()->FrameDataSubject::attachObserver(calib_visualizer);
      this->cam_calibrator()->attachCalibrationAggregator(calib_visualizer);
      return calib_visualizer;
    } else if (type == "ObsSurfVisualizer") {
      // TODO: [Sahand] decide whether to define the following two variables.
      //       We already have `surfaceDetectorActive` and `obstacleDetectorActive`
      //       which are set according to the availability of their components.
      //       (Reason to have it here as well): There might be the case where
      //       we want to run all of the components, but only visualizing some
      //       of them.
      bool show_obstacles = false;
      bool show_surfaces = false;
      show_obstacles = v.find("show_obstacles")->as<bool>();
      show_surfaces = v.find("show_surfaces")->as<bool>();
      
      boost::shared_ptr<ObsSurfVisualizer> obs_surf_vis(
          new ObsSurfVisualizer(name, show_obstacles, show_surfaces, width, height));
      this->source()->FrameDataSubject::attachObserver(obs_surf_vis);
      return obs_surf_vis;
    } else if (type == "GMMTrackingVisualizer") {
      // TODO read all the necessary parameters from the config file.
      bool const debugGUI = v.find("debugGUI")->as<bool>();
      // Set default values for GUI Debug parameters.
      GMM::DebugGUIParams d_gui_params;
      // If there are debug parameters available in the config, get 'em
      if(debugGUI) {
        toml::Value const* debug_gui = v.find("debugGUIParams");

        d_gui_params = readGMMDebugGuiParams(debug_gui);
      }
      return boost::shared_ptr<ObstacleTrackerVisualizer>(
          new ObstacleTrackerVisualizer(d_gui_params, name, width, height));
    }
    else {
      std::cerr << "Unknown visualizer type `" << type << "`" << std::endl;
      throw "Unknown visualizer type";
    }
  }

  /**
   * A helper function that constructs the next `ObstacleAggregator` instance,
   * as defined in the following lines of the config file.
   * If the lines are invalid, an exception is thrown.
   */
  boost::shared_ptr<FrameDataObserver> getAggregator(toml::Value const& v) {
    std::string const type = v.find("type")->as<std::string>();
    std::cout << "agg type: " << type << std::endl;
    if (type == "RobotAggregator") {
      int const update_frequency= v.find("update_frequency")->as<int>();
      std::vector<std::string> datatypes = v.find("data")->as<std::vector<std::string>>();

      std::cout << "find robot service..."<< std::endl;
      const toml::Value* robotServiceEntry = v.find("RobotService");
      if (!robotServiceEntry)
      {
        std::cout << "[[aggregators.RobotService]] not found!" << std::endl;
        std::cout << "  Cannot create RobotAggregator without RobotService!" << std::endl;
        exit(1);
      }

      const toml::Array& agg_array = robotServiceEntry->as<toml::Array>();
      if (agg_array.size() != 1)
      {
        std::cout << "Expected exactly 1 RobotService entry, found: " << agg_array.size() << std::endl;
        exit(1);
      }

      auto robotService = getRobotService(agg_array[0]);

      // attach to RGB data here since we always assume we're dealing with FrameDataObservers elsewhere...
      boost::shared_ptr<RobotAggregator> robotAggregator = boost::make_shared<RobotAggregator>(robotService, update_frequency, datatypes, *this->robot());
      boost::static_pointer_cast<RGBDataSubject>(this->raw_source_)->attachObserver(robotAggregator);
      return robotAggregator;
      } else if (type == "ObstacleEvaluator") {
      int const ref_volume = v.find("ref_volume")->as<int>();
      return boost::shared_ptr<ObstacleEvaluator>(
          new ObstacleEvaluator(ref_volume));
    } else {
      std::cerr << "Unknown aggregator type `" << type << "`" << std::endl;
      throw "Unknown aggregator type";
    }
  }

  /// Private members
  std::string const& file_name_;
  /**
   * A handle to tinytoml's Parseresult. The object receives the path to TOML
   * file and tries to build a tree as an output. This will be then used by
   * toml::Value to get access to each element.
   */
  toml::ParseResult const parser_;
  /**
   * Object containing all the available values from the TOML file which is read
   * by the toml::ParseResult::parse method. Any data could be directly accessed
   * from this object.
   */
  toml::Value const& toml_tree_;
  /**
   * The base detector that we attach to the video source and to which, in
   * turn, the "smooth" detector is attached. The `Parser` maintains a
   * reference to it to make sure it doesn't get destroyed, although it is
   * never exposed to any outside clients.
   */
  boost::shared_ptr<ObstacleDetector<PointT> > base_obstacle_detector_;
  boost::shared_ptr<SurfaceDetector<PointT> > surface_detector_;
  boost::shared_ptr<SurfaceClusterer<PointT> > surface_clusterer_;
  boost::shared_ptr<SurfaceTracker<PointT> > surface_tracker_;
  boost::shared_ptr<ConvexHullDetector> convex_hull_detector_;
  boost::shared_ptr<PlaneInlierFinder<PointT> > inlier_finder_;

  bool surfaceDetectorActive;
  bool obstacleDetectorActive;
  bool ground_removal_;
};

#endif
