//
// Copyright 2020 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_USD_AR_PY_RESOLVER_CONTEXT_H
#define PXR_USD_AR_PY_RESOLVER_CONTEXT_H

/// \file ar/pyResolverContext.h
/// Macros for creating Python bindings for objects used with 
/// ArResolverContext.

#include "pxr/base/tf/pyNanobindBridge.h"
#include "pxr/pxr.h"
#include "pxr/usd/ar/api.h"
#include "pxr/usd/ar/resolverContext.h"

PXR_NAMESPACE_OPEN_SCOPE

/// Register the specified type as a context object that may be converted from
/// Python into a ArResolverContext object in C++.  This typically would be
/// called in the source file where the Python wrapping for the context object
/// is defined.
template <class Context>
void 
ArWrapResolverContextForPython();

#ifndef doxygen

using Ar_ConvertContextFromPythonFn =
    bool (*)(PyObject*, ArResolverContext*);

AR_API
void Ar_RegisterResolverContextForPython(Ar_ConvertContextFromPythonFn fn);

AR_API
bool Ar_ConvertFromPythonToResolverContext(
    PyObject* obj, ArResolverContext* ctx);

template <class Context>
bool Ar_ConvertContextFromPython(PyObject* obj, ArResolverContext* ctx)
{
    Context* ptr = nullptr;
    if (NB_CALL(nb_type_get)(NB_CTX, &typeid(Context), obj, 0, nullptr,
                             reinterpret_cast<void**>(&ptr)) && ptr) {
        if (ctx) {
            *ctx = ArResolverContext(*ptr);
        }
        return true;
    }
    return false;
}

template <class Context>
void 
ArWrapResolverContextForPython()
{
    Ar_RegisterResolverContextForPython(&Ar_ConvertContextFromPython<Context>);
}

#endif //doxygen

PXR_NAMESPACE_CLOSE_SCOPE

#endif
