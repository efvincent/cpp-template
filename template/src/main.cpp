{% if ui_type == "plain" %}
#include <stdio.h>

int main() {
  puts("hello from {{project-name}}");
  return 0;
}
{% elsif ui_type == "ncurses" %}
#include <ncurses.h>

int main() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  mvprintw(1, 2, "Hello from {{project-name}} (ncurses)");
  mvprintw(3, 2, "Press any key to exit...");
  refresh();
  getch();

  endwin();
  return 0;
}
{% elsif ui_type == "imgui" and imgui_backend == "glfw_vulkan" %}
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

static VkAllocationCallbacks* g_allocator = nullptr;
static VkInstance g_instance = VK_NULL_HANDLE;
static VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static uint32_t g_queue_family = (uint32_t)-1;
static VkQueue g_queue = VK_NULL_HANDLE;
static VkDescriptorPool g_descriptor_pool = VK_NULL_HANDLE;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;

static int g_min_image_count = 2;
static bool g_swapchain_rebuild = false;
static ImGui_ImplVulkanH_Window g_main_window_data;

static void check_vk_result(VkResult err) {
  if (err == VK_SUCCESS) {
    return;
  }
  fprintf(stderr, "Vulkan error: %d\n", err);
  if (err < 0) {
    abort();
  }
}

static bool select_physical_device_and_queue(VkSurfaceKHR surface) {
  uint32_t gpu_count = 0;
  vkEnumeratePhysicalDevices(g_instance, &gpu_count, nullptr);
  if (gpu_count == 0) {
    fprintf(stderr, "No Vulkan physical devices found.\n");
    return false;
  }

  VkPhysicalDevice gpus[16] = {};
  if (gpu_count > 16) {
    gpu_count = 16;
  }
  vkEnumeratePhysicalDevices(g_instance, &gpu_count, gpus);

  for (uint32_t gpu_i = 0; gpu_i < gpu_count; ++gpu_i) {
    VkPhysicalDevice gpu = gpus[gpu_i];
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, nullptr);
    VkQueueFamilyProperties queues[32] = {};
    if (queue_count > 32) {
      queue_count = 32;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queues);

    for (uint32_t q = 0; q < queue_count; ++q) {
      VkBool32 present_support = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu, q, surface, &present_support);
      if ((queues[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
        g_physical_device = gpu;
        g_queue_family = q;
        return true;
      }
    }
  }

  fprintf(stderr, "No queue family supports graphics + present.\n");
  return false;
}

static bool create_vulkan_device() {
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_info = {};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = g_queue_family;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &queue_priority;

  const char* device_extensions[] = {"VK_KHR_swapchain"};

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = 1;
  create_info.pQueueCreateInfos = &queue_info;
  create_info.enabledExtensionCount = 1;
  create_info.ppEnabledExtensionNames = device_extensions;

  VkResult err = vkCreateDevice(g_physical_device, &create_info, g_allocator, &g_device);
  if (err != VK_SUCCESS) {
    check_vk_result(err);
    return false;
  }

  vkGetDeviceQueue(g_device, g_queue_family, 0, &g_queue);
  return true;
}

static bool create_descriptor_pool() {
  VkDescriptorPoolSize pool_sizes[] = {
    {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
    {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
    {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
  };

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  pool_info.maxSets = 1000 * static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
  pool_info.poolSizeCount = static_cast<uint32_t>(sizeof(pool_sizes) / sizeof(pool_sizes[0]));
  pool_info.pPoolSizes = pool_sizes;

  VkResult err = vkCreateDescriptorPool(g_device, &pool_info, g_allocator, &g_descriptor_pool);
  if (err != VK_SUCCESS) {
    check_vk_result(err);
    return false;
  }
  return true;
}

static void frame_render(ImGui_ImplVulkanH_Window* wd, ImDrawData* draw_data) {
  VkResult err;
  VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
  VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

  err = vkAcquireNextImageKHR(g_device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_swapchain_rebuild = true;
    return;
  }
  check_vk_result(err);

  ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
  {
    err = vkWaitForFences(g_device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
    check_vk_result(err);
    err = vkResetFences(g_device, 1, &fd->Fence);
    check_vk_result(err);

    err = vkResetCommandPool(g_device, fd->CommandPool, 0);
    check_vk_result(err);
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
    check_vk_result(err);
  }

  {
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = wd->RenderPass;
    info.framebuffer = fd->Framebuffer;
    info.renderArea.extent.width = wd->Width;
    info.renderArea.extent.height = wd->Height;
    info.clearValueCount = 1;
    VkClearValue clear = {};
    clear.color.float32[0] = 0.10f;
    clear.color.float32[1] = 0.10f;
    clear.color.float32[2] = 0.10f;
    clear.color.float32[3] = 1.00f;
    info.pClearValues = &clear;
    vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
  }

  ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
  vkCmdEndRenderPass(fd->CommandBuffer);

  VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &image_acquired_semaphore;
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &fd->CommandBuffer;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &render_complete_semaphore;

  VkResult end_err = vkEndCommandBuffer(fd->CommandBuffer);
  check_vk_result(end_err);
  VkResult submit_err = vkQueueSubmit(g_queue, 1, &submit_info, fd->Fence);
  check_vk_result(submit_err);
}

static void frame_present(ImGui_ImplVulkanH_Window* wd) {
  if (g_swapchain_rebuild) {
    return;
  }

  VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
  VkPresentInfoKHR info = {};
  info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  info.waitSemaphoreCount = 1;
  info.pWaitSemaphores = &render_complete_semaphore;
  info.swapchainCount = 1;
  info.pSwapchains = &wd->Swapchain;
  info.pImageIndices = &wd->FrameIndex;

  VkResult err = vkQueuePresentKHR(g_queue, &info);
  if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
    g_swapchain_rebuild = true;
    return;
  }
  check_vk_result(err);
  wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount;
}

int main() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW.\n");
    return 1;
  }
  if (!glfwVulkanSupported()) {
    fprintf(stderr, "GLFW reports Vulkan is not supported.\n");
    glfwTerminate();
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "{{project-name}} (ImGui + Vulkan)", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    glfwTerminate();
    return 1;
  }

  uint32_t extension_count = 0;
  const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "{{project-name}}";
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "none";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = extension_count;
  create_info.ppEnabledExtensionNames = extensions;

  VkResult instance_err = vkCreateInstance(&create_info, g_allocator, &g_instance);
  if (instance_err != VK_SUCCESS) {
    check_vk_result(instance_err);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  VkResult surface_err = glfwCreateWindowSurface(g_instance, window, g_allocator, &g_surface);
  if (surface_err != VK_SUCCESS) {
    check_vk_result(surface_err);
    vkDestroyInstance(g_instance, g_allocator);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  if (!select_physical_device_and_queue(g_surface)) {
    vkDestroySurfaceKHR(g_instance, g_surface, g_allocator);
    vkDestroyInstance(g_instance, g_allocator);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  if (!create_vulkan_device()) {
    vkDestroySurfaceKHR(g_instance, g_surface, g_allocator);
    vkDestroyInstance(g_instance, g_allocator);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  if (!create_descriptor_pool()) {
    vkDestroyDevice(g_device, g_allocator);
    vkDestroySurfaceKHR(g_instance, g_surface, g_allocator);
    vkDestroyInstance(g_instance, g_allocator);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  int width = 0;
  int height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  ImGui_ImplVulkanH_Window* wd = &g_main_window_data;
  wd->Surface = g_surface;
  const VkFormat request_surface_image_format[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_B8G8R8_UNORM,
    VK_FORMAT_R8G8B8_UNORM
  };
  const VkColorSpaceKHR request_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
    g_physical_device,
    wd->Surface,
    request_surface_image_format,
    static_cast<int>(sizeof(request_surface_image_format) / sizeof(request_surface_image_format[0])),
    request_surface_color_space
  );
  const VkPresentModeKHR request_present_modes[] = {
    VK_PRESENT_MODE_MAILBOX_KHR,
    VK_PRESENT_MODE_IMMEDIATE_KHR,
    VK_PRESENT_MODE_FIFO_KHR
  };
  wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
    g_physical_device,
    wd->Surface,
    &request_present_modes[0],
    static_cast<int>(sizeof(request_present_modes) / sizeof(request_present_modes[0]))
  );
  ImGui_ImplVulkanH_CreateOrResizeWindow(
    g_instance,
    g_physical_device,
    g_device,
    wd,
    g_queue_family,
    g_allocator,
    width,
    height,
    g_min_image_count,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
  );

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForVulkan(window, true);
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.ApiVersion = VK_API_VERSION_1_0;
  init_info.Instance = g_instance;
  init_info.PhysicalDevice = g_physical_device;
  init_info.Device = g_device;
  init_info.QueueFamily = g_queue_family;
  init_info.Queue = g_queue;
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = g_descriptor_pool;
  init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
  init_info.PipelineInfoMain.Subpass = 0;
  init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.MinImageCount = g_min_image_count;
  init_info.ImageCount = wd->ImageCount;
  init_info.Allocator = g_allocator;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    if (g_swapchain_rebuild) {
      glfwGetFramebufferSize(window, &width, &height);
      if (width > 0 && height > 0) {
        ImGui_ImplVulkan_SetMinImageCount(g_min_image_count);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
          g_instance,
          g_physical_device,
          g_device,
          wd,
          g_queue_family,
          g_allocator,
          width,
          height,
          g_min_image_count,
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        );
        wd->FrameIndex = 0;
        g_swapchain_rebuild = false;
      }
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello");
    ImGui::Text("Hello from {{project-name}} (ImGui + Vulkan)");
    ImGui::Text("Press close on the window to exit.");
    ImGui::End();

    ImGui::Render();
    frame_render(wd, ImGui::GetDrawData());
    frame_present(wd);
  }

  VkResult wait_err = vkDeviceWaitIdle(g_device);
  check_vk_result(wait_err);

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  ImGui_ImplVulkanH_DestroyWindow(g_instance, g_device, wd, g_allocator);
  vkDestroyDescriptorPool(g_device, g_descriptor_pool, g_allocator);
  vkDestroyDevice(g_device, g_allocator);
  vkDestroySurfaceKHR(g_instance, g_surface, g_allocator);
  vkDestroyInstance(g_instance, g_allocator);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
{% elsif ui_type == "imgui" and imgui_backend == "glfw_opengl3" %}
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

int main() {
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW.\n");
    return 1;
  }

#if defined(__APPLE__)
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  GLFWwindow* window = glfwCreateWindow(1280, 720, "{{project-name}} (ImGui + OpenGL3)", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Hello");
    ImGui::Text("Hello from {{project-name}} (ImGui + OpenGL3)");
    ImGui::Text("Press close on the window to exit.");
    ImGui::End();

    ImGui::Render();
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.10f, 0.10f, 0.10f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
{% endif %}
