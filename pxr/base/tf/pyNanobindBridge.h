//
// Copyright 2026 Google LLC
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#ifndef PXR_BASE_TF_PY_NANOBIND_BRIDGE_H
#define PXR_BASE_TF_PY_NANOBIND_BRIDGE_H

#include <nanobind/make_iterator.h>
#include <nanobind/nanobind.h>
#include <nanobind/operators.h>
#include <nanobind/stl/detail/nb_dict.h>
#include <nanobind/stl/detail/nb_list.h>
#include <nanobind/stl/detail/nb_set.h>
#include <nanobind/stl/map.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/set.h>
#include <nanobind/stl/shared_ptr.h>
namespace nanobind::detail {
template <typename T, typename Enable = void>
struct Tf_NbOrigStringTypeCaster;
}
#define type_caster Tf_NbOrigStringTypeCaster
#include <nanobind/stl/function.h>
#include <nanobind/stl/string.h>
#undef type_caster
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/unique_ptr.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/unordered_set.h>
#include <nanobind/stl/vector.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "pxr/base/arch/demangle.h"
#include "pxr/base/tf/anyWeakPtr.h"
#include "pxr/base/tf/enum.h"
#include "pxr/base/tf/hashmap.h"
#include "pxr/base/tf/hashset.h"
#include "pxr/base/tf/pyEnum.h"
#include "pxr/base/tf/pyIdentity.h"
#include "pxr/base/tf/pyLock.h"
#include "pxr/base/tf/pyObjWrapper.h"
#include "pxr/base/tf/refPtr.h"
#include "pxr/base/tf/smallVector.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"
#include "pxr/base/tf/weakPtr.h"
#include "pxr/pxr.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_API nanobind::module_ TfPyGetCurrentNanobindModule();
template <typename T>
struct TfPyFunctionFromPython;

template <typename T>
class VtArray;
class VtDictionary;
class VtValue;
template <class T>
class SdfHandle;

namespace pxr_half {
class half;
}
using GfHalf = pxr_half::half;

class ArAsset;
class Ar_PyAsset {
 public:
  explicit Ar_PyAsset(std::shared_ptr<ArAsset> asset)
      : _asset(std::move(asset)) {}
  std::shared_ptr<ArAsset> _asset;
};
class ArResolvedPath;
class ArResolverContext;
bool Ar_ConvertFromPythonToResolverContext(PyObject* obj,
                                           ArResolverContext* ctx);

class SdfSpec;
namespace Sdf_PySpecDetail {
PyObject* _CreateHolder(const std::type_info&, const SdfSpec&);
}

class SdrShaderNode;
class UsdStage;
class UsdPrimDefinition;
class UsdPrimTypeInfo;
class UsdSchemaRegistry;
class UsdStageCacheContext;
class UsdNamespaceEditor;

PXR_NAMESPACE_CLOSE_SCOPE

namespace std {
template <>
struct is_copy_constructible<PXR_NS::SdrShaderNode> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::SdrShaderNode> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdStage> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::UsdStage> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdPrimDefinition> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::UsdPrimDefinition> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdPrimTypeInfo> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::UsdPrimTypeInfo> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdSchemaRegistry> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::UsdSchemaRegistry> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdStageCacheContext> : std::false_type {};
template <>
struct is_move_constructible<PXR_NS::UsdStageCacheContext> : std::false_type {};
template <>
struct is_copy_constructible<PXR_NS::UsdNamespaceEditor> : std::false_type {};
}  // namespace std

namespace nanobind {
namespace detail {

template <typename Return, typename... Args>
struct type_caster<std::function<Return(Args...)>> {
  using ReturnCaster = make_caster<
      std::conditional_t<std::is_void_v<Return>, void_type, Return>>;

  NB_TYPE_CASTER(std::function<Return(Args...)>,
                 const_name("collections.abc.Callable[[") +
                     concat(make_caster<Args>::Name...) + const_name("], ") +
                     ReturnCaster::Name + const_name("]"))

  bool from_python(handle src, uint8_t flags, cleanup_list*) noexcept {
    if (src.is_none()) {
      if (!(flags & static_cast<uint8_t>(cast_flags::convert))) {
        return false;
      }
      value = nullptr;
      return true;
    }
    if (!PyCallable_Check(src.ptr())) {
      return false;
    }
    value = PXR_NS::TfPyFunctionFromPython<Return(Args...)>::ConstructFunction(
        src.ptr());
    return true;
  }

  static handle from_cpp(const Value& value, rv_policy rvp,
                         cleanup_list*) noexcept {
    if (rvp == rv_policy::none) {
      return handle();
    }
    if (!value) {
      return none().release();
    }
    return cpp_function(value).release();
  }
};

template <typename Type, uint32_t N>
struct type_caster<PXR_NS::TfSmallVector<Type, N>>
    : list_caster<PXR_NS::TfSmallVector<Type, N>, Type> {};

template <typename Key, class HashFn, class EqualKey, class Alloc>
struct type_caster<PXR_NS::TfHashSet<Key, HashFn, EqualKey, Alloc>>
    : set_caster<PXR_NS::TfHashSet<Key, HashFn, EqualKey, Alloc>, Key> {};

template <typename Key, typename Value, class HashFn, class EqualKey,
          class Alloc>
struct type_caster<PXR_NS::TfHashMap<Key, Value, HashFn, EqualKey, Alloc>>
    : dict_caster<PXR_NS::TfHashMap<Key, Value, HashFn, EqualKey, Alloc>, Key,
                  Value> {};

template <>
struct type_caster<PXR_NS::TfPyObjWrapper> {
  NB_TYPE_CASTER(PXR_NS::TfPyObjWrapper, const_name("object"))

  bool from_python(handle src, uint8_t, cleanup_list*) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    value = PXR_NS::TfPyObjWrapper(nanobind::borrow<nanobind::object>(src));
    return true;
  }

  static handle from_cpp(const PXR_NS::TfPyObjWrapper& v, rv_policy,
                         cleanup_list*) noexcept {
    PyObject* p = v.ptr();
    if (!p) {
      Py_RETURN_NONE;
    }
    Py_INCREF(p);
    return handle(p);
  }
};

template <typename T>
struct type_caster<T, enable_if_t<std::is_same_v<T, PXR_NS::TfAnyWeakPtr>>> {
  NB_TYPE_CASTER(T, const_name("Tf.AnyWeakPtr"))

  bool from_python(handle src, uint8_t flags, cleanup_list*) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    if (src.is_none()) {
      if (flags & static_cast<uint8_t>(cast_flags::none_disallowed)) {
        return false;
      }
      value = T();
      return true;
    }
    return PXR_NS::Tf_PyExtractAnyWeakPtr(src.ptr(), &value);
  }

  static handle from_cpp(const T& v, rv_policy,
                         cleanup_list*) noexcept {
    PXR_NS::TfPyObjWrapper obj = Tf_GetPythonObjectFromAnyWeakPtr(v);
    PyObject* p = obj.ptr();
    if (!p) {
      Py_RETURN_NONE;
    }
    Py_INCREF(p);
    return handle(p);
  }
};

template <typename T>
struct type_caster<T, enable_if_t<std::is_same_v<T, PXR_NS::TfEnum>>> {
  NB_TYPE_CASTER(T, const_name("Tf.Enum"))

  bool from_python(handle src, uint8_t, cleanup_list*) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    return PXR_NS::Tf_PyEnumRegistry::GetInstance().FindEnum(src.ptr(), &value);
  }

  static handle from_cpp(const T& src, rv_policy,
                         cleanup_list*) noexcept {
    return PXR_NS::Tf_PyEnumRegistry::GetInstance().GetPyObject(src);
  }
};

template <>
struct type_caster<std::string> {
  NB_TYPE_CASTER(std::string, const_name("str"))

  bool from_python(handle src, uint8_t flags, cleanup_list*) noexcept {
    Py_ssize_t size;
    if (PyBytes_Check(src.ptr())) {
      char* bstr = nullptr;
      if (PyBytes_AsStringAndSize(src.ptr(), &bstr, &size) == 0 && bstr) {
        value.assign(bstr, static_cast<size_t>(size));
        return true;
      }
      PyErr_Clear();
      return false;
    }
    const char* str = PyUnicode_AsUTF8AndSize(src.ptr(), &size);
    if (str) {
      value.assign(str, static_cast<size_t>(size));
      return true;
    }
    PyErr_Clear();
    if ((flags & static_cast<uint8_t>(cast_flags::convert)) &&
        src.ptr() != nullptr && src.ptr() != Py_None) {
      const char* tpName = Py_TYPE(src.ptr())->tp_name;
      if (tpName &&
          std::string_view(tpName).find("ResolvedPath") !=
              std::string_view::npos) {
        PyObject* strObj = PyObject_Str(src.ptr());
        if (strObj) {
          const char* s = PyUnicode_AsUTF8AndSize(strObj, &size);
          if (s) {
            value.assign(s, static_cast<size_t>(size));
            Py_DECREF(strObj);
            return true;
          }
          Py_DECREF(strObj);
        }
        PyErr_Clear();
      }
    }
    return false;
  }

  static handle from_cpp(const std::string& value, rv_policy,
                         cleanup_list*) noexcept {
    return PyUnicode_FromStringAndSize(value.c_str(), value.size());
  }
};

template <typename T>
struct custom_enum_type_caster<T, std::enable_if_t<std::is_enum_v<T>>>
    : std::true_type {};

template <typename T>
struct type_caster<T, enable_if_t<std::is_enum_v<T>>> {
  NB_TYPE_CASTER(T, const_name<T>())

  bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    PXR_NS::TfEnum tfEnum;
    if (PXR_NS::Tf_PyEnumRegistry::GetInstance().FindEnum(src.ptr(), &tfEnum)) {
      if (tfEnum.IsA<T>()) {
        value = static_cast<T>(tfEnum.GetValueAsInt());
        return true;
      }
      return false;
    }
    if (nanobind::type<T>().is_valid()) {
      int64_t result = 0;
      if (NB_CALL(enum_from_python)(NB_CTX_C(cleanup), &typeid(T), src.ptr(),
                                    &result, flags)) {
        value = static_cast<T>(result);
        return true;
      }
    }
    return false;
  }

  static handle from_cpp(T src, rv_policy, cleanup_list* cleanup) noexcept {
    if (nanobind::type<T>().is_valid()) {
      return NB_CALL(enum_from_cpp)(NB_CTX_C(cleanup), &typeid(T),
                                    static_cast<int64_t>(src));
    }
    return PXR_NS::Tf_PyEnumRegistry::GetInstance().GetPyObject(
        PXR_NS::TfEnum(src));
  }
};

template <typename T>
struct type_caster<PXR_NS::TfWeakPtr<T>> {
  using WeakPtrType = PXR_NS::TfWeakPtr<T>;
  using Pointee = typename WeakPtrType::DataType;
  using NonConstPointee = std::remove_const_t<Pointee>;
  using NonConstWeakPtr = PXR_NS::TfWeakPtr<NonConstPointee>;

  NB_TYPE_CASTER(WeakPtrType, const_name<NonConstPointee>())

  bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    if (src.is_none()) {
      if ((flags & static_cast<uint8_t>(cast_flags::construct)) ||
          (flags & static_cast<uint8_t>(cast_flags::none_disallowed))) {
        return false;
      }
      value = WeakPtrType();
      return true;
    }
    NonConstPointee* ptr = nullptr;
    if (!NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(NonConstPointee),
                              src.ptr(), flags, cleanup,
                              reinterpret_cast<void**>(&ptr)) ||
        !ptr) {
      return false;
    }
    value = WeakPtrType(ptr);
    if (value) {
      PXR_NS::Tf_PySetPythonIdentity(value, src.ptr());
    }
    return true;
  }

  static handle from_cpp(const WeakPtrType& p, rv_policy,
                         cleanup_list*) noexcept {
    if (!p.GetUniqueIdentifier()) {
      return none().release();
    }
    get_pointer(p);
    if (PyObject* id = PXR_NS::Tf_PyGetPythonIdentity(p)) {
      return handle(id);
    }
    nanobind::handle pyType = nanobind::type<NonConstPointee>();
    if (pyType.is_valid()) {
      Pointee* rawPtr = get_pointer(p);
      if (!rawPtr) {
        nanobind::object nbObj = nanobind::inst_alloc_zero(pyType);
        if (nbObj.is_valid()) {
          return nbObj.release();
        }
        return none().release();
      }
      void* instPtr = const_cast<NonConstPointee*>(rawPtr);
      if constexpr (std::is_polymorphic_v<NonConstPointee>) {
        if (PyObject* dynType =
                NB_CALL(nb_type_lookup)(NB_CTX, &typeid(*rawPtr))) {
          pyType = dynType;
          instPtr = const_cast<void*>(PXR_NS::TfCastToMostDerivedType(rawPtr));
        }
      }
      nanobind::object nbObj = nanobind::inst_reference(pyType, instPtr);
      if (nbObj.is_valid()) {
        PyObject* res = nbObj.release().ptr();
        PXR_NS::Tf_PySetPythonIdentity(
            PXR_NS::TfConst_cast<NonConstWeakPtr>(p), res);
        return handle(res);
      }
    }
    return none().release();
  }
};

template <typename T>
struct type_caster<PXR_NS::TfRefPtr<T>> {
  using RefPtrType = PXR_NS::TfRefPtr<T>;
  using Pointee = typename RefPtrType::DataType;
  using NonConstPointee = std::remove_const_t<Pointee>;
  using NonConstRefPtr = PXR_NS::TfRefPtr<NonConstPointee>;
  using WeakPtrType = PXR_NS::TfWeakPtr<NonConstPointee>;

  NB_TYPE_CASTER(RefPtrType, const_name<NonConstPointee>())

  bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    if (src.is_none()) {
      if ((flags & static_cast<uint8_t>(cast_flags::construct)) ||
          (flags & static_cast<uint8_t>(cast_flags::none_disallowed))) {
        return false;
      }
      value = RefPtrType();
      return true;
    }
    NonConstPointee* ptr = nullptr;
    if (!NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(NonConstPointee),
                              src.ptr(), flags, cleanup,
                              reinterpret_cast<void**>(&ptr)) ||
        !ptr) {
      return false;
    }
    NonConstRefPtr nonConstPtr(ptr);
    PXR_NS::Tf_PyRemovePythonOwnership(nonConstPtr, src.ptr());
    if constexpr (PXR_NS::Tf_SupportsWeakPtr<NonConstPointee>::value) {
      WeakPtrType weakPtr(ptr);
      if (weakPtr) {
        PXR_NS::Tf_PySetPythonIdentity(weakPtr, src.ptr());
      }
    }
    value = RefPtrType(ptr);
    return true;
  }

  static handle from_cpp(const RefPtrType& src, rv_policy policy,
                         cleanup_list* cleanup) noexcept {
    if (!src) {
      return none().release();
    }
    if constexpr (PXR_NS::Tf_SupportsWeakPtr<NonConstPointee>::value) {
      WeakPtrType weakPtr(const_cast<NonConstPointee*>(get_pointer(src)));
      handle h = type_caster<WeakPtrType>::from_cpp(weakPtr, policy, cleanup);
      if (h.is_valid() && h.ptr() != Py_None &&
          !PyObject_HasAttrString(h.ptr(), "__owner")) {
        NonConstRefPtr nonConstRef(
            const_cast<NonConstPointee*>(get_pointer(src)));
        PXR_NS::Tf_PyAddPythonOwnership(
            nonConstRef, weakPtr.GetUniqueIdentifier(), h.ptr());
      }
      return h;
    } else {
      return type_caster_base<NonConstPointee>::from_cpp(
          const_cast<NonConstPointee*>(get_pointer(src)),
          rv_policy::reference, cleanup);
    }
  }
};

template <typename T>
struct type_caster<PXR_NS::VtArray<T>> : type_caster_base_tag {
  using ArrayType = PXR_NS::VtArray<T>;
  using Value = ArrayType;
  template <typename T_>
  using Cast = precise_cast_t<T_>;
  static constexpr auto Name = nanobind::detail::const_name<ArrayType>();

  bool from_python(handle src, uint32_t flags, cleanup_list* cleanup) noexcept {
    if (src.is_none() &&
        !(flags & static_cast<uint32_t>(cast_flags::construct))) {
      if (!(flags & static_cast<uint32_t>(cast_flags::none_disallowed))) {
        ptr_ = nullptr;
        return true;
      }
      return false;
    }

    if (NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(ArrayType), src.ptr(),
                             flags, cleanup, reinterpret_cast<void**>(&ptr_))) {
      return true;
    }

    if ((flags & static_cast<uint32_t>(cast_flags::construct)) ||
        !(flags & static_cast<uint32_t>(cast_flags::convert))) {
      return false;
    }

    PyObject* obj_ptr = src.ptr();
    if (!obj_ptr || PyBytes_Check(obj_ptr) || PyUnicode_Check(obj_ptr) ||
        PyDict_Check(obj_ptr) || nanobind::inst_check(src)) {
      return false;
    }
    Py_ssize_t len = PyObject_Length(obj_ptr);
    if (len < 0) {
      PyErr_Clear();
      return false;
    }
    PyObject* iter = PyObject_GetIter(obj_ptr);
    if (!iter) {
      PyErr_Clear();
      return false;
    }
    auto& arr = holder_.emplace(static_cast<size_t>(len));
    T* dst = arr.data();
    size_t i = 0;
    while (PyObject* item = PyIter_Next(iter)) {
      make_caster<T> elem_caster;
      bool ok = elem_caster.from_python(
          item, static_cast<uint8_t>(cast_flags::convert), cleanup);
      Py_DECREF(item);
      if (!ok || i >= static_cast<size_t>(len)) {
        Py_DECREF(iter);
        PyErr_Clear();
        holder_.reset();
        return false;
      }
      dst[i++] = static_cast<T>(elem_caster.operator cast_t<T>());
    }
    Py_DECREF(iter);
    if (PyErr_Occurred() || i != static_cast<size_t>(len)) {
      PyErr_Clear();
      holder_.reset();
      return false;
    }
    ptr_ = &*holder_;
    return true;
  }

  template <typename T_>
  static handle from_cpp(T_&& src, rv_policy policy,
                         cleanup_list* cleanup) noexcept {
    return type_caster_base<ArrayType>::from_cpp(std::forward<T_>(src), policy,
                                                 cleanup);
  }

  template <typename T_>
  bool can_cast() const noexcept {
    return std::is_pointer_v<T_> || (ptr_ != nullptr);
  }

  operator ArrayType*() { return ptr_; }
  operator ArrayType&() { return *ptr_; }
  operator ArrayType&&() && { return std::move(*ptr_); }

 private:
  ArrayType* ptr_ = nullptr;
  std::optional<ArrayType> holder_;
};

template <typename T>
struct type_caster<PXR_NS::SdfHandle<T>> {
  NB_TYPE_CASTER(PXR_NS::SdfHandle<T>, const_name("Sdf.Spec"))

  bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept {
    if (src.is_none() &&
        !(flags & static_cast<uint8_t>(cast_flags::construct))) {
      value = PXR_NS::SdfHandle<T>();
      return true;
    }
    using NonConstT = std::remove_const_t<T>;
    NonConstT* ptr = nullptr;
    if (!NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(NonConstT), src.ptr(),
                              flags, cleanup, reinterpret_cast<void**>(&ptr)) ||
        !ptr) {
      return false;
    }
    value = PXR_NS::SdfHandle<T>(*ptr);
    return true;
  }

  static handle from_cpp(const PXR_NS::SdfHandle<T>& src, rv_policy,
                         cleanup_list*) noexcept {
    using NonConstT = std::remove_const_t<T>;
    return handle(PXR_NS::Sdf_PySpecDetail::_CreateHolder(
        typeid(NonConstT), src.GetSpec()));
  }
};

template <typename T>
struct type_caster<T, enable_if_t<std::is_same_v<T, PXR_NS::ArResolverContext>>>
    : type_caster_base_tag {
  using Value = T;
  template <typename T_>
  using Cast = precise_cast_t<T_>;
  static constexpr auto Name = const_name<T>();

  bool from_python(handle src, uint32_t flags, cleanup_list* cleanup) noexcept {
    if (!src.is_valid()) {
      return false;
    }
    if (src.is_none()) {
      if (flags & static_cast<uint32_t>(cast_flags::construct)) {
        return false;
      }
      holder_.emplace();
      ptr_ = &*holder_;
      return true;
    }
    if (NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(T), src.ptr(), flags,
                             cleanup, reinterpret_cast<void**>(&ptr_))) {
      return true;
    }
    if ((flags & static_cast<uint32_t>(cast_flags::construct)) ||
        !(flags & static_cast<uint32_t>(cast_flags::convert))) {
      return false;
    }
    T ctx;
    if (PXR_NS::Ar_ConvertFromPythonToResolverContext(src.ptr(), &ctx)) {
      holder_.emplace(std::move(ctx));
      ptr_ = &*holder_;
      return true;
    }
    return false;
  }

  template <typename T_>
  static handle from_cpp(T_&& src, rv_policy policy,
                         cleanup_list* cleanup) noexcept {
    return type_caster_base<T>::from_cpp(std::forward<T_>(src), policy,
                                         cleanup);
  }

  template <typename T_>
  bool can_cast() const noexcept {
    return std::is_pointer_v<T_> || (ptr_ != nullptr);
  }

  operator T*() { return ptr_; }
  operator T&() { return *ptr_; }
  operator T&&() && { return std::move(*ptr_); }

 private:
  T* ptr_ = nullptr;
  std::optional<T> holder_;
};

template <>
struct type_caster<std::shared_ptr<PXR_NS::ArAsset>> {
  NB_TYPE_CASTER(std::shared_ptr<PXR_NS::ArAsset>, const_name("Ar.Ar_PyAsset"))

  bool from_python(handle src, uint8_t flags, cleanup_list* cleanup) noexcept {
    if (src.is_none()) {
      value.reset();
      return true;
    }
    PXR_NS::Ar_PyAsset* ptr = nullptr;
    if (!NB_CALL(nb_type_get)(NB_CTX_C(cleanup), &typeid(PXR_NS::Ar_PyAsset),
                              src.ptr(), flags, cleanup,
                              reinterpret_cast<void**>(&ptr)) ||
        !ptr) {
      return false;
    }
    value = ptr->_asset;
    return true;
  }

  static handle from_cpp(const std::shared_ptr<PXR_NS::ArAsset>& src,
                         rv_policy, cleanup_list* cleanup) noexcept {
    if (!src) {
      return none().release();
    }
    return type_caster_base<PXR_NS::Ar_PyAsset>::from_cpp(
        PXR_NS::Ar_PyAsset(src), rv_policy::move, cleanup);
  }
};

template <>
struct type_caster<PXR_NS::TfToken> {
  NB_TYPE_CASTER(PXR_NS::TfToken, const_name("str"))

  bool from_python(handle src, uint8_t, cleanup_list*) noexcept {
    Py_ssize_t size = 0;
    const char* str = PyUnicode_AsUTF8AndSize(src.ptr(), &size);
    if (!str) {
      PyErr_Clear();
      return false;
    }
    value = PXR_NS::TfToken(std::string(str, size));
    return true;
  }

  static handle from_cpp(const PXR_NS::TfToken& src, rv_policy,
                         cleanup_list*) noexcept {
    return PyUnicode_FromStringAndSize(src.GetText(), src.size());
  }
};

template <typename T>
struct type_caster<T, enable_if_t<std::is_same_v<T, PXR_NS::GfHalf>>> {
  NB_TYPE_CASTER(T, const_name("float"))

  bool from_python(handle src, uint8_t, cleanup_list*) noexcept {
    PyObject* flt = PyNumber_Float(src.ptr());
    if (!flt) {
      PyErr_Clear();
      return false;
    }
    double d = PyFloat_AsDouble(flt);
    Py_DECREF(flt);
    if (PyErr_Occurred()) {
      PyErr_Clear();
      return false;
    }
    value = T(static_cast<float>(d));
    return true;
  }

  static handle from_cpp(T src, rv_policy,
                         cleanup_list*) noexcept {
    return PyFloat_FromDouble(src);
  }
};

template <typename T>
struct has_arg_defaults<PXR_NS::TfWeakPtr<T>> : std::true_type {};
template <typename T>
struct has_arg_defaults<PXR_NS::TfRefPtr<T>> : std::true_type {};
template <typename T>
struct has_arg_defaults<PXR_NS::SdfHandle<T>> : std::true_type {};
template <>
struct has_arg_defaults<PXR_NS::TfAnyWeakPtr> : std::true_type {};
template <>
struct has_arg_defaults<PXR_NS::TfPyObjWrapper> : std::true_type {};
template <>
struct has_arg_defaults<PXR_NS::ArResolverContext> : std::true_type {};
template <>
struct has_arg_defaults<PXR_NS::VtValue> : std::true_type {};

}  // namespace detail
}  // namespace nanobind

PXR_NAMESPACE_OPEN_SCOPE

template <typename T>
TfPyObjWrapper Tf_PyObjFromCpp(T const& val) {
  TfPyLock lock;
  if constexpr (std::is_pointer_v<T> &&
                std::is_function_v<std::remove_pointer_t<T>>) {
    PyErr_Format(PyExc_TypeError,
                 "No to_python (by-value) converter found for C++ type: %s",
                 ArchGetDemangled(typeid(T)).c_str());
    return TfPyObjWrapper();
  } else {
    nanobind::rv_policy policy =
        std::is_pointer_v<T>
            ? nanobind::rv_policy(nanobind::rv_policy::reference)
            : nanobind::rv_policy(nanobind::rv_policy::automatic);
    nanobind::detail::cleanup_list cleanup(nullptr);
    nanobind::handle h =
        nanobind::detail::make_caster<T>::from_cpp(val, policy, &cleanup);
    cleanup.release();
    if (!h.is_valid()) {
      if (!PyErr_Occurred()) {
        PyErr_Format(PyExc_TypeError,
                     "No to_python (by-value) converter found for C++ type: %s",
                     ArchGetDemangled(typeid(T)).c_str());
      }
      return TfPyObjWrapper();
    }
    return TfPyObjWrapper(nanobind::steal<nanobind::object>(h));
  }
}

template <typename T, bool IsScopedEnum = !std::is_convertible<T, int>::value>
void TfNbWrapEnum(nanobind::handle scope,
                  std::string const& name = std::string()) {
  (void)TfPyWrapEnum<T, IsScopedEnum>(scope, name);
}

template <typename T, bool IsScopedEnum = !std::is_convertible<T, int>::value>
void TfNbWrapEnum(std::string const& name = std::string()) {
  (void)TfPyWrapEnum<T, IsScopedEnum>(name);
}

template <typename Seq>
nanobind::tuple TfNbSequenceToTuple(Seq const& seq) {
  nanobind::list l;
  for (auto const& item : seq) {
    l.append(nanobind::cast(item));
  }
  return nanobind::tuple(l);
}

template <typename Seq>
nanobind::list TfNbSequenceToList(Seq const& seq) {
  nanobind::list l;
  for (auto const& item : seq) {
    l.append(nanobind::cast(item));
  }
  return l;
}

PXR_NAMESPACE_CLOSE_SCOPE

#include "pxr/base/tf/pyFunction.h"

#endif  // PXR_BASE_TF_PY_NANOBIND_BRIDGE_H
