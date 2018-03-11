ddui
====

Direct-drawing user interface tools for C++

## Background

I write a lot of cross-platform immediate-mode GUIs. The nature of these
applications vary dramatically, but they nonetheless share a handful of common
traits. I've bundled the most crucial common traits into this library.

For window management, ddui uses glfw. `ddui::app` doesn't support multiple
windows despite glfw's support for it. If you need multiple windows, you're
free to call glfw directly. My aim was not to reimplement the entire glfw API.

For drawing, ddui uses nanovg.


