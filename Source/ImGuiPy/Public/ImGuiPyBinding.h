// Copyright (c) 2023 Bibby. All Rights Reserved.

#pragma once

THIRD_PARTY_INCLUDES_START
#pragma push_macro("check")
#undef check
#pragma push_macro("ensure")
#undef ensure
#pragma push_macro("dynamic_cast")
#undef dynamic_cast
#pragma warning (push)
#pragma warning (disable : 4191)
#pragma warning (disable : 4686)
#include "pybind11/pybind11.h"
#pragma warning (pop)
#pragma pop_macro("dynamic_cast")
#pragma pop_macro("ensure")
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END

namespace py = pybind11;

IMGUIPY_API void PyInitModuleImguiBundle(py::module& m);
