let ui_type = variable::get("ui_type");

if ui_type != "imgui" {
    if file::exists("ci/init_imgui.sh") {
        file::delete("ci/init_imgui.sh");
    }
    if file::exists("ci/init_vulkan_headers.sh") {
        file::delete("ci/init_vulkan_headers.sh");
    }
} else {
    let imgui_backend = if variable::is_set("imgui_backend") {
        variable::get("imgui_backend")
    } else {
        "glfw_vulkan"
    };

    if imgui_backend != "glfw_vulkan" && file::exists("ci/init_vulkan_headers.sh") {
        file::delete("ci/init_vulkan_headers.sh");
    }
}

if file::exists("ci/post_gen_cleanup.rhai") {
    file::delete("ci/post_gen_cleanup.rhai");
}
