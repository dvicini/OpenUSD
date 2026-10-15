//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_USD_PY_CONVERSIONS_H
#define PXR_USD_USD_PY_CONVERSIONS_H

#include "pxr/pxr.h"
#include "pxr/usd/usd/api.h"
#include "pxr/base/tf/pyObjWrapper.h"
#include "pxr/base/vt/value.h"
#include "pxr/base/tf/pyNanobindBridge.h"
#include <nanobind/nanobind.h>

PXR_NAMESPACE_OPEN_SCOPE

class TfToken;
class SdfValueTypeName;

// XXX: DEPRECATED.  This function does nothing except convert \p value to
// python.  Do not call, it will be removed.
USD_API
TfPyObjWrapper UsdVtValueToPython(const VtValue &value);

/// Helper for converting a python value to the target Usd/Sdf type, if
/// possible.  Invokes VtValue::CastToTypeOf() to do the conversion, if
/// required.  This internally handles python buffers (e.g. numpy) -> VtArray
/// and some python tuple/list -> VtArray conversions.  If conversion fails,
/// returns a VtValue extracted from the pyVal, which may produce a VtValue
/// holding a python object.
USD_API
VtValue UsdPythonToSdfType(TfPyObjWrapper pyVal,
                           SdfValueTypeName const &targetType);

inline VtValue UsdPythonToSdfType(nanobind::handle pyVal,
                                  SdfValueTypeName const &targetType) {
    return UsdPythonToSdfType(
        TfPyObjWrapper(nanobind::borrow<nanobind::object>(pyVal)),
        targetType);
}

/// Helper for converting a python value to a metadata value for metadata
/// known to the SdfSchema.  Generates a coding error if \p key is unknown
/// to the SdfSchema.
///
/// For dictionary-valued metadata, \p keyPath may be specified as the path
/// in the dictionary we are targeting, so that if the dictionary was registered
/// with a fallback for that dictionary subcomponent, we will convert
/// appropriately to its type.
///
/// \return \c true on successful conversion, which can happen even if
/// the converted \p result is an empty VtValue
USD_API
bool UsdPythonToMetadataValue(const TfToken &key, const TfToken &keyPath, 
                              TfPyObjWrapper pyVal, VtValue *result);

inline bool UsdPythonToMetadataValue(const TfToken &key, const TfToken &keyPath,
                                     nanobind::handle pyVal, VtValue *result) {
    return UsdPythonToMetadataValue(
        key, keyPath,
        TfPyObjWrapper(nanobind::borrow<nanobind::object>(pyVal)),
        result);
}

class UsdPrimRange;

USD_API
void Usd_RegisterPrimRangePythonConversions(
    PyObject *(*toPy)(const UsdPrimRange &),
    bool (*fromPy)(PyObject *, UsdPrimRange *));

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_USD_PY_CONVERSIONS_H
