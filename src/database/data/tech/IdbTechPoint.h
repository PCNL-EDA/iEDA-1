#ifndef IDB_TECH_POINT_H
#define IDB_TECH_POINT_H

namespace idb {
  class IdbTechPoint {
   public:
    IdbTechPoint() : _coordinate_x(0), _coordinate_y(0) { }
    IdbTechPoint(int x, int y) : _coordinate_x(x), _coordinate_y(y) { }
    IdbTechPoint(const IdbTechPoint &other) {
      _coordinate_x = other._coordinate_x;
      _coordinate_y = other._coordinate_y;
    }
    IdbTechPoint(const IdbTechPoint &&other) {
      _coordinate_x = other._coordinate_x;
      _coordinate_y = other._coordinate_y;
    }
    ~IdbTechPoint() { }
    // getter
    int get_coordinate_x() const { return _coordinate_x; }
    int get_coordinate_y() const { return _coordinate_y; }
    // setter
    void set_coordinate_x(int x) { _coordinate_x = x; }
    void set_coordinate_y(int y) { _coordinate_y = y; }
    void setCoordinate(int x, int y) {
      _coordinate_x = x;
      _coordinate_y = y;
    }
    void setCoordinate(const IdbTechPoint &other) {
      _coordinate_x = other._coordinate_x;
      _coordinate_y = other._coordinate_y;
    }
    // others
    IdbTechPoint &operator=(const IdbTechPoint &other) {
      _coordinate_x = other._coordinate_x;
      _coordinate_y = other._coordinate_y;
      return *this;
    }
    IdbTechPoint &operator=(IdbTechPoint &&other) {
      _coordinate_x = other._coordinate_x;
      _coordinate_y = other._coordinate_y;
      return *this;
    }
    bool operator<(const IdbTechPoint &other) {
      return (_coordinate_x == other._coordinate_x) ? (_coordinate_y < other._coordinate_y)
                                                    : (_coordinate_x < other._coordinate_x);
    }
    bool operator==(const IdbTechPoint &other) {
      return (_coordinate_x == other._coordinate_x) && (_coordinate_y == other._coordinate_y);
    }
    bool operator!=(const IdbTechPoint &other) { return !(*this == other); }

   private:
    int _coordinate_x;
    int _coordinate_y;
  };
}  // namespace idb

#endif