//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_VT_VALUE_FROM_PYTHON_H
#define PXR_BASE_VT_VALUE_FROM_PYTHON_H

/// \file vt/valueFromPython.h

#include "pxr/pxr.h"
#include "pxr/base/vt/api.h"
#include "pxr/base/vt/value.h"

#include "pxr/base/tf/hash.h"
#include "pxr/base/tf/hashmap.h"
#include "pxr/base/tf/pyNanobindBridge.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/singleton.h"
#include "pxr/base/tf/stringUtils.h"

#include "pxr/base/tf/pySafePython.h"

#include <string>
#include <type_traits>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

// Lets python explicitly pass in values of C++ types which have no python
// equivalents.
struct Vt_ValueWrapper {
    template <class T>
    static Vt_ValueWrapper Create(T a) { return Vt_ValueWrapper(a); }

    template <typename T> explicit Vt_ValueWrapper(T val) : _val(val) {}

    VtValue const &GetValue() const { return _val; }

    bool operator==(const Vt_ValueWrapper &other) const {
        return _val == other._val;
    }

    bool operator!=(const Vt_ValueWrapper &other) const {
        return _val != other._val;
    }

    std::string GetAsString() const {
        return TfStringPrintf(
            "%s(%s)", _val.GetTypeName().c_str(), TfStringify(_val).c_str());
    }

private:
    VtValue _val;
};

/// \class Vt_ValueFromPythonRegistry
///
class Vt_ValueFromPythonRegistry {
public:

    static bool HasConversions() {
        return !_GetInstance()._lvalueExtractors.empty() &&
               !_GetInstance()._rvalueExtractors.empty();
    }
    
    VT_API static VtValue Invoke(PyObject *obj);

    template <class T>
    static void Register(bool registerRvalue) {
        if (!TfPyIsInitialized()) {
            TF_FATAL_ERROR("Tried to register a VtValue from python conversion "
                           "but python is not initialized!");
        }
        _GetInstance()._RegisterLValue(_Extractor::MakeLValue<T>());
        if (registerRvalue)
            _GetInstance()._RegisterRValue(_Extractor::MakeRValue<T>());
    }

    Vt_ValueFromPythonRegistry(Vt_ValueFromPythonRegistry const&) = delete;
    Vt_ValueFromPythonRegistry& operator=(
        Vt_ValueFromPythonRegistry const&) = delete;

    Vt_ValueFromPythonRegistry(Vt_ValueFromPythonRegistry &&) = delete;
    Vt_ValueFromPythonRegistry& operator=(
        Vt_ValueFromPythonRegistry &&) = delete;

private:
    Vt_ValueFromPythonRegistry() {}
    VT_API ~Vt_ValueFromPythonRegistry();

    friend class TfSingleton<Vt_ValueFromPythonRegistry>;

    class _Extractor {
    private:
        using _ExtractFunc = VtValue (*)(PyObject *);

        // _ExtractLValue will attempt to obtain an l-value T from the python
        // object it's passed.  This effectively disallows type conversions
        // (other than things like derived-to-base type conversions).
        template <class T>
        static VtValue _ExtractLValue(PyObject *);

        // _ExtractRValue will attempt to obtain an r-value T from the python
        // object it's passed.  This allows nanobind to invoke type
        // conversions to produce the T.
        template <class T>
        static VtValue _ExtractRValue(PyObject *);

    public:

        template <class T>
        static _Extractor MakeLValue() {
            return _Extractor(&_ExtractLValue<T>);
        }
        
        template <class T>
        static _Extractor MakeRValue() {
            return _Extractor(&_ExtractRValue<T>);
        }
        
        VtValue Invoke(PyObject *obj) const {
            return _extract(obj);
        }

    private:
        explicit _Extractor(_ExtractFunc extract) : _extract(extract) {}

        _ExtractFunc _extract;
    };

    VT_API static Vt_ValueFromPythonRegistry &_GetInstance() {
        return TfSingleton<Vt_ValueFromPythonRegistry>::GetInstance();
    }
    
    VT_API void _RegisterLValue(_Extractor const &e);
    VT_API void _RegisterRValue(_Extractor const &e);
    
    std::vector<_Extractor> _lvalueExtractors;
    std::vector<_Extractor> _rvalueExtractors;

    typedef TfHashMap<PyObject *, _Extractor, TfHash> _LValueExtractorCache;
    _LValueExtractorCache _lvalueExtractorCache;

};

VT_API_TEMPLATE_CLASS(TfSingleton<Vt_ValueFromPythonRegistry>);

template <class T>
VtValue Vt_ValueFromPythonRegistry::
_Extractor::_ExtractLValue(PyObject *obj) {
    if constexpr (!std::is_enum_v<T>) {
        T *ptr = nullptr;
        if (NB_CALL(nb_type_get)(
                NB_CTX, &typeid(T), obj, 0, nullptr,
                reinterpret_cast<void **>(&ptr)) && ptr) {
            return VtValue(*ptr);
        }
        if (PyErr_Occurred()) {
            PyErr_Clear();
        }
    }
    return VtValue();
}

template <class T>
VtValue Vt_ValueFromPythonRegistry::
_Extractor::_ExtractRValue(PyObject *obj) {
    nanobind::detail::make_caster<T> caster;
    nanobind::detail::cleanup_list cleanup(obj);
    bool ok = caster.from_python(
        nanobind::handle(obj),
        static_cast<uint8_t>(nanobind::detail::cast_flags::convert),
        &cleanup);
    if (ok) {
        VtValue result(
            static_cast<T>(caster.operator nanobind::detail::cast_t<T>()));
        cleanup.release();
        return result;
    }
    cleanup.release();
    if (PyErr_Occurred()) {
        PyErr_Clear();
    }
    return VtValue();
}

template <class T>
void VtValueFromPython() {
    Vt_ValueFromPythonRegistry::Register<T>(/* registerRvalue = */ true);
}

template <class T>
void VtValueFromPythonLValue() {
    Vt_ValueFromPythonRegistry::Register<T>(/* registerRvalue = */ false);
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_VT_VALUE_FROM_PYTHON_H
