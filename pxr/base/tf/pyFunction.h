//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_TF_PY_FUNCTION_H
#define PXR_BASE_TF_PY_FUNCTION_H

#include "pxr/pxr.h"

#include "pxr/base/tf/pyCall.h"
#include "pxr/base/tf/pyLock.h"
#include "pxr/base/tf/pyObjWrapper.h"
#include "pxr/base/tf/pyUtils.h"

#include <nanobind/nanobind.h>

#include <functional>
#include <string>

PXR_NAMESPACE_OPEN_SCOPE

template <typename Return>
struct TfPyCall;

template <typename T>
struct TfPyFunctionFromPython;

template <typename Ret, typename... Args>
struct TfPyFunctionFromPython<Ret (Args...)>
{
    struct Call
    {
        TfPyObjWrapper callable;

        Ret operator()(Args... args) {
            TfPyLock lock;
            return TfPyCall<Ret>(callable)(args...);
        }
    };

    struct CallWeak
    {
        TfPyObjWrapper weak;

        Ret operator()(Args... args) {
            // Attempt to get the referenced callable object.
            TfPyLock lock;
            PyObject *callablePtr;
            if (Tf_PyWeakrefGetRef(weak.ptr(), &callablePtr) != 1) {
                TF_WARN("Tried to call an expired python callback");
                return Ret();
            }
            nanobind::object callable =
                nanobind::steal<nanobind::object>(callablePtr);
            return TfPyCall<Ret>(callable)(args...);
        }
    };

    struct CallMethod
    {
        TfPyObjWrapper func;
        TfPyObjWrapper weakSelf;

        Ret operator()(Args... args) {
            // Attempt to get the referenced self parameter, then build a new
            // instance method and call it.
            TfPyLock lock;
            PyObject *selfPtr;
            if (Tf_PyWeakrefGetRef(weakSelf.ptr(), &selfPtr) != 1) {
                TF_WARN("Tried to call a method on an expired python instance");
                return Ret();
            }
            nanobind::object self = nanobind::steal<nanobind::object>(selfPtr);
            nanobind::object method = nanobind::steal<nanobind::object>(
                PyMethod_New(func.ptr(), self.ptr()));
            return TfPyCall<Ret>(method)(args...);
        }
    };

    TfPyFunctionFromPython() = default;

    template <typename FuncType>
    static void
    RegisterFunctionType() {}

    static void *convertible(PyObject *obj) {
        return ((obj == Py_None) || PyCallable_Check(obj)) ? obj : 0;
    }

    template <typename FuncType = std::function<Ret (Args...)>>
    static FuncType ConstructFunction(PyObject *src) {
        if (src == Py_None) {
            return FuncType();
        }
        nanobind::object callable = nanobind::borrow<nanobind::object>(src);
        PyObject *pyCallable = callable.ptr();
        PyObject *self =
            PyMethod_Check(pyCallable) ?
            PyMethod_GET_SELF(pyCallable) : NULL;

        if (self) {
            nanobind::object func = nanobind::borrow<nanobind::object>(
                PyMethod_GET_FUNCTION(pyCallable));
            nanobind::object weakSelf = nanobind::steal<nanobind::object>(
                PyWeakref_NewRef(self, NULL));
            return FuncType(CallMethod{
                TfPyObjWrapper(func),
                TfPyObjWrapper(weakSelf)
            });
        } else {
            bool isLambda = false;
            if (PyObject_HasAttrString(pyCallable, "__name__")) {
                std::string name;
                if (nanobind::try_cast<std::string>(
                        nanobind::object(callable.attr("__name__")), name) &&
                    name == "<lambda>") {
                    isLambda = true;
                }
            }
            if (isLambda) {
                return FuncType(Call{TfPyObjWrapper(callable)});
            }
            if (PyObject *weakCallable = PyWeakref_NewRef(pyCallable, NULL)) {
                return FuncType(CallWeak{TfPyObjWrapper(
                    nanobind::steal<nanobind::object>(weakCallable))});
            } else {
                PyErr_Clear();
                return FuncType(Call{TfPyObjWrapper(callable)});
            }
        }
    }
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_TF_PY_FUNCTION_H
