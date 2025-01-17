/*
 * @Author: S.J Chen
 * @Date: 2022-01-21 11:52:14
 * @LastEditTime: 2023-02-09 11:53:38
 * @LastEditors: Shijian Chen  chenshj@pcl.ac.cn
 * @Description:
 * @FilePath: /irefactor/src/operation/iPL/source/data/Row.hh
 * Contact : https://github.com/sjchanson
 */

#ifndef IPL_ROW_H
#define IPL_ROW_H

#include <string>
#include <vector>

#include "Orient.hh"
#include "Rectangle.hh"

namespace ipl {

class Site
{
 public:
  Site() = delete;
  explicit Site(std::string name);
  Site(const Site&) = delete;
  Site(Site&&)      = delete;
  ~Site()           = default;

  Site& operator=(const Site&) = delete;
  Site& operator=(Site&&) = delete;

  // getter.
  std::string get_name() const { return _name; }

  Orient  get_orient() const { return _orient; }
  int32_t get_site_width() const { return _site_width; }
  int32_t get_site_height() const { return _site_height; }

  // setter.
  void set_orient(Orient orient) { _orient = orient; }
  void set_width(int32_t width) { _site_width = width; }
  void set_height(int32_t height) { _site_height = height; }

 private:
  std::string _name;
  Orient      _orient;
  int32_t     _site_width;
  int32_t     _site_height;
};
inline Site::Site(std::string name) : _name(std::move(name)), _site_width(0), _site_height(0)
{
}

class Row
{
 public:
  Row() = delete;
  explicit Row(std::string name);
  Row(const Row&) = delete;
  Row(Row&&)      = delete;
  ~Row();

  Row& operator=(const Row&) = delete;
  Row& operator=(Row&&) = delete;

  // getter.
   std::string get_name() const { return _name; }
   Orient      get_orient() const { return _site->get_orient(); }

   Rectangle<int32_t> get_shape() const { return _shape; }
   Point<int32_t>     get_coordi() const { return _shape.get_lower_left(); }

   Site* get_site() const { return _site;}
   int32_t get_site_num() const { return _site_num; }
   int32_t get_site_width() const { return _site->get_site_width(); }
   int32_t get_site_height() const { return _site->get_site_height(); }

  // setter.
  void set_shape(Rectangle<int32_t> shape) { _shape = std::move(shape); }
  void set_site(Site* site) { _site = site; }
  void set_site_num(int32_t site_num) { _site_num = site_num; }

 private:
  std::string        _name;
  Rectangle<int32_t> _shape;
  Site*              _site;
  int32_t            _site_num;
};
inline Row::Row(std::string name) : _name(std::move(name)), _site(nullptr)
{
}

inline Row::~Row()
{
  delete _site;
}

}  // namespace ipl

#endif