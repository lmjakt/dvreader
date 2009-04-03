TEMPLATE        = app
CONFIG          += qt opengl release  thread
QT		+= qt3support 
CONFIG          += console
LIBS		+= -lrt
HEADERS		= \
		dvReader.h \
		deltaViewer.h \
		dataStructs.h \
		jpgView/jpgView.h \
		opengl/glImage.h \
		distchooser/distChooser.h \
		distchooser/tabWidget.h \
		colorChooser.h \
		parameterChooser.h \
		colorMapper.h \
		linGraph/slider.h \
		linGraph/plotWidget.h \
		spotFinder/spotWindow.h \
		spotFinder/channelWidget.h \
		spotFinder/modelWidget.h \
		spotFinder/nucleusWidget.h \
		spotFinder/setWidget.h \
		spotFinder/spotSectionView.h \
		spotFinder/spotSection.h \
		spotFinder/spotsWidget.h \
		spotFinder/perimeter.h \
		spotFinder/perimeterWindow/perimeterPlotter.h \
		spotFinder/perimeterWindow/perimeterWindow.h \
		spotFinder/spotDensityWidget.h \
		spotFinder/blurWidget.h \
		spotFinder/spotMapper/spotMapperWindow.h \
		spotFinder/spotMapper/spotMapper.h \
		spotFinder/spotMapper/nearestNeighborMapper.h \
		spotFinder/spotMapper/spotPerimeterMapper.h \
		spotFinder/cell.h \
		maskPainter.h \
		pointViewer/pointViewWidget.h \
		button/arrowButton.h \
		button/dropButton.h \
		flatViewer/flatView.h \
		kcluster/kClusterProcess.h \
		kcluster/clusterWidget.h \
		kcluster/clusterWindow.h \
		stat/stat.h \
		panels/frame.h \
		panels/frameSet.h \
		panels/frameStack.h \
		panels/fileSet.h \
		panels/overlapViewer.h \
		panels/overlapWindow.h \
		panels/fileSetInfo.h \
		image/imageFunc.h \
		image/imageAnalyser.h \
		image/threeDPeakFinder.h \
		image/bitMask.h \
		image/volumeMask.h \
		distanceMapper/compareController.h \
		distanceMapper/distanceMapper.h \
		distanceMapper/distanceViewer.h \
		distanceMapper/pointDrawer.h \
		distanceMapper/objectComparer.h \
		customWidgets/fSpinBox.h \
		abstract/space.h \
		tiff/tiffReader.h 
SOURCES		= \
		dvReader.cpp \
		deltaViewer.cpp \
		dataStructs.cpp \
		jpgView/jpgView.cpp \
		opengl/glImage.cpp \
		distchooser/distChooser.cpp \
		distchooser/tabWidget.cpp \
		colorChooser.cpp \
		parameterChooser.cpp \
		colorMapper.cpp \
		linGraph/slider.cpp \
		linGraph/plotWidget.cpp \
		spotFinder/spotWindow.cpp \
		spotFinder/channelWidget.cpp \
		spotFinder/modelWidget.cpp \
		spotFinder/nucleusWidget.cpp \
		spotFinder/setWidget.cpp \
		spotFinder/spotSectionView.cpp \
		spotFinder/spotSection.cpp \
		spotFinder/spotsWidget.cpp \
		spotFinder/perimeter.cpp \
		spotFinder/perimeterWindow/perimeterPlotter.cpp \
		spotFinder/perimeterWindow/perimeterWindow.cpp \
		spotFinder/spotDensityWidget.cpp \
		spotFinder/blurWidget.cpp \
		spotFinder/spotMapper/spotMapperWindow.cpp \
		spotFinder/spotMapper/spotMapper.cpp \
		spotFinder/spotMapper/nearestNeighborMapper.cpp \
		spotFinder/spotMapper/spotPerimeterMapper.cpp \
		spotFinder/cell.cpp \
		maskPainter.cpp \
		pointViewer/pointViewWidget.cpp \
		button/arrowButton.cpp \
		button/dropButton.cpp \
		flatViewer/flatView.cpp \
		kcluster/kClusterProcess.cpp \
		kcluster/clusterWidget.cpp \
		kcluster/clusterWindow.cpp \
		stat/stat.cpp \
		panels/frame.cpp \
		panels/frameSet.cpp \
		panels/frameStack.cpp \
		panels/fileSet.cpp \
		panels/overlapViewer.cpp \
		panels/overlapWindow.cpp \
		panels/fileSetInfo.cpp \
		image/imageFunc.cpp \
		image/imageAnalyser.cpp \
		image/threeDPeakFinder.cpp \
		image/bitMask.cpp \
		image/volumeMask.cpp \
		distanceMapper/compareController.cpp \
		distanceMapper/distanceMapper.cpp \
		distanceMapper/distanceViewer.cpp \
		distanceMapper/pointDrawer.cpp \
		distanceMapper/objectComparer.cpp \
		customWidgets/fSpinBox.cpp \
		abstract/space.cpp \
		tiff/tiffReader.cpp \
		main.cpp
TARGET		= reader

#The following line was inserted by qt3to4
QT +=  opengl 
