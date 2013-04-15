# change debug to release and recompile when working
TEMPLATE        = app
CONFIG          += qt opengl debug thread
QT		+= qt3support svg
CONFIG          += console
INCLUDEPATH	+= /usr/local/cuda/include
LIBS		+= -lOpenCL
HEADERS         = \
                SodController.h \
                distanceViewer.h \
                distanceMapper.h \
		VectorAdjustThread.h \
                pointDrawer.h \
                stressPlotter.h \
                BackgroundDrawer.h \
                node_set.h \
		Annotation.h \
                posInfo.h \
		DensityPlot.h \
		ColorScale.h \
		../open_cl/oCL_base.h \
		../open_cl/clError.h \
		oCL_DistanceMapper.h \
		oCL_DistanceMapperManager.h \
                ../customWidgets/clineEdit.h \
                ../imageBuilder/f_parameter.h
SOURCES         = \
                SodController.cpp \
                distanceViewer.cpp \
                distanceMapper.cpp \
		VectorAdjustThread.cpp \
                pointDrawer.cpp \
                stressPlotter.cpp \
                BackgroundDrawer.cpp \
                node_set.cpp \
		Annotation.cpp \
		DensityPlot.cpp \
		ColorScale.cpp \
		../open_cl/oCL_base.cpp \
		../open_cl/clError.cpp \
		oCL_DistanceMapper.cpp \
		oCL_DistanceMapperManager.cpp \
                ../imageBuilder/f_parameter.cpp \
                main.cpp
TARGET          = dimSqueezer
