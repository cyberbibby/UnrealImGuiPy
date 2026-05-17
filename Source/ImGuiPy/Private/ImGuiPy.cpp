// Copyright (c) 2024 Bibby. All Rights Reserved.

#include "ImGuiPy.h"

#define LOCTEXT_NAMESPACE "FImGuiPyModule"

#ifdef IMGUI_BUNDLE_PYTHON_API
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
#include <pybind11/embed.h>
#pragma warning (pop)
#pragma pop_macro("dynamic_cast")
#pragma pop_macro("ensure")
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END
#include "ImGuiPyBinding.h"

namespace py = pybind11;

// This builds the native python module `_imgui_bundle`
// it will be wrapped in a standard python module `imgui_bundle`
PYBIND11_EMBEDDED_MODULE(_imgui_bundle, m)
{
	m.attr("__info__") = "Embedded in NEPy";

	PyInitModuleImguiBundle(m);
}
#endif

void FImGuiPyModule::StartupModule()
{

}

void FImGuiPyModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiPyModule, ImGuiPy)
