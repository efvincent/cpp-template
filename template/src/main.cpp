{% if app_type == "plain" %}
import app;

int main() {
  return app::run();
}
{% elsif app_type == "ncurses" %}
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
{% elsif app_type == "imgui" %}
#include <iostream>
#include "imgui.h"

int main() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2(640.0f, 480.0f);
  io.DeltaTime = 1.0f / 60.0f;

  // No platform/renderer backend is initialized in this template variant,
  // so bake the default font atlas explicitly before NewFrame().
  io.Fonts->AddFontDefault();
  unsigned char* font_pixels = nullptr;
  int font_width = 0;
  int font_height = 0;
  io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);

  ImGui::NewFrame();
  ImGui::Begin("Hello");
  ImGui::Text("Hello from {{project-name}} (ImGui)");
  ImGui::End();
  ImGui::Render();

  std::cout << "Built one ImGui frame successfully." << std::endl;
  ImGui::DestroyContext();
  return 0;
}
{% endif %}
