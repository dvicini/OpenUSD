//
// Copyright 2018 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//

#include "pxr/pxr.h"
#include "pxr/usd/ar/packageUtils.h"
#include "pxr/base/tf/pyResultConversions.h"


PXR_NAMESPACE_USING_DIRECTIVE


#include "pxr/base/tf/pyNanobindBridge.h"

void
wrapPackageUtils(nanobind::module_& m)
{
    namespace nb = nanobind;
    m.def("IsPackageRelativePath",
          &ArIsPackageRelativePath, nb::arg("path"));

    m.def("JoinPackageRelativePath",
          (std::string(*)(const std::vector<std::string>&))
              &ArJoinPackageRelativePath,
          nb::arg("paths"));

    m.def("JoinPackageRelativePath",
          (std::string(*)(const std::pair<std::string, std::string>&))
              &ArJoinPackageRelativePath,
          nb::arg("paths"));

    m.def("JoinPackageRelativePath",
          (std::string(*)(const std::string&, const std::string&))
              &ArJoinPackageRelativePath,
          nb::arg("packagePath"), nb::arg("packagedPath"));

    m.def("SplitPackageRelativePathOuter",
          &ArSplitPackageRelativePathOuter, nb::arg("path"));

    m.def("SplitPackageRelativePathInner",
          &ArSplitPackageRelativePathInner, nb::arg("path"));
}
