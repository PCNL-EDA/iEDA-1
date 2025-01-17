#pragma once

#include "DbInterface.h"
#include "flute3/flute.h"

#include "ids.hpp"

#include "Point.h"

namespace ito {
using std::multimap;
using std::map;

using namespace ista;
using XCord = int; // x-axis coordinates
using YCord = int; // y-axis coordinates

class RoutingTree;
class Node;

enum class RoutingType : int { kHVTree = 0, kSteiner = 1 };

RoutingTree *makeRoutingTree(Net *net, ista::TimingDBAdapter *db_adapter,
                             RoutingType rout_type);

void getConnectedPins(Net *net,
                      // Return value.
                      DesignObjSeq &pins);

void printTree(Node *root);

void findMaxDistfromDrvr(Node *root, int curr_dist, int &max_dist);

void drvrToLoadLength();

class Node {
 public:
  Node() = default;
  Node(Point pt) : _pt(pt) {}
  ~Node() = default;

  void set_father_node(Node *f_n) { _father_node = f_n; }
  void set_first_child(Node *f_c) { _first_child = f_c; }
  void set_next_sibling(Node *n_s) { _next_sibling = n_s; }
  void set_neighbor(int n) { _neighbor = n; }
  void set_id(int id) { _id = id; }

  Node *get_father_node() const { return _father_node; }
  Node *get_first_child() const { return _first_child; }
  Node *get_next_sibling() const { return _next_sibling; }
  int   get_neighbor() const { return _neighbor; }
  int   get_id() const { return _id; }
  Point get_location() { return _pt; }

  bool operator==(const Node n2) { return this->_pt == n2._pt; }

 private:
  int   _neighbor = -1;
  int   _id = -1;
  Point _pt;
  Node *_father_node = nullptr;
  Node *_first_child = nullptr;
  Node *_next_sibling = nullptr;
};

class RoutingTree {
 public:
  RoutingTree(DbInterface *dbinterface) {}
  RoutingTree() = default;
  ~RoutingTree() = default;

  // set
  void set_root(Node *root) { this->_root = root; }
  void set_points(vector<Point> pts) { this->_points = pts; }

  // get
  DesignObjSeq &get_pins() { return _pins; }
  Node         *get_root() const { return _root; }
  vector<Point> get_points() const { return _points; }
  Point         get_location(int idx) const { return _points[idx]; }
  unsigned int  get_pins_count() { return _pins.size(); }

  DesignObject *get_pin(int idx) const;

  void makeHVTree(int x[], int y[]);

  void makeSteinerTree(int x[], int y[]);
  void findLeftRight(Node *father, int father_id, int child_id, vector<vector<int>> &adj,
                     Flute::Tree stTree);

  void segmentIndexAndLength(Node *root,
                             // return values
                             vector<pair<int, int>> &wire_segment_idx,
                             vector<int>            &length);
  void drvrToLoadLength(Node *root, vector<int> &load_index, vector<int> &length,
                        int curr_dist);
  void updateBranch();
  // require updateBranch
  int left(int idx) const { return _left_branch[idx]; }
  int middle(int idx) const { return _middle_branch[idx]; }
  int right(int idx) const { return _right_branch[idx]; }

  vector<vector<int>> findPathOverMaxLength(Node *root, int sum, int max_length,
                                            // return values
                                            vector<vector<int>> &res);

  void set_pin_visit(int idx) { _is_visit_pin[idx] = 1; }
  int  get_pin_visit(int idx) { return _is_visit_pin[idx]; }

  void plotConectionInfo(const std::string file_name, const std::string net_name,
                         RoutingTree *tree, Rectangle core);

  //   void clusterTreeToRoutingTree(cluster_tree::ClusterTree* Ctree);
  //   void findLeftRight(Node *father,
  //   cluster_tree::BinaryTreeNode<cluster_tree::Point<int> *>* BTNode);

  static int _null_pt;

 private:
  void insertTree(Node *father, Node *child);
  void insertTree(Node *father, Node *child, int id);

  void updateTree(int index, XCord x_coordinate, Node *&parent,
                  std::vector<int>       cordinate_vec,
                  std::multimap<YCord, XCord> cordinate_to_x_map);

  int findIndexInPoints(Node *n);
  int findIndexInPoints(Node *n, int id);

  void preOrderTraversal(Node *root);

  void deepFirstSearch(Node *root, int sum, vector<vector<int>> &res, vector<int> vec,
                       int max_length);

  void plotPinName(std::stringstream &feed, const std::string net_name, RoutingTree *tree,
                   int id);
  // _root always cooresponds to the driver pin.
  Node        *_root = nullptr;
  DesignObjSeq _pins;

  // pin pts + steiner pts, the index of steiner pts is behind of the pin pts.
  std::vector<Point> _points;

  // Since each node has at most three branches, due to the way of RoutingTree is
  // constructed. _left_branch[i] = j means: node j is the left child of node i.
  vector<int> _left_branch;
  vector<int> _middle_branch;
  vector<int> _right_branch;

  vector<int> _is_visit_pin;
};
} // namespace ito