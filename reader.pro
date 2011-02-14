# change debug to release and recompile when working
TEMPLATE        = app
CONFIG          += qt opengl debug thread
QT		+= qt3support 
CONFIG          += console
LIBS		+= -lrt
HEADERS		= \
		dvReader.h \
		deltaViewer.h \
		dataStructs.h \
		datastructs/a_pos.h \
		datastructs/channelOffset.h \
		jpgView/jpgView.h \
		opengl/glImage.h \
		distchooser/distChooser.h \
		distchooser/tabWidget.h \
		colorChooser.h \
		parameterChooser.h \
		colorMapper.h \
		linGraph/slider.h \
		linGraph/plotWidget.h \
		linGraph/linePlotter.h \
		linGraph/distPlotter.h \
		scatterPlot/scatterPlotter.h \
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
		spotFinder/spotMapper/NNMapper2.h \
		spotFinder/cell.h \
		spotFinder/channelSelector.h \
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
		panels/overlapEditor.h \
		panels/overlapEditorWindow.h \
		panels/frameRect.h \
		panels/borderInformation.h \
		panels/idMap.h \
		panels/stack_stats.h \
		image/imageFunc.h \
		image/imageAnalyser.h \
		image/imageData.h \
		image/volumeCache.h \
		image/threeDPeakFinder.h \
		image/bitMask.h \
		image/volumeMask.h \
		image/volumeMap.h \
		image/linMap.h \
		image/blobMapper.h \
		image/blobMapperWidget.h \
		image/blobMapperWidgetManager.h \
		image/coordConverter.h \
		image/blob.h \
		image/superBlob.h \
		image/blobScatterPlot.h\
		image/nd_classifier.h \
		image/blobClassifier.h \
		image/superBlobWidget.h \
		image/background.h \
		image/backgroundWidget.h \
		image/backgroundWindow.h \
		image/rectangle.h \
		image/two_d_background.h \
		image/spectralResponse.h \
		image/gaussian.h \
		image/blobModel.h \
		cavity/cavityBall.h \
		cavity/cavityBallInputWidget.h \
		cavity/cavityMapper.h \
		cavity/ballMap.h \
		distanceMapper/compareController.h \
		distanceMapper/distanceMapper.h \
		distanceMapper/distanceViewer.h \
		distanceMapper/pointDrawer.h \
		distanceMapper/objectComparer.h \
		customWidgets/fSpinBox.h \
		customWidgets/clineEdit.h \
		customWidgets/valueLabels.h \
		abstract/space.h \
		tiff/tiffReader.h \
		imageBuilder/imageBuilder.h \
		imageBuilder/imageBuilderWidget.h \
		imageBuilder/imStack.h \
		imageBuilder/p_parameter.h \
		imageBuilder/f_parameter.h \
	       	imageBuilder/centerFinder.h \
		imageBuilder/blob_mt_mapper.h \
		imageBuilder/stack_info.h \
		imageBuilder/qnt_colors.h \
		imageBuilder/blob_set.h \
		imageBuilder/Blob_mt_mapper_collection.h \
		util/matrix.h	
SOURCES		= \
		dvReader.cpp \
		deltaViewer.cpp \
		dataStructs.cpp \
		datastructs/channelOffset.cpp \
		jpgView/jpgView.cpp \
		opengl/glImage.cpp \
		distchooser/distChooser.cpp \
		distchooser/tabWidget.cpp \
		colorChooser.cpp \
		parameterChooser.cpp \
		colorMapper.cpp \
		linGraph/slider.cpp \
		linGraph/plotWidget.cpp \
		linGraph/linePlotter.cpp \
		linGraph/distPlotter.cpp \
		scatterPlot/scatterPlotter.cpp \
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
		spotFinder/spotMapper/NNMapper2.cpp \
		spotFinder/cell.cpp \
		spotFinder/channelSelector.cpp \
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
		panels/overlapEditor.cpp \
		panels/overlapEditorWindow.cpp \
		panels/frameRect.cpp \
		panels/borderInformation.cpp \
		panels/idMap.cpp \
		image/imageFunc.cpp \
		image/imageAnalyser.cpp \
		image/volumeCache.cpp \
		image/imageData.cpp \
		image/threeDPeakFinder.cpp \
		image/bitMask.cpp \
		image/volumeMask.cpp \
		image/volumeMap.cpp \
		image/linMap.cpp \
		image/blobMapper.cpp \
		image/blobMapperWidget.cpp \
		image/blobMapperWidgetManager.cpp \
		image/blob.cpp \
		image/blobScatterPlot.cpp \
		image/nd_classifier.cpp \
		image/blobClassifier.cpp \
		image/superBlobWidget.cpp \
		image/background.cpp \
		image/backgroundWidget.cpp \
		image/backgroundWindow.cpp \
		image/two_d_background.cpp \
		image/spectralResponse.cpp \
		image/gaussian.cpp \
		image/blobModel.cpp \
		cavity/cavityBall.cpp \
		cavity/cavityBallInputWidget.cpp \
		cavity/cavityMapper.cpp \
		distanceMapper/compareController.cpp \
		distanceMapper/distanceMapper.cpp \
		distanceMapper/distanceViewer.cpp \
		distanceMapper/pointDrawer.cpp \
		distanceMapper/objectComparer.cpp \
		customWidgets/fSpinBox.cpp \
		customWidgets/valueLabels.cpp \
		abstract/space.cpp \
		tiff/tiffReader.cpp \
		imageBuilder/imageBuilder.cpp \
		imageBuilder/imageBuilderWidget.cpp \
		imageBuilder/imStack.cpp \ 
		imageBuilder/p_parameter.cpp \
		imageBuilder/f_parameter.cpp \
		imageBuilder/centerFinder.cpp \
		imageBuilder/blob_mt_mapper.cpp \
		imageBuilder/qnt_colors.cpp \
		imageBuilder/blob_set.cpp \
		imageBuilder/Blob_mt_mapper_collection.cpp \
		util/matrix.cpp \
		globalVariables.cpp \
		main.cpp
TARGET		= reader

#The following line was inserted by qt3to4
QT +=  opengl 
