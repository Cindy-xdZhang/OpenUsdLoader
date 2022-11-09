// SPDX-License-Identifier: MIT
#pragma once

#ifdef _MSC_VER
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

//
#include "value-types.hh"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#endif

#include "nonstd/expected.hpp"
#include "nonstd/optional.hpp"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "primvar.hh"
#include "tiny-variant.hh"
//
#include "value-eval-util.hh"

namespace tinyusdz {

// SpecType enum must be same order with pxrUSD's SdfSpecType(since enum value
// is stored in Crate directly)
enum class SpecType {
  Unknown = 0,  // must be 0
  Attribute,
  Connection,
  Expression,
  Mapper,
  MapperArg,
  Prim,
  PseudoRoot,
  Relationship,
  RelationshipTarget,
  Variant,
  VariantSet,
  Invalid,  // or NumSpecTypes
};

enum class Orientation {
  RightHanded,  // 0
  LeftHanded,
  Invalid
};

enum class Visibility {
  Inherited,  // "inherited" (default)
  Invisible,  // "invisible"
  Invalid
};

enum class Purpose {
  Default,  // 0
  Render,   // "render"
  Proxy,    // "proxy"
  Guide,    // "guide"
};

//
// USDZ extension: sceneLibrary
// https://developer.apple.com/documentation/arkit/usdz_schemas_for_ar/scenelibrary
//
enum class Kind {
  Model,
  Group,
  Assembly,
  Component,
  Subcomponent,
  SceneLibrary, // USDZ extension
  Invalid
};

// Attribute interpolation
enum class Interpolation {
  Constant,     // "constant"
  Uniform,      // "uniform"
  Varying,      // "varying"
  Vertex,       // "vertex"
  FaceVarying,  // "faceVarying"
  Invalid
};

// NOTE: Attribute cannot have ListEdit qualifier
enum class ListEditQual {
  ResetToExplicit,  // "unqualified"(no qualifier)
  Append,           // "append"
  Add,              // "add"
  Delete,           // "delete"
  Prepend,          // "prepend"
  Order,            // "order"
  Invalid
};

enum class Axis { X, Y, Z, Invalid };

// For PrimSpec
enum class Specifier {
  Def,  // 0
  Over,
  Class,
  Invalid
};

enum class Permission {
  Public,  // 0
  Private,
  Invalid
};

enum class Variability {
  Varying,  // 0
  Uniform,
  Config,
  Invalid
};

// single or triple-quoted('"""' or ''') string
struct StringData {
  std::string value;
  bool is_triple_quoted{false};
  bool single_quote{false};  // true for ', false for "

  // optional(for USDA)
  int line_row{0};
  int line_col{0};
};

///
/// Simlar to SdfPath.
/// NOTE: We are doging refactoring of Path class, so the following comment may
/// not be correct.
///
/// We don't need the performance for USDZ, so use naiive implementation
/// to represent Path.
/// Path is something like Unix path, delimited by `/`, ':' and '.'
/// Square brackets('<', '>' is not included)
///
/// Root path is represented as prim path "/" and elementPath ""(empty).
///
/// Example:
///
/// - `/muda/bora.dora` : prim_part is `/muda/bora`, prop_part is `.dora`.
/// - `bora` : Could be Element(leaf) path or Relative path
///
/// ':' is a namespce delimiter(example `input:muda`).
///
/// Limitations:
///
/// - Relational attribute path(`[` `]`. e.g. `/muda/bora[/ari].dora`) is not
/// supported.
/// - Variant chars('{' '}') is not supported(yet0.
/// - '../' is TODO
///
/// and have more limitatons.
///
class Path {
 public:
  // Similar to SdfPathNode
  enum class PathType {
    Prim,
    PrimProperty,
    RelationalAttribute,
    MapperArg,
    Target,
    Mapper,
    PrimVariantSelection,
    Expression,
    Root,
  };

  Path() : _valid(false) {}

  static Path make_root_path() {
    Path p = Path("/", "");
    // elementPath is empty for root.
    p._element = "";
    p._valid = true;
    return p;
  }

  // `p` is split into prim_part and prop_part
  // Path(const std::string &p);
  Path(const std::string &prim, const std::string &prop);

  // : prim_part(prim), valid(true) {}
  // Path(const std::string &prim, const std::string &prop)
  //    : prim_part(prim), prop_part(prop) {}

  Path(const Path &rhs) = default;

  Path &operator=(const Path &rhs) {
    this->_valid = rhs._valid;

    this->_prim_part = rhs._prim_part;
    this->_prop_part = rhs._prop_part;
    this->_element = rhs._element;

    return (*this);
  }

  std::string full_path_name() const {
    std::string s;
    if (!_valid) {
      s += "#INVALID#";
    }

    s += _prim_part;
    if (_prop_part.empty()) {
      return s;
    }

    s += "." + _prop_part;

    return s;
  }

  std::string prim_part() const { return _prim_part; }
  std::string prop_part() const { return _prop_part; }

  void set_path_type(const PathType ty) { _path_type = ty; }

  bool get_path_type(PathType &ty) {
    if (_path_type) {
      ty = _path_type.value();
    }
    return false;
  }

  // IsPropertyPath: PrimProperty or RelationalAttribute
  bool is_property_path() const {
    if (_path_type) {
      if ((_path_type.value() == PathType::PrimProperty ||
           (_path_type.value() == PathType::RelationalAttribute))) {
        return true;
      }
    }

    // TODO: RelationalAttribute
    if (_prim_part.empty()) {
      return false;
    }

    if (_prop_part.size()) {
      return true;
    }

    return false;
  }

  // Is Prim's property path?
  // True when both PrimPart and PropPart are not empty.
  bool is_prim_property_path() const {
    if (_prim_part.empty()) {
      return false;
    }
    if (_prop_part.size()) {
      return true;
    }
    return false;
  }

  bool is_valid() const { return _valid; }

  bool is_empty() { return (_prim_part.empty() && _prop_part.empty()); }

  // static Path RelativePath() { return Path("."); }

  Path append_property(const std::string &elem);

  Path append_element(const std::string &elem);

  std::string element_name() const { return _element; }

  ///
  /// Split a path to the root(common ancestor) and its siblings
  ///
  /// example:
  ///
  /// - / -> [/, Empty]
  /// - /bora -> [/bora, Empty]
  /// - /bora/dora -> [/bora, /dora]
  /// - /bora/dora/muda -> [/bora, /dora/muda]
  /// - bora -> [Empty, bora]
  /// - .muda -> [Empty, .muda]
  ///
  std::pair<Path, Path> split_at_root() const;

  ///
  /// Get parent Prim path
  ///
  /// example:
  ///
  /// - / -> invalid Path
  /// - /bora -> invalid Path(since `/` is not the Prim path)
  /// - /bora/dora -> /bora
  /// - dora/bora -> dora
  /// - dora -> invalid Path
  /// - .dora -> invalid Path(path is property path)
  Path get_parent_prim_path() const;

  ///
  /// @returns true if a path is '/' only
  ///
  bool is_root_path() const {
    if (!_valid) {
      return false;
    }

    if ((_prim_part.size() == 1) && (_prim_part[0] == '/')) {
      return true;
    }

    return false;
  }

  ///
  /// @returns true if a path is root prim: e.g. '/bora'
  ///
  bool is_root_prim() const {
    if (!_valid) {
      return false;
    }

    if (is_root_path()) {
      return false;
    }

    if ((_prim_part.size() > 1) && (_prim_part[0] == '/')) {
      // no other '/' except for the fist one
      if (_prim_part.find_last_of('/') == 0) {
        return true;
      }
    }

    return false;
  }

  bool is_absolute_path() const {
    if (_prim_part.size()) {
      if ((_prim_part.size() > 0) && (_prim_part[0] == '/')) {
        return true;
      }
    }

    return false;
  }

  bool is_relative_path() const {
    if (_prim_part.size()) {
      return !is_absolute_path();
    }

    return true;  // prop part only
  }

  // Strip '/'
  Path &make_relative() {
    if (is_absolute_path() && (_prim_part.size() > 1)) {
      // Remove first '/'
      _prim_part.erase(0, 1);
    }
    return *this;
  }

  const Path make_relative(Path &&rhs) {
    (*this) = std::move(rhs);

    return make_relative();
  }

  static const Path make_relative(const Path &rhs) {
    Path p = rhs;  // copy
    return p.make_relative();
  }

 private:
  std::string _prim_part;  // e.g. /Model/MyMesh, MySphere
  std::string _prop_part;  // e.g. .visibility
  std::string _element;    // Element name

  nonstd::optional<PathType> _path_type;  // Currently optional.

  bool _valid{false};
};

///
/// Split Path by the delimiter(e.g. "/") then create lists.
///
class TokenizedPath {
 public:
  TokenizedPath() {}

  TokenizedPath(const Path &path) {
    std::string s = path.prop_part();
    if (s.empty()) {
      // ???
      return;
    }

    if (s[0] != '/') {
      // Path must start with "/"
      return;
    }

    s.erase(0, 1);

    char delimiter = '/';
    size_t pos{0};
    while ((pos = s.find(delimiter)) != std::string::npos) {
      std::string token = s.substr(0, pos);
      _tokens.push_back(token);
      s.erase(0, pos + sizeof(char));
    }

    if (!s.empty()) {
      // leaf element
      _tokens.push_back(s);
    }
  }

 private:
  std::vector<std::string> _tokens;
};

bool operator==(const Path &lhs, const Path &rhs);

// variants in Prim Meta.
//
// e.g.
// variants = {
//   string variant0 = "bora"
//   string variant1 = "dora"
// }
// pxrUSD uses dict type for the content, but TinyUSDZ only accepts list of
// strings for now
//
using VariantSelectionMap = std::map<std::string, std::string>;

class MetaVariable;

using CustomDataType = std::map<std::string, MetaVariable>;

// Variable class for Prim and Attribute Metadataum.
//
// - Accepts limited number of types for value
// - No 'custom' keyword
// - 'None'(Value Block) is supported for some type(at least `references` and `payload` accepts None)
// - No TimeSamples, No Connection, No Relationship(`rel`)
// - Value must be assigned(e.g. "float myval = 1.3"). So no definition only syntax("float myval")
// - Can be string only(no type information) 
//   - Its variable name is interpreted as "comment"
// 
class MetaVariable {
 public:

  MetaVariable &operator=(const MetaVariable &rhs) {
    _name = rhs._name;
    _value = rhs._value;

    return *this;
  }

  MetaVariable(const MetaVariable &rhs) {
    _name = rhs._name;
    _value = rhs._value;
  }

  // template <typename T>
  // bool is() const {
  //   return value.index() == ValueType::index_of<T>();
  // }

  bool is_valid() const {
    return _value.type_id() != value::TypeTraits<std::nullptr_t>::type_id;
  }

  //// TODO
  //bool is_timesamples() const { return false; }

  MetaVariable() = default;

  //
  // custom data must have some value, so no set_type()
  // OK "float myval = 1"
  // NG "float myval"
  //
  template <typename T>
  void set_value(const T &v) {
    // TODO: Check T is supported type for Metadatum.
    _value = v;

    _name = std::string(); // empty
  }

  template <typename T>
  void set_value(const std::string &name, const T &v) {
    // TODO: Check T is supported type for Metadatum.
    _value = v;

    _name = name;
  }

  template <typename T>
  bool get_value(T *dst) const {
    if (!dst) {
      return false;
    }

    if (const T *v = _value.as<T>()) {
      (*dst) = *v;
      return true;
    }

    return false;
  }

  template <typename T>
  nonstd::optional<T> get_value() const {
    if (const T *v = _value.as<T>()) {
      return *v;
    }

    return nonstd::nullopt;
  }

  void set_name(const std::string &name) { _name = name; }
  const std::string &get_name() const { return _name; }

  const value::Value &get_raw_value() const { return _value; }

  // No set_type_name()
  const std::string type_name() const { return TypeName(*this); }

  uint32_t type_id() const { return TypeId(*this); }

  bool is_blocked() const {
    return type_id() == value::TypeId::TYPE_ID_VALUEBLOCK;
  }

 private:
  static std::string TypeName(const MetaVariable &v) {
    return v._value.type_name();
  }

  static uint32_t TypeId(const MetaVariable &v) {
    return v._value.type_id();
  }

 private:
  value::Value _value{nullptr};
  std::string _name;
};


struct APISchemas {
  // TinyUSDZ does not allow user-supplied API schema for now
  enum class APIName {
    MaterialBindingAPI,  // "MaterialBindingAPI"
    SkelBindingAPI,      // "SkelBindingAPI"
    // USDZ AR extensions
    Preliminary_AnchoringAPI,
    Preliminary_PhysicsColliderAPI,
    // Preliminary_Trigger,
    // Preliminary_PhysicsGravitationalForce,
    Preliminary_PhysicsMaterialAPI,
    Preliminary_PhysicsRigidBodyAPI,
    // Preliminary_InfiniteColliderPlane,
    // Preliminary_ReferenceImage,
    // Preliminary_Action,
    // Preliminary_Text,
  };

  ListEditQual listOpQual{ListEditQual::ResetToExplicit};  // must be 'prepend'

  // std::get<1>: instance name. For Multi-apply API Schema e.g.
  // `material:MainMaterial` for `CollectionAPI:material:MainMaterial`
  std::vector<std::pair<APIName, std::string>> names;
};

// SdfLayerOffset
struct LayerOffset {
  double _offset{0.0};
  double _scale{1.0};
};

// SdfReference
struct Reference {
  value::AssetPath asset_path;
  Path prim_path;
  LayerOffset layerOffset;
  CustomDataType customData;
};

// SdfPayload
struct Payload {
  value::AssetPath asset_path;  // std::string in SdfPayload
  Path _prim_path;
  LayerOffset _layer_offset;  // from 0.8.0
  // No customData for Payload
};

// Metadata for Prim
struct PrimMeta {
  nonstd::optional<bool> active;                // 'active'
  nonstd::optional<bool> hidden;                // 'hidden'
  nonstd::optional<Kind> kind;                  // 'kind'
  nonstd::optional<CustomDataType> assetInfo;   // 'assetInfo'
  nonstd::optional<CustomDataType> customData;  // `customData`
  nonstd::optional<StringData> doc;             // 'documentation'
  nonstd::optional<StringData> comment;         // 'comment'
  nonstd::optional<APISchemas> apiSchemas;      // 'apiSchemas'

  //
  // Compositions
  //
  nonstd::optional<std::pair<ListEditQual, std::vector<Reference>>> references;
  nonstd::optional<std::pair<ListEditQual, std::vector<Payload>>> payload;
  nonstd::optional<std::pair<ListEditQual, std::vector<Path>>>
      inherits;  // 'inherits'
  nonstd::optional<std::pair<ListEditQual, std::vector<std::string>>>
      variantSets;  // 'variantSets'. Could be `token` but treat as
                    // `string`(Crate format uses `string`)

  nonstd::optional<VariantSelectionMap> variants;  // `variants`

  nonstd::optional<std::pair<ListEditQual, std::vector<Path>>>
      specializes;  // 'specializes'

  // USDZ extensions
  nonstd::optional<std::string> sceneName;  // 'sceneName'

  // Omniverse extensions(TODO: Use UTF8 string type?)
  // https://github.com/PixarAnimationStudios/USD/pull/2055
  nonstd::optional<std::string> displayName;  // 'displayName'

  std::map<std::string, MetaVariable> meta;  // other meta values

  // String only metadataum.
  // TODO: Represent as `MetaVariable`?
  std::vector<StringData> stringData;

  // FIXME: Find a better way to detect Prim meta is authored...
  bool authored() const {
    return (active || hidden || kind || customData || references || payload ||
            inherits || variants || variantSets || specializes || displayName ||
            sceneName || doc || comment || meta.size() || apiSchemas ||
            stringData.size() || assetInfo);
  }

  //
  // Crate only. Only used internally&debugging.
  //

  nonstd::optional<std::pair<ListEditQual, std::vector<Path>>> inheritPaths;
  nonstd::optional<std::vector<value::token>> primChildren;
  nonstd::optional<std::vector<value::token>> variantChildren;
  nonstd::optional<std::vector<value::token>> variantSetChildren;
};

// Metadata for Attribute
struct AttrMeta {
  // frequently used items
  // nullopt = not specified in USD data
  nonstd::optional<Interpolation> interpolation;  // 'interpolation'
  nonstd::optional<uint32_t> elementSize;         // usdSkel 'elementSize'
  nonstd::optional<bool> hidden;                  // 'hidden'
  nonstd::optional<StringData> comment;           // `comment`
  nonstd::optional<CustomDataType> customData;    // `customData`

  std::map<std::string, MetaVariable> meta;  // other meta values

  // String only metadataum.
  // TODO: Represent as `MetaVariable`?
  std::vector<StringData> stringData;

  bool authored() const {
    return (interpolation || elementSize || hidden || customData ||
            meta.size() || stringData.size());
  }
};

// Typed TimeSamples value
//
// double radius.timeSamples = { 0: 1.0, 1: None, 2: 3.0 }
//
// in .usd, are represented as
//
// 0: (1.0, false)
// 1: (2.0, true)
// 2: (3.0, false)
//

template <typename T>
struct TypedTimeSamples {
 public:
  struct Sample {
    double t;
    T value;
    bool blocked{false};
  };

  bool empty() const { return _samples.empty(); }

  void update() const {
    std::sort(_samples.begin(), _samples.end(),
              [](const Sample &a, const Sample &b) { return a.t < b.t; });

    _dirty = false;

    return;
  }

  // Get value at specified time.
  // Return linearly interpolated value when TimeSampleInterpolationType is
  // Linear. Returns nullopt when specified time is out-of-range.
  bool get(T *dst, double t = value::TimeCode::Default(),
           value::TimeSampleInterpolationType interp =
               value::TimeSampleInterpolationType::Held) const {
    if (!dst) {
      return false;
    }

    if (empty()) {
      return false;
    }

    if (_dirty) {
      update();
    }

    if (value::TimeCode(t).is_default()) {
      // FIXME: Use the first item for now.
      // TODO: Handle bloked
      (*dst) = _samples[0].value;
      return true;
    } else {
      auto it = std::lower_bound(
          _samples.begin(), _samples.end(), t,
          [](const Sample &a, double tval) { return a.t < tval; });

      if (interp == value::TimeSampleInterpolationType::Linear) {
        size_t idx0 = size_t(std::max(
            int64_t(0),
            std::min(int64_t(_samples.size() - 1),
                     int64_t(std::distance(_samples.begin(), it - 1)))));
        size_t idx1 =
            size_t(std::max(int64_t(0), std::min(int64_t(_samples.size() - 1),
                                                 int64_t(idx0) + 1)));

        double tl = _samples[idx0].t;
        double tu = _samples[idx1].t;

        double dt = (t - tl);
        if (std::fabs(tu - tl) < std::numeric_limits<double>::epsilon()) {
          // slope is zero.
          dt = 0.0;
        } else {
          dt /= (tu - tl);
        }

        // Just in case.
        dt = std::max(0.0, std::min(1.0, dt));

        const value::Value &pv0 = _samples[idx0].value;
        const value::Value &pv1 = _samples[idx1].value;

        if (pv0.type_id() != pv1.type_id()) {
          // Type mismatch.
          return false;
        }

        // To concrete type
        const T *p0 = pv0.as<T>();
        const T *p1 = pv1.as<T>();

        if (!p0 || !p1) {
          return false;
        }

        const T p = lerp(*p0, *p1, dt);

        (*dst) = std::move(p);
        return true;
      } else {
        if (it == _samples.end()) {
          // ???
          return false;
        }

        (*dst) = it->value;
        return true;
      }
    }

    return false;
  }

  void add_sample(const Sample &s) {
    _samples.push_back(s);
    _dirty = true;
  }

  void add_sample(const double t, const T &v) {
    Sample s;
    s.t = t;
    s.value = v;
    _samples.emplace_back(s);
    _dirty = true;
  }

  void add_blocked_sample(const double t) {
    Sample s;
    s.t = t;
    s.blocked = true;
    _samples.emplace_back(s);
    _dirty = true;
  }

  const std::vector<Sample> &get_samples() const {
    if (_dirty) {
      update();
    }

    return _samples;
  }

  std::vector<Sample> &samples() {
    if (_dirty) {
      update();
    }

    return _samples;
  }

 private:
  // Need to be sorted when looking up the value.
  mutable std::vector<Sample> _samples;
  mutable bool _dirty{false};
};

//
// Scalar or TimeSamples.
//
template <typename T>
struct Animatable {
 public:
  bool is_blocked() const { return _blocked; }

  bool is_timesamples() const {
    if (is_blocked()) {
      return false;
    }
    return !_ts.empty();
  }

  bool is_scalar() const {
    if (is_blocked()) {
      return false;
    }
    return _ts.empty();
  }

  ///
  /// Get value at specific time.
  ///
  bool get(double t, T *v,
           const value::TimeSampleInterpolationType tinerp =
               value::TimeSampleInterpolationType::Held) const {
    if (!v) {
      return false;
    }

    if (is_blocked()) {
      return false;
    } else if (is_scalar()) {
      (*v) = _value;
      return true;
    } else {  // timesamples
      return _ts.get(v, t, tinerp);
    }
  }

  ///
  /// Get scalar value.
  ///
  bool get_scalar(T *v) const {
    if (!v) {
      return false;
    }

    if (is_blocked()) {
      return false;
    } else if (is_scalar()) {
      (*v) = _value;
      return true;
    } else {  // timesamples
      return false;
    }
  }

  // TimeSamples
  // void set(double t, const T &v);

  void add_sample(const double t, const T &v) { _ts.add_sample(t, v); }

  // Add None(ValueBlock) sample to timesamples
  void add_blocked_sample(const double t) { _ts.add_blocked_sample(t); }

  // Scalar
  void set(const T &v) {
    _value = v;
    _blocked = false;
  }

  const TypedTimeSamples<T> &get_timesamples() const { return _ts; }

  Animatable() {}

  Animatable(const T &v) : _value(v) {}

  // TODO: Init with timesamples

 private:
  // scalar
  T _value;
  bool _blocked{false};

  // timesamples
  TypedTimeSamples<T> _ts;
};

///
/// Tyeped Attribute without fallback(default) value.
/// For attribute with `uniform` qualifier or TimeSamples, or have
/// `.connect`(Connection)
///
/// - `authored() = true` : Attribute value is authored(attribute is
/// described in USDA/USDC)
/// - `authored() = false` : Attribute value is not authored(not described
/// in USD). If you call `get()`, fallback value is returned.
///
template <typename T>
class TypedAttribute {
 public:
  void set_value(const T &v) { _attrib = v; }

  const nonstd::optional<T> get_value() const {
    if (_attrib) {
      return _attrib.value();
    }
    return nonstd::nullopt;
  }

  bool get_value(T *dst) const {
    if (!dst) return false;

    if (_attrib) {
      (*dst) = _attrib.value();
      return true;
    }
    return false;
  }

  bool is_blocked() const { return _blocked; }

  // for `uniform` attribute only
  void set_blocked(bool onoff) { _blocked = onoff; }

  bool is_connection() const { return _paths.size(); }

  void set_connection(const Path &path) {
    _paths.clear();
    _paths.push_back(path);
  }

  void set_connections(const std::vector<Path> &paths) { _paths = paths; }

  const std::vector<Path> &get_connections() const { return _paths; }

  const nonstd::optional<Path> get_connection() const {
    if (_paths.size()) {
      return _paths[0];
    }

    return nonstd::nullopt;
  }

  void set_value_empty() { _empty = true; }

  bool is_value_empty() const { return _empty; }

  // value set?
  bool authored() const {
    if (_empty) {
      return true;
    }

    if (_attrib) {
      return true;
    }
    if (_paths.size()) {
      return true;
    }
    return false;
  }

  const AttrMeta &metas() const { return _metas; }
  AttrMeta &metas() { return _metas; }

 private:
  AttrMeta _metas;
  bool _empty{false};
  std::vector<Path> _paths;
  nonstd::optional<T> _attrib;
  bool _blocked{false};  // for `uniform` attribute.
};

///
/// Tyeped Terminal(Output) Attribute(No value assign, no fallback(default)
/// value, no connection)
///
/// - `authored() = true` : Attribute value is authored(attribute is
/// described in USDA/USDC)
/// - `authored() = false` : Attribute value is not authored(not described
/// in USD).
///
template <typename T>
class TypedTerminalAttribute {
 public:
  void set_authored(bool onoff) { _authored = onoff; }

  // value set?
  bool authored() const { return _authored; }

  std::string type_name() const { return value::TypeTraits<T>::type_name(); }

  uint32_t type_id() const { return value::TypeTraits<T>::type_id; }

  const AttrMeta &metas() const { return _metas; }
  AttrMeta &metas() { return _metas; }

 private:
  AttrMeta _metas;
  bool _authored{false};
};

template <typename T>
class TypedAttributeWithFallback;

///
/// Attribute with fallback(default) value.
/// For attribute with `uniform` qualifier or TimeSamples, but don't have
/// `.connect`(Connection)
///
/// - `authored() = true` : Attribute value is authored(attribute is
/// described in USDA/USDC)
/// - `authored() = false` : Attribute value is not authored(not described
/// in USD). If you call `get()`, fallback value is returned.
///
template <typename T>
class TypedAttributeWithFallback {
 public:
  TypedAttributeWithFallback() = delete;

  ///
  /// Init with fallback value;
  ///
  TypedAttributeWithFallback(const T &fallback) : _fallback(fallback) {}

  TypedAttributeWithFallback &operator=(const T &value) {
    _attrib = value;

    // fallback Value should be already set with `AttribWithFallback(const T&
    // fallback)` constructor.

    return (*this);
  }

  //
  // FIXME: Defininig copy constructor, move constructor and  move assignment
  // operator Gives compilation error :-(. so do not define it.
  //

  // AttribWithFallback(const AttribWithFallback &rhs) {
  //   attrib = rhs.attrib;
  //   fallback = rhs.fallback;
  // }

  // AttribWithFallback &operator=(T&& value) noexcept {
  //   if (this != &value) {
  //       attrib = std::move(value.attrib);
  //       fallback = std::move(value.fallback);
  //   }
  //   return (*this);
  // }

  // AttribWithFallback(AttribWithFallback &&rhs) noexcept {
  //   if (this != &rhs) {
  //       attrib = std::move(rhs.attrib);
  //       fallback = std::move(rhs.fallback);
  //   }
  // }

  void set_value(const T &v) { _attrib = v; }

  void set_value_empty() { _empty = true; }

  bool is_value_empty() const { return _empty; }

  const T &get_value() const {
    if (_attrib) {
      return _attrib.value();
    }
    return _fallback;
  }

  bool is_blocked() const { return _blocked; }

  // for `uniform` attribute only
  void set_blocked(bool onoff) { _blocked = onoff; }

  bool is_connection() const { return _paths.size(); }

  void set_connection(const Path &path) {
    _paths.clear();
    _paths.push_back(path);
  }

  void set_connections(const std::vector<Path> &paths) { _paths = paths; }

  const std::vector<Path> &get_connections() const { return _paths; }

  const nonstd::optional<Path> get_connection() const {
    if (_paths.size()) {
      return _paths[0];
    }

    return nonstd::nullopt;
  }

  // value set?
  bool authored() const {
    if (_empty) {  // authored with empty value.
      return true;
    }
    if (_attrib) {
      return true;
    }
    if (_paths.size()) {
      return true;
    }
    if (_blocked) {
      return true;
    }
    return false;
  }

  const AttrMeta &metas() const { return _metas; }
  AttrMeta &metas() { return _metas; }

 private:
  AttrMeta _metas;
  std::vector<Path> _paths;
  nonstd::optional<T> _attrib;
  bool _empty{false};
  T _fallback;
  bool _blocked{false};  // for `uniform` attribute.
};

template <typename T>
using TypedAnimatableAttributeWithFallback =
    TypedAttributeWithFallback<Animatable<T>>;

///
/// Similar to pxrUSD's PrimIndex
///
class PrimNode;

#if 0  // TODO
class PrimRange
{
 public:
  class iterator;

  iterator begin() const {
  }
  iterator end() const {
  }

 private:
  const PrimNode *begin_;
  const PrimNode *end_;
  size_t depth_{0};
};
#endif

template <typename T>
class ListOp {
 public:
  ListOp() : is_explicit(false) {}

  void ClearAndMakeExplicit() {
    explicit_items.clear();
    added_items.clear();
    prepended_items.clear();
    appended_items.clear();
    deleted_items.clear();
    ordered_items.clear();

    is_explicit = true;
  }

  bool IsExplicit() const { return is_explicit; }
  bool HasExplicitItems() const { return explicit_items.size(); }

  bool HasAddedItems() const { return added_items.size(); }

  bool HasPrependedItems() const { return prepended_items.size(); }

  bool HasAppendedItems() const { return appended_items.size(); }

  bool HasDeletedItems() const { return deleted_items.size(); }

  bool HasOrderedItems() const { return ordered_items.size(); }

  const std::vector<T> &GetExplicitItems() const { return explicit_items; }

  const std::vector<T> &GetAddedItems() const { return added_items; }

  const std::vector<T> &GetPrependedItems() const { return prepended_items; }

  const std::vector<T> &GetAppendedItems() const { return appended_items; }

  const std::vector<T> &GetDeletedItems() const { return deleted_items; }

  const std::vector<T> &GetOrderedItems() const { return ordered_items; }

  void SetExplicitItems(const std::vector<T> &v) { explicit_items = v; }

  void SetAddedItems(const std::vector<T> &v) { added_items = v; }

  void SetPrependedItems(const std::vector<T> &v) { prepended_items = v; }

  void SetAppendedItems(const std::vector<T> &v) { appended_items = v; }

  void SetDeletedItems(const std::vector<T> &v) { deleted_items = v; }

  void SetOrderedItems(const std::vector<T> &v) { ordered_items = v; }

 private:
  bool is_explicit{false};
  std::vector<T> explicit_items;
  std::vector<T> added_items;
  std::vector<T> prepended_items;
  std::vector<T> appended_items;
  std::vector<T> deleted_items;
  std::vector<T> ordered_items;
};

struct ListOpHeader {
  enum Bits {
    IsExplicitBit = 1 << 0,
    HasExplicitItemsBit = 1 << 1,
    HasAddedItemsBit = 1 << 2,
    HasDeletedItemsBit = 1 << 3,
    HasOrderedItemsBit = 1 << 4,
    HasPrependedItemsBit = 1 << 5,
    HasAppendedItemsBit = 1 << 6
  };

  ListOpHeader() : bits(0) {}

  explicit ListOpHeader(uint8_t b) : bits(b) {}

  explicit ListOpHeader(ListOpHeader const &op) : bits(0) {
    bits |= op.IsExplicit() ? IsExplicitBit : 0;
    bits |= op.HasExplicitItems() ? HasExplicitItemsBit : 0;
    bits |= op.HasAddedItems() ? HasAddedItemsBit : 0;
    bits |= op.HasPrependedItems() ? HasPrependedItemsBit : 0;
    bits |= op.HasAppendedItems() ? HasAppendedItemsBit : 0;
    bits |= op.HasDeletedItems() ? HasDeletedItemsBit : 0;
    bits |= op.HasOrderedItems() ? HasOrderedItemsBit : 0;
  }

  bool IsExplicit() const { return bits & IsExplicitBit; }

  bool HasExplicitItems() const { return bits & HasExplicitItemsBit; }
  bool HasAddedItems() const { return bits & HasAddedItemsBit; }
  bool HasPrependedItems() const { return bits & HasPrependedItemsBit; }
  bool HasAppendedItems() const { return bits & HasAppendedItemsBit; }
  bool HasDeletedItems() const { return bits & HasDeletedItemsBit; }
  bool HasOrderedItems() const { return bits & HasOrderedItemsBit; }

  uint8_t bits;
};

//
// Colum-major order(e.g. employed in OpenGL).
// For example, 12th([3][0]), 13th([3][1]), 14th([3][2]) element corresponds to
// the translation.
//
// template <typename T, size_t N>
// struct Matrix {
//  T m[N][N];
//  constexpr static uint32_t n = N;
//};

inline void Identity(value::matrix2d *mat) {
  memset(mat->m, 0, sizeof(value::matrix2d));
  for (size_t i = 0; i < 2; i++) {
    mat->m[i][i] = static_cast<double>(1);
  }
}

inline void Identity(value::matrix3d *mat) {
  memset(mat->m, 0, sizeof(value::matrix3d));
  for (size_t i = 0; i < 3; i++) {
    mat->m[i][i] = static_cast<double>(1);
  }
}

inline void Identity(value::matrix4d *mat) {
  memset(mat->m, 0, sizeof(value::matrix4d));
  for (size_t i = 0; i < 4; i++) {
    mat->m[i][i] = static_cast<double>(1);
  }
}

// ret = m x n
template <typename MTy, typename STy, size_t N>
MTy Mult(const MTy &m, const MTy &n) {
  MTy ret;
  memset(ret.m, 0, sizeof(MTy));

  for (size_t j = 0; j < N; j++) {
    for (size_t i = 0; i < N; i++) {
      STy value = static_cast<STy>(0);
      for (size_t k = 0; k < N; k++) {
        value += m.m[k][i] * n.m[j][k];
      }
      ret.m[j][i] = value;
    }
  }

  return ret;
}

struct Extent {
  value::float3 lower{{std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::infinity(),
                       std::numeric_limits<float>::infinity()}};

  value::float3 upper{{-std::numeric_limits<float>::infinity(),
                       -std::numeric_limits<float>::infinity(),
                       -std::numeric_limits<float>::infinity()}};

  Extent() = default;

  Extent(const value::float3 &l, const value::float3 &u) : lower(l), upper(u) {}

  bool is_valid() const {
    if (lower[0] > upper[0]) return false;
    if (lower[1] > upper[1]) return false;
    if (lower[2] > upper[2]) return false;

    return std::isfinite(lower[0]) && std::isfinite(lower[1]) &&
           std::isfinite(lower[2]) && std::isfinite(upper[0]) &&
           std::isfinite(upper[1]) && std::isfinite(upper[2]);
  }

  std::array<std::array<float, 3>, 2> to_array() const {
    std::array<std::array<float, 3>, 2> ret;
    ret[0][0] = lower[0];
    ret[0][1] = lower[1];
    ret[0][2] = lower[2];
    ret[1][0] = upper[0];
    ret[1][1] = upper[1];
    ret[1][2] = upper[2];

    return ret;
  }
};

#if 0
struct ConnectionPath {
  bool is_input{false};  // true: Input connection. false: Output connection.

  Path path;  // original Path information in USD

  std::string token;  // token(or string) in USD
  int64_t index{-1};  // corresponding array index(e.g. the array index to
                      // `Scene.shaders`)
};

// struct Connection {
//   int64_t src_index{-1};
//   int64_t dest_index{-1};
// };
//
// using connection_id_map =
//     std::unordered_map<std::pair<std::string, std::string>, Connection>;
#endif

//
// Relationship(typeless property)
//
class Relationship {
 public:
  // For some reaon, using tinyusdz::variant will cause double-free in some
  // environemt on clang, so use old-fashioned way for a while.
  enum class Type { Empty, String, Path, PathVector };

  Type type{Type::Empty};
  std::string targetString;
  Path targetPath;
  std::vector<Path> targetPathVector;
  ListEditQual listOpQual{ListEditQual::ResetToExplicit};

  static Relationship make_empty() {
    Relationship r;
    r.set_empty();
    return r;
  }

  // TODO: Remove
  void set_listedit_qual(ListEditQual q) { listOpQual = q; }
  ListEditQual GetListEditQualifier() const { return listOpQual; }

  void set_empty() { type = Type::Empty; }

  void set(const std::string &s) {
    targetString = s;
    type = Type::String;
  }

  void set(const Path &p) {
    targetPath = p;
    type = Type::Path;
  }

  void set(const std::vector<Path> &pv) {
    targetPathVector = pv;
    type = Type::PathVector;
  }

  bool is_empty() const { return type == Type::Empty; }

  bool is_string() const { return type == Type::String; }

  bool is_path() const { return type == Type::Path; }

  bool is_pathvector() const { return type == Type::PathVector; }

  AttrMeta meta;
};

//
// Connection is a typed version of Relation
//
template <typename T>
class Connection {
 public:
  using type = typename value::TypeTraits<T>::value_type;

  static std::string type_name() { return value::TypeTraits<T>::type_name(); }

  // Connection() = delete;
  // Connection(const T &v) : fallback(v) {}

  nonstd::optional<Path> target;
};

// Interpolator for TimeSample data
enum class TimeSampleInterpolation {
  Nearest,  // nearest neighbor
  Linear,   // lerp
  // TODO: more to support...
};


// Attribute is a struct to hold generic attribute of a property(e.g. primvar)
// of Prim
// TODO: Refactor
struct Attribute {
  const std::string &name() const { return _name; }

  std::string &name() { return _name; }

  void set_name(const std::string &name) { _name = name; }

  void set_type_name(const std::string &tname) { _type_name = tname; }

  // `var` may be empty, so store type info with set_type_name and set_type_id.
  std::string type_name() const {
    if (_type_name.size()) {
      return _type_name;
    }

    if (!is_connection()) {
      // Fallback. May be unreliable(`var` could be empty).
      return _var.type_name();
    }

    return std::string();
  }

  template <typename T>
  void set_value(const T &v) {
    if (_type_name.empty()) {
      _type_name = value::TypeTraits<T>::type_name();
    }
    _var.set_value(v);
  }

  void set_var(primvar::PrimVar &v) {
    if (_type_name.empty()) {
      _type_name = v.type_name();
    }

    _var = v;
  }

  void set_var(primvar::PrimVar &&v) {
    if (_type_name.empty()) {
      _type_name = v.type_name();
    }

    _var = std::move(v);
  }

  /// @brief Get the value of Attribute of specified type.
  /// @tparam T value type
  /// @return The value if the underlying PrimVar is type T. Return
  /// nonstd::nullpt when type mismatch.
  template <typename T>
  nonstd::optional<T> get_value() const {
    return _var.get_value<T>();
  }

  template <typename T>
  bool get_value(T *v) const {
    if (!v) {
      return false;
    }

    nonstd::optional<T> ret = _var.get_value<T>();
    if (ret) {
      (*v) = std::move(ret.value());
      return true;
    }

    return false;
  }

  template<typename T>
  void set_timesample(const T &v, double t) {
    _var.set_timesample(t, v);
  }

  template<typename T>
  bool get_value(const double t, T *dst,
           value::TimeSampleInterpolationType interp =
               value::TimeSampleInterpolationType::Held) const {
    if (!dst) {
      return false;
    }

    if (is_timesamples()) {
      return _var.get_ts_value(dst, t, interp);
    } else {
      nonstd::optional<T> v = _var.get_value<T>();
      if (v) {
        (*dst) = v;
        return true;
      }

      return false;
    }
  }

  const AttrMeta &metas() const { return _metas; }
  AttrMeta &metas() { return _metas; }

  const primvar::PrimVar &get_var() const { return _var; }

  void set_blocked(bool onoff) { _blocked = onoff; }

  bool is_blocked() const { return _blocked; }

  Variability &variability() { return _variability; }
  Variability variability() const { return _variability; }

  bool is_uniform() const { return _variability == Variability::Uniform; }

  bool is_connection() const { return _paths.size(); }

  bool is_value() const {
    if (is_connection()) {
      return false;
    }

    if (is_blocked()) {
      return false;
    }

    return true;
  }

  bool is_timesamples() const {
    if (!is_value()) {
      return false;
    }

    return _var.is_timesamples();
  }

  void set_connection(const Path &path) {
    _paths.clear();
    _paths.push_back(path);
  }
  void set_connections(const std::vector<Path> &paths) { _paths = paths; }

  nonstd::optional<Path> get_connection() const {
    if (_paths.size() == 1) {
      return _paths[0];
    }
    return nonstd::nullopt;
  }

  const std::vector<Path> &connections() const { return _paths; }
  std::vector<Path> &connections() { return _paths; }

 private:
  std::string _name;  // attrib name
  Variability _variability{
      Variability::Varying};  // 'uniform` qualifier is handled with
                              // `variability=uniform`
  bool _blocked{false};       // Attribute Block('None')
  std::string _type_name;
  primvar::PrimVar _var;
  std::vector<Path> _paths;
  AttrMeta _metas;
};

// Generic container for Attribute or Relation/Connection. And has this property
// is custom or not (Need to lookup schema if the property is custom or not for
// Crate data)
// TODO: Move Connection to Attribute
// TODO: Deprecate `custom` attribute:
// https://github.com/PixarAnimationStudios/USD/issues/2069
class Property {
 public:
  enum class Type {
    EmptyAttrib,        // Attrib with no data.
    Attrib,             // Attrib which contains actual data
    Relation,           // `rel` with targetPath(s).
    NoTargetsRelation,  // `rel` with no targets.
    Connection,         // Connection attribute(`.connect` suffix)
  };

  Property() = default;

  Property(const std::string &type_name, bool custom) : _has_custom(custom) {
    _attrib.set_type_name(type_name);
    _type = Type::EmptyAttrib;
  }

  Property(const Attribute &a, bool custom) : _attrib(a), _has_custom(custom) {
    _type = Type::Attrib;
  }

  Property(Attribute &&a, bool custom)
      : _attrib(std::move(a)), _has_custom(custom) {
    _type = Type::Attrib;
  }

  // Relationship(typeless)
  Property(const Relationship &r, bool custom) : _rel(r), _has_custom(custom) {
    _type = Type::Relation;
  }

  // Relationship(typeless)
  Property(Relationship &&r, bool custom)
      : _rel(std::move(r)), _has_custom(custom) {
    _type = Type::Relation;
  }

  // Attribute Connection: has type
  Property(const Path &path, const std::string &prop_value_type_name,
           bool custom)
      : _prop_value_type_name(prop_value_type_name), _has_custom(custom) {
    _attrib.set_connection(path);
    _attrib.set_type_name(prop_value_type_name);
    _type = Type::Connection;
  }

  // Attribute Connection: has multiple targetPaths
  Property(const std::vector<Path> &paths,
           const std::string &prop_value_type_name, bool custom)
      : _prop_value_type_name(prop_value_type_name), _has_custom(custom) {
    _attrib.set_connections(paths);
    _attrib.set_type_name(prop_value_type_name);
    _type = Type::Connection;
  }

  bool is_attribute() const {
    return (_type == Type::EmptyAttrib) || (_type == Type::Attrib);
  }
  bool is_empty() const {
    return (_type == Type::EmptyAttrib) || (_type == Type::NoTargetsRelation);
  }
  bool is_relationship() const {
    return (_type == Type::Relation) || (_type == Type::NoTargetsRelation);
  }
  bool is_connection() const { return _type == Type::Connection; }

  nonstd::optional<Path> get_relationTarget() const {
    if (!is_connection()) {
      return nonstd::nullopt;
    }

    if (_rel.is_path()) {
      return _rel.targetPath;
    }

    return nonstd::nullopt;
  }

  std::vector<Path> get_relationTargets() const {
    std::vector<Path> pv;

    if (!is_connection()) {
      return pv;
    }

    if (_rel.is_path()) {
      pv.push_back(_rel.targetPath);
    } else if (_rel.is_pathvector()) {
      pv = _rel.targetPathVector;
    }

    return pv;
  }

  std::string value_type_name() const {
    if (is_connection()) {
      return _prop_value_type_name;
    } else if (is_relationship()) {
      // relation is typeless.
      return std::string();
    } else {
      return _attrib.type_name();
    }
  }

  bool has_custom() const { return _has_custom; }

  void set_property_type(Type ty) { _type = ty; }

  Type get_property_type() const { return _type; }

  void set_listedit_qual(ListEditQual qual) { _listOpQual = qual; }

  const Attribute &get_attribute() const { return _attrib; }

  Attribute &attribute() { return _attrib; }

  void set_attribute(const Attribute &attrib) {
    _attrib = attrib;
    _type = Type::Attrib;
  }

  const Relationship &get_relationship() const { return _rel; }

  Relationship &relationship() { return _rel; }

  ListEditQual get_listedit_qual() const { return _listOpQual; }

 private:
  Attribute _attrib;  // attribute(value or ".connect")

  // List Edit qualifier(Attribute can never be list editable)
  // TODO:  Store listEdit qualifier to `Relation`
  ListEditQual _listOpQual{ListEditQual::ResetToExplicit};

  Type _type{Type::EmptyAttrib};
  Relationship _rel;                  // Relation(`rel`)
  std::string _prop_value_type_name;  // for Connection.
  bool _has_custom{false};            // Qualified with 'custom' keyword?
};

struct XformOp {
  enum class OpType {
    // matrix
    Transform,

    // vector3
    Translate,
    Scale,

    // scalar
    RotateX,
    RotateY,
    RotateZ,

    // vector3
    RotateXYZ,
    RotateXZY,
    RotateYXZ,
    RotateYZX,
    RotateZXY,
    RotateZYX,

    // quaternion
    Orient,

    // Special token
    ResetXformStack,  // !resetXformStack!
  };

  // OpType op;
  OpType op_type;
  bool inverted{false};  // true when `!inverted!` prefix
  std::string
      suffix;  // may contain nested namespaces. e.g. suffix will be
               // ":blender:pivot" for "xformOp:translate:blender:pivot". Suffix
               // will be empty for "xformOp:translate"

  primvar::PrimVar _var;
  //const value::TimeSamples &get_ts() const { return _var.ts_raw(); }

  std::string get_value_type_name() const {
    return _var.type_name();
  }

  uint32_t get_value_type_id() const {
    return _var.type_id();
  }

  // TODO: Check if T is valid type.
  template <class T>
  void set_value(const T &v) {
    _var.set_value(v);
  }

  template <class T>
  void set_timesample(const float t, const T &v) {
    _var.set_timesample(t, v);
  }

  void set_timesamples(const value::TimeSamples &v) {
    _var.set_timesamples(v);
  }

  void set_timesamples(value::TimeSamples &&v) {
    _var.set_timesamples(v);
  }

  bool is_timesamples() const {
    return _var.is_timesamples();
  }

  nonstd::optional<value::TimeSamples> get_timesamples() const {
    if (is_timesamples()) {
      return _var.ts_raw();
    }
    return nonstd::nullopt;
  }

  nonstd::optional<value::Value> get_scalar() const {
    if (is_timesamples()) {
      return nonstd::nullopt;
    }
    return _var.value_raw();
  }

  // Type-safe way to get concrete value.
  template <class T>
  nonstd::optional<T> get_value() const {
    if (is_timesamples()) {
      return nonstd::nullopt;
    }

    return _var.get_value<T>();
  }

  const primvar::PrimVar &get_var() const {
    return _var;
  }

  primvar::PrimVar &var() {
    return _var;
  }

};

// Prim metas, Prim tree and properties.
struct VariantSet {
  PrimMeta metas;
  std::vector<int64_t> primIndices;
  std::map<std::string, Property> props;
};

// Generic primspec container.
struct Model {
  std::string name;

  Specifier spec{Specifier::Def};

  int64_t parent_id{-1};  // Index to parent node

  PrimMeta meta;

  std::pair<ListEditQual, std::vector<Reference>> references;
  std::pair<ListEditQual, std::vector<Payload>> payload;

  std::map<std::string, VariantSet> variantSet;

  std::map<std::string, Property> props;
};

#if 0  // TODO: Remove
// Generic "class" Node
// Mostly identical to GPrim
struct Klass {
  std::string name;
  int64_t parent_id{-1};  // Index to parent node

  std::vector<std::pair<ListEditQual, Reference>> references;

  std::map<std::string, Property> props;
};
#endif

struct MaterialBindingAPI {
  Path binding;            // rel material:binding
  Path bindingCorrection;  // rel material:binding:correction
  Path bindingPreview;     // rel material:binding:preview

  // TODO: allPurpose, preview, ...
};

//
// Predefined node classes
//

// USDZ Schemas for AR
// https://developer.apple.com/documentation/arkit/usdz_schemas_for_ar/schema_definitions_for_third-party_digital_content_creation_dcc

// UsdPhysics
struct Preliminary_PhysicsGravitationalForce {
  // physics::gravitatioalForce::acceleration
  value::double3 acceleration{{0.0, -9.81, 0.0}};  // [m/s^2]
};

struct Preliminary_PhysicsMaterialAPI {
  // preliminary:physics:material:restitution
  double restitution;  // [0.0, 1.0]

  // preliminary:physics:material:friction:static
  double friction_static;

  // preliminary:physics:material:friction:dynamic
  double friction_dynamic;
};

struct Preliminary_PhysicsRigidBodyAPI {
  // preliminary:physics:rigidBody:mass
  double mass{1.0};

  // preliminary:physics:rigidBody:initiallyActive
  bool initiallyActive{true};
};

struct Preliminary_PhysicsColliderAPI {
  // preliminary::physics::collider::convexShape
  Path convexShape;
};

struct Preliminary_InfiniteColliderPlane {
  value::double3 position{{0.0, 0.0, 0.0}};
  value::double3 normal{{0.0, 0.0, 0.0}};

  Extent extent;  // [-FLT_MAX, FLT_MAX]

  Preliminary_InfiniteColliderPlane() {
    extent.lower[0] = -(std::numeric_limits<float>::max)();
    extent.lower[1] = -(std::numeric_limits<float>::max)();
    extent.lower[2] = -(std::numeric_limits<float>::max)();
    extent.upper[0] = (std::numeric_limits<float>::max)();
    extent.upper[1] = (std::numeric_limits<float>::max)();
    extent.upper[2] = (std::numeric_limits<float>::max)();
  }
};

// UsdInteractive
struct Preliminary_AnchoringAPI {
  // preliminary:anchoring:type
  std::string type;  // "plane", "image", "face", "none";

  std::string alignment;  // "horizontal", "vertical", "any";

  Path referenceImage;
};

struct Preliminary_ReferenceImage {
  int64_t image_id{-1};  // asset image

  double physicalWidth{0.0};
};

struct Preliminary_Behavior {
  Path triggers;
  Path actions;
  bool exclusive{false};
};

struct Preliminary_Trigger {
  // uniform token info:id
  std::string info;  // Store decoded string from token id
};

struct Preliminary_Action {
  // uniform token info:id
  std::string info;  // Store decoded string from token id

  std::string multiplePerformOperation{
      "ignore"};  // ["ignore", "allow", "stop"]
};

struct Preliminary_Text {
  std::string content;
  std::vector<std::string> font;  // An array of font names

  float pointSize{144.0f};
  float width;
  float height;
  float depth{0.0f};

  std::string wrapMode{"flowing"};  // ["singleLine", "hardBreaks", "flowing"]
  std::string horizontalAlignmment{
      "center"};  // ["left", "center", "right", "justified"]
  std::string verticalAlignmment{
      "middle"};  // ["top", "middle", "lowerMiddle", "baseline", "bottom"]
};

// Simple volume class.
// Currently this is just an placeholder. Not implemented.

struct OpenVDBAsset {
  std::string fieldDataType{"float"};
  std::string fieldName{"density"};
  std::string filePath;  // asset
};

// MagicaVoxel Vox
struct VoxAsset {
  std::string fieldDataType{"float"};
  std::string fieldName{"density"};
  std::string filePath;  // asset
};

struct Volume {
  OpenVDBAsset vdb;
  VoxAsset vox;
};

// `Scope` is uncommon in graphics community, its something like `Group`.
// From USD doc: Scope is the simplest grouping primitive, and does not carry
// the baggage of transformability.
struct Scope {
  std::string name;
  Specifier spec{Specifier::Def};

  int64_t parent_id{-1};

  PrimMeta meta;

  Animatable<Visibility> visibility{Visibility::Inherited};
  Purpose purpose{Purpose::Default};

  std::map<std::string, VariantSet> variantSet;

  std::map<std::string, Property> props;
};

///
/// Get elementName from Prim(e.g., Xform::name, GeomMesh::name)
/// `v` must be the value of Prim class.
///
nonstd::optional<std::string> GetPrimElementName(const value::Value &v);

///
/// Set name for Prim `v`(e.g. Xform::name = elementName)
/// `v` must be the value of Prim class.
///
bool SetPrimElementName(value::Value &v, const std::string &elementName);

//
// For `Stage` scene graph.
// Similar to `Prim` in pxrUSD.
// This class uses tree-representation of `Prim`. Easy to use, but may not be
// performant than flattened Prim array + index representation of Prim
// tree(Index-based scene graph such like glTF).
//
class Prim {
 public:
  // elementName is read from `rhs`(if it is a class of Prim)
  Prim(const value::Value &rhs);
  Prim(value::Value &&rhs);

  Prim(const std::string &elementName, const value::Value &rhs);
  Prim(const std::string &elementName, value::Value &&rhs);

  template <typename T>
  Prim(const T &prim) {
    // Check if T is Prim class type.
    static_assert(
        (value::TypeId::TYPE_ID_MODEL_BEGIN <= value::TypeTraits<T>::type_id) &&
            (value::TypeId::TYPE_ID_MODEL_END > value::TypeTraits<T>::type_id),
        "T is not a Prim class type");
    _data = prim;
    // Use prim.name for elementName
    _elementPath = Path(prim.name, "");
  }

  template <typename T>
  Prim(const std::string &elementName, const T &prim) {
    // Check if T is Prim class type.
    static_assert(
        (value::TypeId::TYPE_ID_MODEL_BEGIN <= value::TypeTraits<T>::type_id) &&
            (value::TypeId::TYPE_ID_MODEL_END > value::TypeTraits<T>::type_id),
        "T is not a Prim class type");
    _data = prim;
    SetPrimElementName(_data, elementName);
    _elementPath = Path(elementName, "");
  }

  std::vector<Prim> &children() { return _children; }

  const std::vector<Prim> &children() const { return _children; }

  const value::Value &data() const { return _data; }

  Specifier &specifier() { return _specifier; }

  Specifier specifier() const { return _specifier; }

  Path &local_path() { return _path; }

  const Path &local_path() const { return _path; }

  Path &element_path() { return _elementPath; }
  const Path &element_path() const { return _elementPath; }

  // elementName = element_path's prim part
  const std::string element_name() const {
    return _elementPath.prop_part();
  }

  const std::string type_name() const { return _data.type_name(); }

  uint32_t type_id() const { return _data.type_id(); }

  template <typename T>
  bool is() const {
    return (_data.type_id() == value::TypeTraits<T>::type_id);
  }

  // Return a pointer of a concrete Prim class(Xform, Material, ...)
  // Return nullptr when failed to cast or T is not a Prim type.
  template <typename T>
  const T *as() const {
    // Check if T is Prim type. e.g. Xform, Material, ...
    if ((value::TypeId::TYPE_ID_MODEL_BEGIN <= value::TypeTraits<T>::type_id) &&
        (value::TypeId::TYPE_ID_MODEL_END > value::TypeTraits<T>::type_id)) {
      return _data.as<T>();
    }

    return nullptr;
  }

  const PrimMeta &metas() const; 
  PrimMeta &metas();

 private:
  Path _path;  // Prim's local path name. May contain Property, Relationship and
               // other infos, but do not include parent's path. To get fully
               // absolute path of a Prim(e.g. "/xform0/mymesh0", You need to
               // traverse Prim tree and concatename `elementPath` or use
               // ***(T.B.D>) method in `Stage` class
  Path _elementPath;  // leaf("terminal") Prim name.(e.g. "myxform" for `def
                      // Xform "myform"`). For root node, elementPath name is
                      // empty string("").
  Specifier _specifier{
      Specifier::Invalid};  // `def`, `over` or `class`. Usually `def`
  value::Value
      _data;  // Generic container for concrete Prim object. GPrim, Xform, ...
  std::vector<Prim> _children;  // child Prim nodes
};

///
/// Contains concrete Prim object and composition elements.
///
/// PrimNode is near to the final state of `Prim`.
/// Doing one further step(Composition, Flatten, select Variant) to get `Prim`.
///
/// Similar to `PrimIndex` in pxrUSD
///
class PrimNode {
  Path path;
  Path elementPath;

  PrimNode(const value::Value &rhs);

  PrimNode(value::Value &&rhs);

  value::Value prim;  // GPrim, Xform, ...

  std::vector<PrimNode> children;  // child nodes

  ///
  /// Select variant.
  ///
  bool select_variant(const std::string &target_name,
                      const std::string &variant_name) {
    const auto m = vsmap.find(target_name);
    if (m != vsmap.end()) {
      current_vsmap[target_name] = variant_name;
      return true;
    } else {
      return false;
    }
  }

  ///
  /// List variants in this Prim
  /// key = variant prim name
  /// value = variats
  ///
  const VariantSelectionMap &get_variant_selection_map() const { return vsmap; }

  ///
  /// Variants
  ///
  /// variant element = Property or Prim
  ///
  using PropertyMap = std::map<std::string, Property>;
  using PrimNodeMap = std::map<std::string, PrimNode>;

  VariantSelectionMap vsmap;          // Original variant selections
  VariantSelectionMap current_vsmap;  // Currently selected variants

  // key = variant_name
  std::map<std::string, PropertyMap> variantAttributeMap;
  std::map<std::string, PrimNodeMap> variantPrimNodeMap;

  ///
  /// Information for Crate(USDC binary)
  ///
  std::vector<value::token> primChildren;
  std::vector<value::token> variantChildren;
};

#if 0 // TODO: Remove
// Simple bidirectional Path(string) <-> index lookup
struct StringAndIdMap {
  void add(int32_t key, const std::string &val) {
    _i_to_s[key] = val;
    _s_to_i[val] = key;
  }

  void add(const std::string &key, int32_t val) {
    _s_to_i[key] = val;
    _i_to_s[val] = key;
  }

  size_t count(int32_t i) const { return _i_to_s.count(i); }

  size_t count(const std::string &s) const { return _s_to_i.count(s); }

  std::string at(int32_t i) const { return _i_to_s.at(i); }

  int32_t at(std::string s) const { return _s_to_i.at(s); }

  std::map<int32_t, std::string> _i_to_s;  // index -> string
  std::map<std::string, int32_t> _s_to_i;  // string -> index
};

struct NodeIndex {
  std::string name;

  // TypeTraits<T>::type_id
  value::TypeId type_id{value::TypeId::TYPE_ID_INVALID};

  int64_t index{-1};  // array index to `Scene::xforms`, `Scene::geom_cameras`,
                      // ... -1 = invlid(or not set)
};
#endif

nonstd::optional<Interpolation> InterpolationFromString(const std::string &v);
nonstd::optional<Orientation> OrientationFromString(const std::string &v);
nonstd::optional<Kind> KindFromString(const std::string &v);

// Return false when invalid character(e.g. '%') exists.
bool ValidatePrimName(const std::string &tok);

namespace value {

#include "define-type-trait.inc"

DEFINE_TYPE_TRAIT(Reference, "ref", TYPE_ID_REFERENCE, 1);
DEFINE_TYPE_TRAIT(Specifier, "specifier", TYPE_ID_SPECIFIER, 1);
DEFINE_TYPE_TRAIT(Permission, "permission", TYPE_ID_PERMISSION, 1);
DEFINE_TYPE_TRAIT(Variability, "variability", TYPE_ID_VARIABILITY, 1);

DEFINE_TYPE_TRAIT(VariantSelectionMap, "variants", TYPE_ID_VARIANT_SELECION_MAP,
                  0);

DEFINE_TYPE_TRAIT(Payload, "payload", TYPE_ID_PAYLOAD, 1);
DEFINE_TYPE_TRAIT(LayerOffset, "LayerOffset", TYPE_ID_LAYER_OFFSET, 1);

DEFINE_TYPE_TRAIT(ListOp<value::token>, "ListOpToken", TYPE_ID_LIST_OP_TOKEN,
                  1);
DEFINE_TYPE_TRAIT(ListOp<std::string>, "ListOpString", TYPE_ID_LIST_OP_STRING,
                  1);
DEFINE_TYPE_TRAIT(ListOp<Path>, "ListOpPath", TYPE_ID_LIST_OP_PATH, 1);
DEFINE_TYPE_TRAIT(ListOp<Reference>, "ListOpReference",
                  TYPE_ID_LIST_OP_REFERENCE, 1);
DEFINE_TYPE_TRAIT(ListOp<int32_t>, "ListOpInt", TYPE_ID_LIST_OP_INT, 1);
DEFINE_TYPE_TRAIT(ListOp<uint32_t>, "ListOpUInt", TYPE_ID_LIST_OP_UINT, 1);
DEFINE_TYPE_TRAIT(ListOp<int64_t>, "ListOpInt64", TYPE_ID_LIST_OP_INT64, 1);
DEFINE_TYPE_TRAIT(ListOp<uint64_t>, "ListOpUInt64", TYPE_ID_LIST_OP_UINT64, 1);
DEFINE_TYPE_TRAIT(ListOp<Payload>, "ListOpPayload", TYPE_ID_LIST_OP_PAYLOAD, 1);

DEFINE_TYPE_TRAIT(Path, "Path", TYPE_ID_PATH, 1);
DEFINE_TYPE_TRAIT(Relationship, "Relationship", TYPE_ID_RELATIONSHIP, 1);
// TODO(syoyo): Define PathVector as 1D array?
DEFINE_TYPE_TRAIT(std::vector<Path>, "PathVector", TYPE_ID_PATH_VECTOR, 1);

DEFINE_TYPE_TRAIT(std::vector<value::token>, "token[]", TYPE_ID_TOKEN_VECTOR,
                  1);

DEFINE_TYPE_TRAIT(value::TimeSamples, "TimeSamples", TYPE_ID_TIMESAMPLES, 1);

DEFINE_TYPE_TRAIT(Model, "Model", TYPE_ID_MODEL, 1);
DEFINE_TYPE_TRAIT(Scope, "Scope", TYPE_ID_SCOPE, 1);

DEFINE_TYPE_TRAIT(StringData, "string", TYPE_ID_STRING_DATA, 1);

DEFINE_TYPE_TRAIT(CustomDataType, "customData", TYPE_ID_CUSTOMDATA,
                  1);  // TODO: Unify with `dict`?

DEFINE_TYPE_TRAIT(Extent, "float3[]", TYPE_ID_EXTENT, 2);  // float3[2]

#undef DEFINE_TYPE_TRAIT
#undef DEFINE_ROLE_TYPE_TRAIT

}  // namespace value

namespace prim {

using PropertyMap = std::map<std::string, Property>;
using ReferenceList = std::pair<ListEditQual, std::vector<Reference>>;
using PayloadList = std::pair<ListEditQual, std::vector<Payload>>;

}  // namespace prim

// TODO(syoyo): Range, Interval, Rect2i, Frustum, MultiInterval
// and Quaternion?

/*
#define VT_GFRANGE_VALUE_TYPES                 \
((      GfRange3f,           Range3f        )) \
((      GfRange3d,           Range3d        )) \
((      GfRange2f,           Range2f        )) \
((      GfRange2d,           Range2d        )) \
((      GfRange1f,           Range1f        )) \
((      GfRange1d,           Range1d        ))

#define VT_RANGE_VALUE_TYPES                   \
    VT_GFRANGE_VALUE_TYPES                     \
((      GfInterval,          Interval       )) \
((      GfRect2i,            Rect2i         ))

#define VT_QUATERNION_VALUE_TYPES           \
((      GfQuaternion,        Quaternion ))

#define VT_NONARRAY_VALUE_TYPES                 \
((      GfFrustum,           Frustum))          \
((      GfMultiInterval,     MultiInterval))

*/

}  // namespace tinyusdz
