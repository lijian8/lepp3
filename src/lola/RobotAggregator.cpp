#include "lola/RobotAggregator.h"
#include "deps/easylogging++.h"

using namespace lepp;

namespace {
/**
 * A `ModelVisitor` implementation used for the implementation of the
 * `RobotAggregator`. Allows us to obtain all information that is required to
 * assemble a message for the robot.
 *
 * Similar to the `ParametersVisitor`, but for the sake of convenience of
 * the two aggregators' implementations, they are not reconciled.
 */
class CoefsVisitor : public lepp::ModelVisitor {
public:
  void visitSphere(SphereModel& sphere) {
    coefs_.push_back(sphere.center().x);
    coefs_.push_back(sphere.center().y);
    coefs_.push_back(sphere.center().z);
    for (size_t i = 0; i < 6; ++i) coefs_.push_back(0);

    type_id_ = 0;
    radius_ = sphere.radius();
  }

  void visitCapsule(CapsuleModel& capsule) {
    coefs_.push_back(capsule.first().x);
    coefs_.push_back(capsule.first().y);
    coefs_.push_back(capsule.first().z);
    coefs_.push_back(capsule.second().x);
    coefs_.push_back(capsule.second().y);
    coefs_.push_back(capsule.second().z);
    for (size_t i = 0; i < 3; ++i) coefs_.push_back(0);

    type_id_ = 1;
    radius_ = capsule.radius();
  }

  std::vector<double> const& coefs() const { return coefs_; }
  double radius() const { return radius_; }
  int type_id() const { return type_id_; }
private:
  std::vector<double> coefs_;
  int type_id_;
  double radius_;
};
}  // namespace <anonymous>

RobotAggregator::RobotAggregator(boost::shared_ptr<RobotService> service,
                                int freq,
                                std::vector<std::string> datatypes,
                                Robot& robot,
                                double min_surface_height,
                                double surface_normal_tolerance
                              )
    : service_(service), diff_(freq), next_id_(0),
      min_surface_height(min_surface_height),
      surface_normal_tolerance(surface_normal_tolerance),
      robot_(robot) {

  for (auto t : datatypes)
  {
    if (t == "obstacles")
      send_obstacles_ = true;
    else if (t == "surfaces")
      send_surfaces_ = true;
    else if (t == "pointclouds")
      send_pointclouds_ = true;
    else if (t == "images")
      send_images_ = true;
    else
      std::cout << "RobotAggregator: Unexpected datatype '" << t << "'" << std::endl;
  }

  // Set up the callbacks that handle the particular cases.
  if (send_obstacles_)
  {
    diff_.set_new_obstacle_callback(boost::bind(&RobotAggregator::new_obstacle_cb_, this, _1, _2));
    diff_.set_modified_obstacle_callback(boost::bind(&RobotAggregator::mod_obstacle_cb_, this, _1, _2));
    diff_.set_deleted_obstacle_callback(boost::bind(&RobotAggregator::del_obstacle_cb_, this, _1, _2));
  }

  if (send_surfaces_)
  {
    diff_.set_new_surface_callback(boost::bind(&RobotAggregator::new_surface_cb_, this, _1, _2));
    diff_.set_modified_surface_callback(boost::bind(&RobotAggregator::mod_surface_cb_, this, _1, _2));
    diff_.set_deleted_surface_callback(boost::bind(&RobotAggregator::del_surface_cb_, this, _1, _2));

    // prepare dummy convex hull for surfaces we don't want the planning system to consider
    dummy_hull.reset(new PointCloudT());
    dummy_hull->width = 8;
    dummy_hull->height = 1;
    dummy_hull->is_dense = false;
    dummy_hull->points.resize(dummy_hull->width * dummy_hull->height);
    for (size_t i = 0; i < dummy_hull->width * dummy_hull->height; i++)
    {
      (*dummy_hull)[i].x = -4.9 + (sin((float)i) * 0.01f);
      (*dummy_hull)[i].y = -4.9 + (cos((float)i) * 0.01f);
      (*dummy_hull)[i].z = -4.9 + ((float)i * 0.01f);
    }
  }
}

bool RobotAggregator::shouldSendSurface(SurfaceModel& model) {
  if (fabs(model.centerpoint().z) < min_surface_height)
  {
    double angle = acos(model.get_normal().dot(Eigen::Vector3d(0, 0, 1)));
    if (angle < surface_normal_tolerance)
      return false;
  }
  return true;
}

void RobotAggregator::new_obstacle_cb_(ObjectModel& model, long frame_num) {
  std::vector<ObjectModel*> primitives(getPrimitives(model));
  size_t const sz = primitives.size();
  std::vector<int>& ids = robot_ids_[model.id()];
  // First, assign an ID to the entire model.
  int const model_id = nextId();
  ids.push_back(model_id);
  // Now, create each part, assigning an ID to each of them.
  for (size_t i = 0; i < sz; ++i) {
    // Assign it a new ID
    int const id = nextId();
    ids.push_back(id);
    // ...and send a message to the robot.
    if (i == 0) {
      // When creating the first part, we implicitly also create the parent model
      sendNew(*primitives[i], model_id, id, frame_num);
    } else {
      // The other parts are considered modifications of the parent...
      sendModify(*primitives[i], model_id, id, frame_num);
    }
  }
}
void RobotAggregator::new_surface_cb_(SurfaceModel& model, long frame_num) {
  auto hull_pts = model.get_hull();
  std::cout << "New Surface (" << model.id() << "): [ " << std::endl;
  for (auto& p : *hull_pts)
  {
    std::cout << "  " << p.x << ", " << p.y << ", " << p.z << std::endl;
  }
  std::cout << "]" << std::endl;
  if (shouldSendSurface(model))
  {
    sendNew(model, frame_num);
  }
  else
  {
    SurfaceModel hack = model;
    hack.set_hull(dummy_hull);
    sendNew(hack, frame_num);
  }
}

bool RobotAggregator::del_obstacle_cb_(ObjectModel& model, long frame_num) {
  // Disable any deletions of objects that are too close to the robot.
  // We don't want to confuse it by sending it delete commands
  // for objects that it might be in the process of stepping over.
  if (robot_.isInRobotBoundary(model)) {
    return false;
  }

  int const obj_id = model.id();
  {
    // Delete the entire model with a single message
    std::vector<int>& ids = robot_ids_[obj_id];
    // The first ID is always the ID of the object itself.
    sendDeleteObstacle(ids[0], frame_num);
    // ...the reference to the vector is invalid after the erase
    // so forget it before then by closing the scope to make sure
    // no dangling pointer accesses occur.
  }
  // Remove it from the map too.
  robot_ids_.erase(obj_id);
  // Signal the diff aggregator to definitely delete this object
  return true;
}

bool RobotAggregator::del_surface_cb_(SurfaceModel& model, long frame_num) {
  // Disable any deletions of surfaces that are too close to the robot.
  // We don't want to confuse it by sending it delete commands
  // for objects that it might be in the process of stepping over.
  if (robot_.isInRobotBoundary(model)) {
     return false;
  }

  sendDeleteSurface(model.id(), frame_num);
  return true;
}

void RobotAggregator::mod_obstacle_cb_(ObjectModel& model, long frame_num) {
  // Disable any modifications to models that are too close to the
  // robot.
  if (robot_.isInRobotBoundary(model)) {
    return;
  }

  std::vector<ObjectModel*> primitives(getPrimitives(model));
  size_t const new_size = primitives.size();
  std::vector<int>& ids = robot_ids_[model.id()];
  for (auto& id : ids)
  {
    std::cout << id << ", ";
  }
  std::cout << "]" << std::endl;
  // The ids list includes the ID of the object itself (at index 0), not only
  // the primitives.
  // There always need to be at least two IDs in the vector (the model and at
  // least one of its parts), therefore this will not underflow the `size_t`.
  size_t const old_size = ids.size() - 1;

  if (new_size < old_size) {
    // Some parts need to be deleted.
    for (size_t i = 0; i < old_size - new_size; ++i) {
      sendDeleteObstaclePart(ids[0], ids[old_size - i - 1], frame_num);
      ids.pop_back();
    }
  }

  // Now we modify what we have left from before
  size_t const sz = ids.size();
  for (size_t i = 1; i < sz; ++i) {
    sendModify(*primitives[i - 1], ids[0], ids[i], frame_num);
  }

  // And finally add new ones, if necessary
  if (new_size > old_size) {
    for (size_t i = 0; i < new_size - old_size; ++i) {
      int const id = nextId();
      ids.push_back(id);
      // New parts are modifications of the parent; the ID of the entire model
      // is always the first ID in the `ids` vector.
      sendModify(*primitives[old_size + i], ids[0], id, frame_num);
    }
  }
}

void RobotAggregator::mod_surface_cb_(SurfaceModel& model, long frame_num) {
// Disable any modifications to models that are too close to the
// robot.
auto hull_pts = model.get_hull();
std::cout << "Modify Surface (" << model.id() << "): [ " << std::endl;
for (auto& p : *hull_pts)
{
  std::cout << "  " << p.x << ", " << p.y << ", " << p.z << std::endl;
}
std::cout << "]" << std::endl;

 if (robot_.isInRobotBoundary(model)) {
   return;
 }

  if (shouldSendSurface(model))
  {
    sendModify(model, frame_num);
  }
  else
  {
    SurfaceModel hack = model;
    hack.set_hull(dummy_hull);
    sendModify(hack, frame_num);
  }
}

std::vector<ObjectModel*> RobotAggregator::getPrimitives(ObjectModel& model) const {
  FlattenVisitor flattener;
  model.accept(flattener);
  return flattener.objs();
}

void RobotAggregator::sendNew(ObjectModel& new_model, int model_id, int part_id, long frame_num) {
  CoefsVisitor coefs;
  new_model.accept(coefs);

  VisionMessage msg = VisionMessage(ObstacleMessage::SetMessage(
      coefs.type_id(), model_id, part_id, coefs.radius(), coefs.coefs()),
      frame_num);
  LINFO << "RobotAggregator: Creating new primitive ["
        << "type = " << coefs.type_id()
        << "; id = " << part_id
        << "]";
  service_->sendMessage(msg);
}

void RobotAggregator::sendNew(SurfaceModel& new_surface, long frame_num) {
  std::vector<float> vertices;
  std::vector<float> normal = {new_surface.get_planeCoefficients().values[0],new_surface.get_planeCoefficients().values[1],new_surface.get_planeCoefficients().values[2]};
  for(auto point : new_surface.get_hull()->points)
  {
    vertices.push_back(point.x);
    vertices.push_back(point.y);
    vertices.push_back(point.z);
  }

  VisionMessage msg = VisionMessage(SurfaceMessage::SetMessage(new_surface.id(), normal, vertices), frame_num);

  // LINFO << "RobotAggregator: Creating new surface ["
  //       << "id = " << new_surface.id()
  //       << "]";
  service_->sendMessage(msg);
}

void RobotAggregator::sendDeleteObstacle(int id, long frame_num) {
  // LINFO << "RobotAggregator: Deleting a primitive id = "
  //       << id;
  VisionMessage del = VisionMessage(ObstacleMessage::DeleteMessage(id), frame_num);
  service_->sendMessage(del);
}

void RobotAggregator::sendDeleteObstaclePart(int model_id, int part_id, long frame_num) {
  // LINFO << "RobotAggregator: Deleting a primitive id = "
  //       << part_id;
  VisionMessage del = VisionMessage(ObstacleMessage::DeletePartMessage(model_id, part_id), frame_num);
  service_->sendMessage(del);
}

void RobotAggregator::sendDeleteSurface(int id, long frame_num)
{
  // LINFO << "RobotAggregator: Deleting surface: " << id;
  VisionMessage msg = VisionMessage(SurfaceMessage::DeleteMessage(id), frame_num);
  service_->sendMessage(msg);
}

void RobotAggregator::sendModify(ObjectModel& model, int model_id, int part_id, long frame_num) {
  CoefsVisitor coefs;
  model.accept(coefs);
  VisionMessage msg = VisionMessage(ObstacleMessage::ModifyMessage(
      coefs.type_id(), model_id, part_id, coefs.radius(), coefs.coefs()),
      frame_num);
  LINFO << "RobotAggregator: Modifying existing primitive ["
            << "type = " << coefs.type_id()
            << "; id = " << model_id << " | " << part_id
            << "]";
  service_->sendMessage(msg);
}

void RobotAggregator::sendModify(SurfaceModel& surface, long frame_num)
{
  std::vector<float> vertices;
  std::vector<float> normal = {surface.get_planeCoefficients().values[0],surface.get_planeCoefficients().values[1],surface.get_planeCoefficients().values[2]};
  for(auto point : surface.get_hull()->points)
  {
    vertices.push_back(point.x);
    vertices.push_back(point.y);
    vertices.push_back(point.z);
  }

  VisionMessage msg = VisionMessage(SurfaceMessage::ModifyMessage(surface.id(), normal, vertices), frame_num);

  // LINFO << "RobotAggregator: Modifying existing surface ["
  //       << "id = " << surface.id()
  //      << "]";
  service_->sendMessage(msg);
}

void RobotAggregator::sendPointCloud(PointCloudConstPtr cloud, long frame_num)
{
  unsigned char* pt_data = (unsigned char*)&cloud->points[0];
  VisionMessage msg = VisionMessage(PointCloudMessage(pt_data, cloud->points.size(), sizeof(PointT)), frame_num);

  service_->sendMessage(msg);
}

void RobotAggregator::sendRGBImage(cv::Mat const& image, long frame_num)
{
  const unsigned char* img_data = nullptr;
  if (CV_8UC3 == image.type()) {
    img_data = image.data;
  } else {
    std::ostringstream ss;
    ss << "Unexpected image format: " << image.type();
    throw std::runtime_error(ss.str());
  }

  VisionMessage msg = VisionMessage(RGBMessage(img_data, image.cols, image.rows), frame_num);
  service_->sendMessage(msg);
}
