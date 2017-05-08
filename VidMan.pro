TEMPLATE = subdirs
SUBDIRS = src app

app.depends = src

CONFIG(debug, debug|release) {
    SUBDIRS += tests
    tests.depends = src
}

RESOURCES += resource.qrc
