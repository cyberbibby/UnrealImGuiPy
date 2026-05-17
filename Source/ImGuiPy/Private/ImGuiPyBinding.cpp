// Copyright (c) 2023 Bibby. All Rights Reserved.

#ifdef IMGUI_BUNDLE_PYTHON_API
#include "ImGuiPyBinding.h"
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
#pragma warning (disable : 4883)
#include "pybind11/pybind11.h"
#include "bindings/pybind_imgui.cpp"
#include "bindings/imgui_pywrappers.cpp"
#include "bindings/pybind_imgui_color_text_edit.cpp"
#include "bindings/pybind_imgui_toggle.cpp"
#include "bindings/pybind_implot.cpp"
#include "bindings/pybind_imspinner.cpp"
#pragma warning (pop)
#pragma pop_macro("dynamic_cast")
#pragma pop_macro("ensure")
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END

namespace py = pybind11;

void PyInitModuleImguiBundle(py::module& m)
{
	m.attr("__version__") = "1.92.801";

	m.def("compilation_time", []()
	{
		return std::string("imgui_bundle, compiled on ") + __DATE__ + " at " + __TIME__;
	});

	// imgui and its submodules
	auto module_imgui = m.def_submodule("imgui");
	py_init_module_imgui_main(module_imgui);

	auto module_implot = m.def_submodule("implot");
	py_init_module_implot(module_implot);

	auto module_imgui_color_text_edit = m.def_submodule("imgui_color_text_edit");
	py_init_module_imgui_color_text_edit(module_imgui_color_text_edit);

	auto module_imgui_toggle = m.def_submodule("imgui_toggle");
	py_init_module_imgui_toggle(module_imgui_toggle);

	auto module_imspinner = m.def_submodule("imspinner");
	py_init_module_imspinner(module_imspinner);
}

#endif
