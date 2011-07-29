# change debug to release and recompile when working
TEMPLATE        = app
CONFIG          += qt opengl debug thread
QT		+= qt3support svg
CONFIG          += console
HEADERS         = \
                SodController.h \
                distanceViewer.h \
                distanceMapper.h \
                pointDrawer.h \
                stressPlotter.h \
                BackgroundDrawer.h \
                node_set.h \
                posInfo.h \
                ../customWidgets/clineEdit.h \
                ../imageBuilder/f_parameter.h
SOURCES         = \
                SodController.cpp \
                distanceViewer.cpp \
                distanceMapper.cpp \
                pointDrawer.cpp \
                stressPlotter.cpp \
                BackgroundDrawer.cpp \
                node_set.cpp \
                ../imageBuilder/f_parameter.cpp \
                main.cpp
TARGET          = dimSqueezer
