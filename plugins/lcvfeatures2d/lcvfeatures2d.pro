TEMPLATE = lib
TARGET   = lcvfeatures2d
QT      += qml quick
CONFIG  += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)
uri = plugins.lcvfeatures2d

OTHER_FILES = qmldir

DEFINES += Q_LCV
DEFINES += Q_LCVFEATURES2D_LIB


include($$PWD/../use_lcvcore.pri)
include($$PWD/../use_liblive.pri)
include($$PWD/src/lcvfeatures2d.pri)
include($$PWD/include/lcvfeatures2dheaders.pri)
include($$PWD/../../3rdparty/opencvconfig.pro)

deployOpenCV()

# Destination

win32:CONFIG(debug, debug|release): DLLDESTDIR = $$quote($$OUT_PWD/../../application/debug/plugins/lcvfeatures2d)
else:win32:CONFIG(release, debug|release): DLLDESTDIR = $$quote($$OUT_PWD/../../application/release/plugins/lcvfeatures2d)
else:unix: TARGET = $$quote($$OUT_PWD/../../application/plugins/lcvfeatures2d)

# Qml

OTHER_FILES += \
    $$PWD/qml/KeypointListView.qml \
    $$PWD/qml/FeatureDetectorSelection.qml \
    $$PWD/qml/SelectionArea.qml \
    $$PWD/qml/SelectionWindow.qml \
    $$PWD/qml/FeatureObjectList.qml \
    $$PWD/qml/FeatureObjectMatch.qml \
    $$PWD/qml/DescriptorExtractorSelection.qml
    $$PWD/qml/qmldir

# Deploy qml

PLUGIN_DEPLOY_FROM = $$PWD/qml
win32:CONFIG(debug, debug|release): PLUGIN_DEPLOY_TO = $$OUT_PWD/../../application/debug/plugins/lcvfeatures2d
else:win32:CONFIG(release, debug|release): PLUGIN_DEPLOY_TO = $$OUT_PWD/../../application/release/plugins/lcvfeatures2d
else:unix: PLUGIN_DEPLOY_TO = $$OUT_PWD/../../application/plugins/lcvfeatures2d

win32:PLUGIN_DEPLOY_TO ~= s,/,\\,g
win32:PLUGIN_DEPLOY_FROM ~= s,/,\\,g

plugincopy.commands = $(COPY_DIR) \"$$PLUGIN_DEPLOY_FROM\" \"$$PLUGIN_DEPLOY_TO\"

first.depends = $(first) plugincopy
export(first.depends)
export(plugincopy.commands)

QMAKE_EXTRA_TARGETS += first plugincopy
