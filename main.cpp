#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Window.H>
#include <FL/x.H>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <cstring>
#include <ctime>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wordexp.h>

#include "get_screen.h"

pid_t recordingPid = 0;

Fl_Light_Button *gRecordBtn = nullptr;
bool isBlinking = false;

Fl_Check_Button *gOntopBtn = nullptr;

// Helper function to expand home directory
char *wordexp_helper(const char *filename) {
  wordexp_t exp_result;
  char *expanded_filename = nullptr;

  if (wordexp(filename, &exp_result, 0) == 0) {
    expanded_filename = strdup(exp_result.we_wordv[0]);
    wordfree(&exp_result);
  } else {
    // Fallback to original filename
    expanded_filename = strdup(filename);
  }

  return expanded_filename;
}

// Helper function to update the button label and color
void update_button_state(const char *label, Fl_Color color) {
  if (gRecordBtn) {
    gRecordBtn->label(label);
    gRecordBtn->color(color);
    gRecordBtn->redraw();
  }
}

void stop_recording(Fl_Widget *, void *) {
  if (recordingPid > 0) {
    kill(recordingPid, SIGTERM);

    int status;
    waitpid(recordingPid, &status, 0);

    recordingPid = 0;

    update_button_state(" Start Rec", FL_BACKGROUND_COLOR);
  }
}

void start_recording(Fl_Widget *, void *) {
  try {
    auto [screen_width, screen_height] = get_screen_size();

    recordingPid = fork();

    if (recordingPid == 0) {
      // Child process
      time_t now = time(0);
      char filename[128];
      strftime(filename, sizeof(filename),
               "$HOME/Videos/screen_recording_%Y-%m-%d_%H-%M-%S.mp4",
               localtime(&now));

      // Expand the home directory
      char *expanded_filename = wordexp_helper(filename);

      char width_str[10], height_str[10];
      snprintf(width_str, sizeof(width_str), "%d", screen_width);
      snprintf(height_str, sizeof(height_str), "%d", screen_height);

      char resolution[32];
      snprintf(resolution, sizeof(resolution), "%sx%s", width_str, height_str);

      execlp("ffmpeg", "ffmpeg", "-f", "x11grab", "-y", "-framerate", "30",
             "-s", resolution, "-i", ":0.0", "-c:v", "libx264", "-preset",
             "superfast", "-crf", "18", "-flush_packets", "1",
             expanded_filename, NULL);

      // If execlp fails
      free(expanded_filename);
      perror("Failed to start ffmpeg");
      exit(1);
    } else if (recordingPid > 0) {
      update_button_state(" Stop Rec", FL_BACKGROUND_COLOR);
    } else {
      perror("Failed to fork");
      update_button_state(" Start Rec", FL_RED);
    }
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    update_button_state(" Start Rec", FL_RED);
  }
}

void blink_record_button(void *) {
  if (gRecordBtn) {
    gRecordBtn->value(!gRecordBtn->value());
    gRecordBtn->redraw();
  }

  if (isBlinking) {
    Fl::repeat_timeout(0.5, blink_record_button);
  }
}

void start_blinking() {
  if (!isBlinking) {
    isBlinking = true;
    Fl::add_timeout(0.5, blink_record_button);
  }
}

void stop_blinking() {
  isBlinking = false;
  Fl::remove_timeout(blink_record_button);
  if (gRecordBtn) {
    gRecordBtn->value(0);
    gRecordBtn->redraw();
  }
}

void toggle_always_on_top(Fl_Widget *w, void *window_ptr) {
  Fl_Window *window = static_cast<Fl_Window *>(window_ptr);
  static Display *display = XOpenDisplay(nullptr);

  if (!display) {
    std::cerr << "Failed to open X display" << std::endl;
    return;
  }

  Window xid = fl_xid(window); // Get the X11 window ID

  // Define the atom for the always-on-top property
  Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
  Atom wm_state_above = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);

  XEvent xev;
  memset(&xev, 0, sizeof(xev));

  xev.type = ClientMessage;
  xev.xclient.window = xid;
  xev.xclient.message_type = wm_state;
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = static_cast<Fl_Check_Button *>(w)->value()
                              ? 1
                              : 0; // 1 to add, 0 to remove
  xev.xclient.data.l[1] = wm_state_above;
  xev.xclient.data.l[2] = 0;

  XSendEvent(display, DefaultRootWindow(display), False,
             SubstructureNotifyMask | SubstructureRedirectMask, &xev);

  XFlush(display);
}

int main(int argc, char **argv) {
  Fl::scheme("oxy");
  Fl_Window *window = new Fl_Window(120, 75, "SRX");
  window->begin();

  gRecordBtn = new Fl_Light_Button(10, 10, 100, 30, " Start Rec");
  gRecordBtn->selection_color(fl_rgb_color(35, 209, 139));
  gRecordBtn->callback([](Fl_Widget *w, void *) {
    Fl_Light_Button *btn = (Fl_Light_Button *)w;
    if (isBlinking) {
      stop_recording(w, nullptr);
      stop_blinking();
    }
    if (btn->value()) {
      start_recording(w, nullptr);
      start_blinking();
    } else {
      return;
    }
    btn->redraw();
    Fl::flush();
  });

  gOntopBtn = new Fl_Check_Button(10, 50, 100, 10, " On top");
  gOntopBtn->callback(toggle_always_on_top, window);
  gOntopBtn->redraw();

  window->end();
  window->show(argc, argv);

  return Fl::run();
}
