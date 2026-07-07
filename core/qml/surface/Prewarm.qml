import QtQuick

// Minimal scene used only to initialize Qt Quick before the main window's first
// show, so the top-level is created in composited mode and never recreated (and
// flashed) when the first real surface view appears.
Item {}
