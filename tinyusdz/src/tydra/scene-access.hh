// SPDX-License-Identifier: Apache 2.0
// Copyright 2022-Present Light Transport Entertainment, Inc.
//
// Scene access API
//
// NOTE: Tydra API does not use nonstd::optional and nonstd::expected, std::functions and other non basic STL feature for easier language bindings.
//
#pragma once

#include <map>

#include "prim-types.hh"
#include "stage.hh"
#include "usdGeom.hh"
#include "usdShade.hh"
#include "usdSkel.hh"

namespace tinyusdz {
namespace tydra {

// key = fully absolute Prim path in string(e.g. "/xform/geom0")
template <typename T>
using PathPrimMap = std::map<std::string, const T *>;

//
// value = pair of Shader Prim which contains the Shader type T("info:id") and
// its concrete Shader type(UsdPreviewSurface)
//
template <typename T>
using PathShaderMap =
    std::map<std::string, std::pair<const Shader *, const T *>>;

///
/// List Prim of type T from the Stage.
/// Returns false when unsupported/unimplemented Prim type T is given.
///
template <typename T>
bool ListPrims(const tinyusdz::Stage &stage, PathPrimMap<T> &m /* output */);

///
/// List Shader of shader type T from the Stage.
/// Returns false when unsupported/unimplemented Shader type T is given.
/// TODO: User defined shader type("info:id")
///
template <typename T>
bool ListShaders(const tinyusdz::Stage &stage,
                 PathShaderMap<T> &m /* output */);

///
/// Get parent Prim from Path.
/// Path must be fully expanded absolute path.
///
/// Example: Return "/xform" Prim for "/xform/mesh0" path
///
/// Returns nullptr when the given Path is a root Prim or invalid Path(`err` will be filled when failed).
///
const Prim *GetParentPrim(const tinyusdz::Stage &stage, const tinyusdz::Path &path, std::string *err);


///
/// Visit Stage and invoke callback functions for each Prim.
/// Can be used for alternative method of Stage::Traverse() in pxrUSD
///

///
/// Use old-style Callback function approach for easier language bindings
///
/// @param[in] prim Prim
/// @param[in] tree_depth Tree depth of this Prim. 0 = root prim. 
/// @param[inout] userdata User data.
///
/// @return Usually true. false to notify stop visiting Prims further.
///
typedef bool (*VisitPrimFunction)(const Prim &prim, const int32_t tree_depth, void *userdata);

void VisitPrims(const tinyusdz::Stage &stage, VisitPrimFunction visitor_fun, void *userdata=nullptr);

}  // namespace tydra
}  // namespace tinyusdz
