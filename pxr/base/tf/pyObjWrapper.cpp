//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"

#include "pxr/base/tf/pyObjWrapper.h"

#ifdef PXR_PYTHON_SUPPORT_ENABLED
#include "pxr/base/tf/pyLock.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/type.h"

#include <utility>

PXR_NAMESPACE_OPEN_SCOPE

TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<TfPyObjWrapper>();
}

namespace {

// A custom deleter for shared_ptr<nanobind::object> that takes the
// python lock before deleting the python object.  This is necessary since it's
// invalid to decrement the python refcount without holding the lock.
struct _DeleteObjectWithLock {
    void operator()(nanobind::object *obj) const {
        PXR_NS::TfPyLock lock;
        delete obj;
    }
};
}

TfPyObjWrapper::TfPyObjWrapper()
{
    TfPyLock lock;
    *this = TfPyObjWrapper(nanobind::none());
}

TfPyObjWrapper::TfPyObjWrapper(nanobind::object obj)
    : _objectPtr(
          new object(obj.is_valid() ? std::move(obj) : nanobind::none()),
          _DeleteObjectWithLock())
{
}

PyObject *
TfPyObjWrapper::ptr() const
{
    return _objectPtr->ptr();
}

bool
TfPyObjWrapper::operator==(TfPyObjWrapper const &other) const
{
    // If they point to the exact same object instance, we know they're equal.
    if (_objectPtr == other._objectPtr)
        return true;

    // Otherwise lock and let python determine equality.
    TfPyLock lock;
    return Get().equal(other.Get());
}

bool
TfPyObjWrapper::operator!=(TfPyObjWrapper const &other) const
{
    return !(*this == other);
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_PYTHON_SUPPORT_ENABLED
