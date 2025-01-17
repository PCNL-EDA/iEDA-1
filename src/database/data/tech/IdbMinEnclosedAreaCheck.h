#ifndef IDB_MIN_ENCLOSED_AREA_CHECK_H
#define IDB_MIN_ENCLOSED_AREA_CHECK_H
#include <vector>

namespace idb {
  class IdbMinEnclosedAreaCheck {
   public:
    IdbMinEnclosedAreaCheck() : _area(-1), _width(-1) { }
    explicit IdbMinEnclosedAreaCheck(int area, int width = -1) : _area(area), _width(width) { }
    ~IdbMinEnclosedAreaCheck() { }

    // getter
    int get_area() { return _area; }
    int get_width() { return _width; }
    // setter
    void set_area(int area) { _area = area; }
    void set_width(int width) { _width = width; }

   private:
    int _area;
    int _width;
  };

  class IdbMinEnclosedAreaCheckList {
   public:
    IdbMinEnclosedAreaCheckList() { }
    ~IdbMinEnclosedAreaCheckList() { }

    void addMinEnclosedAreaCheck(std::unique_ptr<IdbMinEnclosedAreaCheck> &check) {
      _min_enclosed_area_checks.push_back(std::move(check));
    }
    IdbMinEnclosedAreaCheck *getFirstEnclosedAreaCheck() { return (_min_enclosed_area_checks.begin())->get(); }

   private:
    std::vector<std::unique_ptr<IdbMinEnclosedAreaCheck>> _min_enclosed_area_checks;
  };

}  // namespace idb

#endif
