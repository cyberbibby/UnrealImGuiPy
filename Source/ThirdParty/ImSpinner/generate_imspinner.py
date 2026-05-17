# Part of ImGui Bundle - MIT License - Copyright (c) 2022-2023 Pascal Thomet - https://github.com/pthom/imgui_bundle
import os
import re
from enum import Enum

import litgen
from codemanip.code_utils import join_string_by_pipe_char
from srcmlcpp.scrml_warning_settings import WarningType


THIS_DIR = os.path.dirname(__file__)
PYDEF_DIR = THIS_DIR
STUB_DIR = f"{THIS_DIR}/../../../Content/Lib/imgui_bundle/"
CPP_HEADERS_DIR = f"{THIS_DIR}/Public/"


class ImguiOptionsType(Enum):
    imgui_h = 1
    imgui_stdlib_h = 2
    imgui_internal_h = 3


def _preprocess_imgui_code(code: str) -> str:
    new_code = code

    new_code = new_code.replace(
        "const float a =  a_min + ((float)i / (float)num_segments) * (a_max - a_min);",
        "// const float a =  a_min + ((float)i / (float)num_segments) * (a_max - a_min);"
    )
    new_code = new_code.replace(
        "const float rspeed = a + (float)ImGui::GetTime() * speed;",
        "// const float rspeed = a + (float)ImGui::GetTime() * speed;"
    )
    new_code = new_code.replace(
        "const float x = (scale(16) * ImPow(ImSin(a), 3));",
        "// const float x = (scale(16) * ImPow(ImSin(a), 3));"
    )
    new_code = new_code.replace(
        "const float y = -1.f * (scale(13) * ImCos(a) - scale(5) * ImCos(2 * a) - scale(2) * ImCos(3 * a) - ImCos(4 * a));",
        "// const float y = -1.f * (scale(13) * ImCos(a) - scale(5) * ImCos(2 * a) - scale(2) * ImCos(3 * a) - ImCos(4 * a));"
    )
    new_code = new_code.replace(
        "const float a = start + (i * (PI_2 / (num_segments - 1)));",
        "// const float a = start + (i * (PI_2 / (num_segments - 1)));"
    )

    return new_code


def main():
    # print("autogenerate_imspinner")
    input_cpp_header = f"{CPP_HEADERS_DIR}/imspinner.h"
    output_cpp_pydef_file = f"{THIS_DIR}/Private/bindings/pybind_imspinner.cpp"
    output_stub_pyi_file = f"{STUB_DIR}/imspinner.pyi"

    # Configure options
    options = litgen.LitgenOptions()
    options.namespace_root__regex = "^ImSpinner$"
    options.fn_exclude_by_name__regex = join_string_by_pipe_char(
        [
            r"demoSpinners",
            r"color_alpha",
            r"^Spinner$",
        ]
    )
    options.srcmlcpp_options.ignored_warnings = [
        WarningType.SrcmlcppIgnoreElement,
        WarningType.LitgenIgnoreElement
    ]
    options.srcmlcpp_options.ignored_warning_parts = [
        "Ignoring template function",
        "unhandled tag template"
    ]
    options.srcmlcpp_options.code_preprocess_function = _preprocess_imgui_code
    options.fn_template_options.add_ignore(r"PI_DIV")
    options.fn_template_options.add_ignore(r"PI_2_DIV")
    # options.python_run_black_formatter = True

    litgen.write_generated_code_for_file(
        options,
        input_cpp_header_file=input_cpp_header,
        output_cpp_pydef_file=output_cpp_pydef_file,
        output_stub_pyi_file=output_stub_pyi_file,
        omit_boxed_types=True,
    )


if __name__ == "__main__":
    main()
