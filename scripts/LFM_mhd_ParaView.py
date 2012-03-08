import glob

try: paraview.simple
except: from paraview.simple import *
paraview.simple._DisableFirstRenderCameraReset()

# TODO:  
#  1.  Set min/max value for every frame in AnimationScene
#      See http://www.paraview.org/pipermail/paraview/2011-September/022682.html
#


LoadPlugin('/Users/schmitt/paraview/paraview/trunk/plugins/vtkLFMReader/build/libvtkLFMReader.dylib',ns=globals())

# Load LFM file(s)
#vtkLfmReaderObject = vtkLFMReader( FileNames=['/Users/schmitt/paraview/testData/LRs_mhd_1995-03-21T04-20-00Z.hdf'] )
#vtkLfmReaderObject = vtkLFMReader( FileNames=['/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_004.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_005.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_006.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_007.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_008.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_009.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_010.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_011.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_012.hdf',
#                                              '/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_013.hdf'] )

#files = glob.glob('/Users/schmitt/paraview/testData/doctoredAnimation/LMs_mhd_*.hdf')
files = glob.glob('/Users/schmitt/paraview/testData/doctoredAnimation/orig/LMs_mhd_*.hdf')
files.sort()
vtkLfmReaderObject = vtkLFMReader(FileNames = files)


vtkLfmReaderObject.PointArrayStatus = []
vtkLfmReaderObject.CellArrayStatus = []
vtkLfmReaderObject.GridScaleFactor = 'Earth Radius: 6.5e8 cm'
vtkLfmReaderObject.CellArrayStatus = ['Plasma Density', 'Sound Speed', 'Velocity Vector','Magnetic Field Vector']
Show().Visibility = 0

##################
# Top-left panel #
########################################################################
# Orient the camera
TopLeftRenderView = GetRenderView()
TopLeftRenderView.CameraPosition = [-7.0, -70.0, 0.0]
TopLeftRenderView.CameraFocalPoint = [-7, 0.0, 0.0]
TopLeftRenderView.CameraViewUp = [0.0, 0.0, 1.0]
TopLeftRenderView.CameraClippingRange = [122.35967717295773, 129.70814347061219]
TopLeftRenderView.CameraParallelScale = 218.48459610631258
TopLeftRenderView.CenterOfRotation = [0.0, 0.0, 0.0]

# Add plane and map it to the dataset
XZVectorPlane = Plane()
XZVectorPlane.Origin = [-60.0, 0.0, -30.0]
XZVectorPlane.Point1 = [30.0, 0.0, -30.0]
XZVectorPlane.Point2 = [-60.0, 0.0, 30.0]
XZVectorPlane.XResolution = 20
XZVectorPlane.YResolution = 15
SetActiveSource(XZVectorPlane)
Show().Visibility = 0

# ResampleWithDataset
SetActiveSource(vtkLfmReaderObject)
XZField = ResampleWithDataset( Source=XZVectorPlane )
Show().Visibility = 0

# Render vector field
XZVectors = Glyph( GlyphType="Arrow", GlyphTransform="Transform2" )
XZVectors.SetScaleFactor = 9.0
XZVectors.Vectors = ['POINTS', 'Velocity Vector']
XZVectors.GlyphTransform = "Transform2"
XZVectors.GlyphType = "Arrow"
XZVectors.GlyphType.TipRadius = 0.04
XZVectors.GlyphType.TipLength = 0.15
XZVectors.GlyphType.ShaftRadius = 0.015
XZVectors.SetScaleFactor = 2.14564239898506e-07
DataRepresentation16 = Show()
DataRepresentation16.EdgeColor = [0.0, 0.0, 0.5019607843137255]
DataRepresentation16.ColorArrayName = ''

# XY cutplane for colormap
SetActiveSource(vtkLfmReaderObject)
XZSlice = Slice( SliceType="Plane" )
XZSlice.SliceOffsetValues = [0.0]
XZSlice.SliceType.Origin = [-151.826509475708, 0.0, 0.0]
XZSlice.SliceType = "Plane"
XZSlice.SliceType.Normal = [0.0, 1.0, 0.0]

# Calculator for pressure
Pressure = Calculator()
Pressure.AttributeMode = 'point_data'
Pressure.Function = 'Plasma Density*4.7619e23*Sound Speed*Sound Speed*3.75e8'
Pressure.ResultArrayName = 'Pressure'

PressureRepresentation = Show()
PressureRepresentation.EdgeColor = [0.0, 0.0, 0.5000076295109483]
PressureRepresentation.ColorArrayName = 'Pressure'
a1_Pressure_PVLookupTable = GetLookupTableForArray( "Pressure", 1, NanColor=[0.498039, 0.498039, 0.498039], RGBPoints=[7.232339585875363e+19, 0.0, 0.0, 1.0, 3.964840999531023e+24, 1.0, 0.0, 0.0], VectorMode='Magnitude', ColorSpace='HSV', ScalarRangeInitialized=1.0 )
a1_Pressure_PiecewiseFunction = CreatePiecewiseFunction()
PressureRepresentation.LookupTable = a1_Pressure_PVLookupTable

ScalarBarWidgetLog10Pressure = CreateScalarBar( Orientation='Horizontal', Title='Pressure', Position2=[0.5, 0.15], LabelFontSize=12, Enabled=1, TitleFontSize=12, Position=[0.25,0.85] )
TopLeftRenderView.Representations.append(ScalarBarWidgetLog10Pressure)
a1_Pressure_PVLookupTable = GetLookupTableForArray( "Pressure", 1, UseLogScale=1,  RGBPoints=[1e+22, 0.0, 0.0, 1.0, 3.96484e+24, 1.0, 0.0, 0.0], LockScalarRange=1 )
TopLeftRenderView.CameraClippingRange = [119.96970760320372, 132.85099018726737]
ScalarBarWidgetLog10Pressure.LookupTable = a1_Pressure_PVLookupTable

# Describe the view
minValue = a1_Pressure_PVLookupTable.RGBPoints[0]
maxValue = a1_Pressure_PVLookupTable.RGBPoints[4]
TopLeftText = Text()
TopLeftText.Text = 'XZ (min=%e  max=%e)' % (minValue, maxValue)
TextRep = GetDisplayProperties(TopLeftText)
TextRep.Visibility = 1


###################
# Top-Right panel #
########################################################################
TopRightRenderView = CreateRenderView()
#TopRightRenderView.CameraPosition = [-9.54128751659703, -1.5694684006493071, 150.56293391130203]
TopRightRenderView.CameraPosition = [-7, 0.0, 70]
#TopRightRenderView.CameraFocalPoint = [-9.54128751659703, -1.5694684006493071, 0.0]
TopRightRenderView.CameraFocalPoint = [-7, 0.0, 0.0]
TopRightRenderView.CameraViewUp = [0.0, 1.0, 0.0]
TopRightRenderView.CompressorConfig = 'vtkSquirtCompressor 0 3'
TopRightRenderView.UseLight = 1
TopRightRenderView.LightSwitch = 0
TopRightRenderView.RemoteRenderThreshold = 3.0
TopRightRenderView.ViewTime = 0.0
TopRightRenderView.Background = [0.31999694819562063, 0.3400015259021897, 0.4299992370489052]
TopRightRenderView.CenterAxesVisibility = 0
TopRightRenderView.CenterOfRotation = [0.0, 0.0, 0.0]
TopRightRenderView.CameraParallelScale = 317.214749894812
TopRightRenderView.CameraClippingRange = [149.05730362328296, 152.8213791144487]

# XY Cutplane
SetActiveSource(vtkLfmReaderObject)

# Subtract Dipole
BzMinusDipole = Calculator()
BzMinusDipole.AttributeMode = 'point_data'
BzMinusDipole.Function = '(Magnetic Field Vector_Z*1e5)+(3.05e4*((coordsX^2+coordsY^2+coordsZ^2)^(-1.5))*(2-(3*(coordsX^2+coordsY^2))/(coordsX^2+coordsY^2+coordsZ^2)))'
BzMinusDipole.ResultArrayName = 'Bz-Dipole'

BzNoDipole = Slice( SliceType="Plane" )
BzNoDipole.SliceOffsetValues = [0.0]
BzNoDipole.SliceType.Origin = [-151.826509475708, 0.0, 0.0]
BzNoDipole.SliceType = "Plane"
BzNoDipole.SliceType.Normal = [0.0, 0.0, 1.0]
DataRepresentation22 = Show()
DataRepresentation22.EdgeColor = [0.0, 0.0, 0.5000076295109483]
DataRepresentation22.EdgeColor = [0.0, 0.0, 0.5019607843137255]

a1_BzDipole_PVLookupTable = GetLookupTableForArray( "Bz-Dipole", 1, RGBPoints=[-20.0, 0.0, 0.0, 1.0, 20.0, 1.0, 0.0, 0.0], VectorMode='Magnitude', NanColor=[0.498039, 0.498039, 0.498039], ColorSpace='HSV', ScalarRangeInitialized=1.0, LockScalarRange=1 )
a1_BzDipole_PiecewiseFunction = CreatePiecewiseFunction()
DataRepresentation22.ColorArrayName = 'Bz-Dipole'
DataRepresentation22.LookupTable = a1_BzDipole_PVLookupTable

ScalarBarWidgetBzNoDipole = CreateScalarBar( Orientation='Horizontal',Title='Bz-Dipole', LabelFontSize=12,Position2=[0.5, 0.15], Enabled=1, TitleFontSize=12,Position=[0.25,0.85] )
TopRightRenderView.Representations.append(ScalarBarWidgetBzNoDipole)
a1_BzNoDip_PVLookupTable = GetLookupTableForArray( "Bz-Dipole", 1, UseLogScale=1 )
ScalarBarWidgetBzNoDipole.LookupTable = a1_BzNoDip_PVLookupTable


# Describe the view
minValue = a1_BzNoDip_PVLookupTable.RGBPoints[0]
maxValue = a1_BzNoDip_PVLookupTable.RGBPoints[4]
TopRightText = Text()
TopRightText.Text = 'XY (min=%e  max=%e)' % (minValue, maxValue)
TextRep = GetDisplayProperties(TopRightText)
TextRep.Visibility = 1

#####################
# Bottom-left panel #
########################################################################
SetActiveView(TopLeftRenderView)
BotLeftRenderView = CreateRenderView()
#BotLeftRenderView.CameraPosition =  [0.0, 0.0, 116.77367590722402]
#BotLeftRenderView.CameraFocalPoint = [-7, 0.0, 0.0]
#BotLeftRenderView.CameraViewUp = [0.0, 0.0, 1.0]

BotLeftRenderView.CameraPosition = [-7, 0.0, 70]
BotLeftRenderView.CameraFocalPoint = [-7, 0.0, 0.0]
BotLeftRenderView.CameraViewUp = [0.0, 1.0, 0.0]

BotLeftRenderView.CameraParallelScale = 1.7320508075688772
BotLeftRenderView.CompressorConfig = 'vtkSquirtCompressor 0 3'
BotLeftRenderView.UseLight = 1
BotLeftRenderView.LightSwitch = 0
BotLeftRenderView.RemoteRenderThreshold = 3.0
BotLeftRenderView.CameraClippingRange =  [111.82555065103759, 126.028886930742]
BotLeftRenderView.LODResolution = 50.0
BotLeftRenderView.Background = [0.31999694819562063, 0.3400015259021897, 0.4299992370489052]
BotLeftRenderView.CenterAxesVisibility = 0
BotLeftRenderView.CenterOfRotation = [0.0, 0.0, 0.0]

# Add plane and map it to the dataset
XYVectorPlane = Plane()
XYVectorPlane.Origin = [-60.0, -30.0, 0.0]
XYVectorPlane.Point1 = [30.0,  -30.0, 0.0]
XYVectorPlane.Point2 = [-60.0,  30.0, 0.0]
XYVectorPlane.XResolution = 20
XYVectorPlane.YResolution = 15
SetActiveSource(XYVectorPlane)
Show().Visibility = 0
RenameSource("XY Vector Plane", XYVectorPlane)

# ResampleWithDataset
SetActiveSource(vtkLfmReaderObject)
XYField = ResampleWithDataset( Source=XYVectorPlane )
Show().Visibility = 0

# Render vector field
XYVectors = Glyph( GlyphType="Arrow", GlyphTransform="Transform2" )
XYVectors.SetScaleFactor = 9.0
XYVectors.Vectors = ['POINTS', 'Velocity Vector']
XYVectors.GlyphTransform = "Transform2"
XYVectors.GlyphType = "Arrow"
XYVectors.GlyphType.TipRadius = 0.04
XYVectors.GlyphType.TipLength = 0.15
XYVectors.GlyphType.ShaftRadius = 0.015
XYVectors.SetScaleFactor = 2.14564239898506e-07
DataRepresentation16 = Show()
DataRepresentation16.EdgeColor = [0.0, 0.0, 0.5019607843137255]
DataRepresentation16.ColorArrayName = ''

# XY cutplane for colormap
SetActiveSource(vtkLfmReaderObject)
XYSlice = Slice( SliceType="Plane" )
XYSlice.SliceOffsetValues = [0.0]
XYSlice.SliceType.Origin = [-151.826509475708, 0.0, 0.0]
XYSlice.SliceType = "Plane"
XYSlice.SliceType.Normal = [0.0, 0.0, 1.0]

# Calculator for pressure
Pressure = Calculator()
Pressure.AttributeMode = 'point_data'
Pressure.Function = 'Plasma Density*4.7619e23*Sound Speed*Sound Speed*3.75e8'
Pressure.ResultArrayName = 'Pressure'

PressureRepresentation = Show()
PressureRepresentation.EdgeColor = [0.0, 0.0, 0.5000076295109483]
PressureRepresentation.ColorArrayName = 'Pressure'
a1_Pressure_PVLookupTable = GetLookupTableForArray( "Pressure", 1, NanColor=[0.498039, 0.498039, 0.498039], RGBPoints=[7.232339585875363e+19, 0.0, 0.0, 1.0, 3.964840999531023e+24, 1.0, 0.0, 0.0], VectorMode='Magnitude', ColorSpace='HSV', ScalarRangeInitialized=1.0 )
a1_Pressure_PiecewiseFunction = CreatePiecewiseFunction()
PressureRepresentation.LookupTable = a1_Pressure_PVLookupTable

ScalarBarWidgetLog10Pressure = CreateScalarBar( Orientation='Horizontal', Title='Pressure', Position2=[0.5, 0.15],LabelFontSize=12, Enabled=1, TitleFontSize=12,Position=[0.25,0.85] )
BotLeftRenderView.Representations.append(ScalarBarWidgetLog10Pressure)
a1_Pressure_PVLookupTable = GetLookupTableForArray( "Pressure", 1, UseLogScale=1,RGBPoints=[1e+22, 0.0, 0.0, 1.0, 3.96484e+24, 1.0, 0.0, 0.0], LockScalarRange=1 )
ScalarBarWidgetLog10Pressure.LookupTable = a1_Pressure_PVLookupTable


# Describe the view
minValue = a1_Pressure_PVLookupTable.RGBPoints[0]
maxValue = a1_Pressure_PVLookupTable.RGBPoints[4]
BotLeftText = Text()
BotLeftText.Text = 'XY (min=%e  max=%e)' % (minValue, maxValue)
TextRep = GetDisplayProperties(BotLeftText)
TextRep.Visibility = 1

######################
# Bottom-Right panel #
########################################################################
SetActiveView(TopRightRenderView)
BotRightRenderView = CreateRenderView()
#BotRightRenderView.CameraPosition = [-8.45319037422091, 0.7965184288563187, 127.82383156323988]
#BotRightRenderView.CameraFocalPoint = [-8.45319037422091, 0.7965184288563187, 0.0]

BotRightRenderView.CameraPosition = [-7, 0.0, 70]
BotRightRenderView.CameraFocalPoint = [-7, 0.0, 0.0]
BotRightRenderView.CameraViewUp = [0.0, 1.0, 0.0]

BotRightRenderView.CompressorConfig = 'vtkSquirtCompressor 0 3'
BotRightRenderView.UseLight = 1
BotRightRenderView.LightSwitch = 0
BotRightRenderView.RemoteRenderThreshold = 3.0
BotRightRenderView.LODResolution = 50.0
BotRightRenderView.Background = [0.31999694819562063, 0.3400015259021897, 0.4299992370489052]
BotRightRenderView.CenterAxesVisibility = 0
BotRightRenderView.CameraClippingRange = [126.54559229870145, 129.7411902311656]
BotRightRenderView.CenterOfRotation = [0.0, 0.0, 0.0]
BotRightRenderView.CameraParallelScale = 325.86109001049476

SetActiveSource(vtkLfmReaderObject)

# XY Cutplane
Slice5 = Slice( SliceType="Plane" )
Slice5.SliceOffsetValues = [0.0]
Slice5.SliceType.Origin = [-151.826509475708, 0.0, 0.0]
Slice5.SliceType = "Plane"
Slice5.SliceType.Normal = [0.0, 0.0, 1.0]

DataRepresentation23 = Show()
DataRepresentation23.EdgeColor = [0.0, 0.0, 0.5000076295109483]
a3_VelocityVector_PVLookupTable = GetLookupTableForArray( "Velocity Vector", 3, NanColor=[0.498039, 0.498039, 0.498039], RGBPoints=[6236.560207233221, 0.0, 0.0, 1.0, 59331831.819066755, 1.0, 0.0, 0.0], VectorMode='Magnitude', ColorSpace='HSV', ScalarRangeInitialized=1.0 )
a3_VelocityVector_PiecewiseFunction = CreatePiecewiseFunction()
DataRepresentation23.ColorArrayName = 'Velocity Vector'
DataRepresentation23.LookupTable = a3_VelocityVector_PVLookupTable

ScalarBarWidgetVelocity = CreateScalarBar( ComponentTitle='Magnitude', Orientation='Horizontal', Title='Velocity Vector', Position2=[0.5, 0.15], Enabled=1, LabelFontSize=12, TitleFontSize=12,Position=[0.25,0.85] )
BotRightRenderView.Representations.append(ScalarBarWidgetVelocity)

a3_VelocityVector_PVLookupTable = GetLookupTableForArray( "Velocity Vector", 3, RGBPoints=[0.0, 0.0, 0.0, 1.0, 50000000.0, 1.0, 0.0, 0.0], LockScalarRange=1 )
ScalarBarWidgetVelocity.LookupTable = a3_VelocityVector_PVLookupTable


# Describe the view
minValue = a3_VelocityVector_PVLookupTable.RGBPoints[0]
maxValue = a3_VelocityVector_PVLookupTable.RGBPoints[4]
BotRightText = Text()
BotRightText.Text = 'XY (min=%e  max=%e)' % (minValue, maxValue)
TextRep = GetDisplayProperties(BotRightText)
TextRep.Visibility = 1

#################################
# Global visualization settings #
########################################################################

AnimationScene = GetAnimationScene()
AnimationScene.ViewModules = [ TopLeftRenderView, TopRightRenderView, BotLeftRenderView, BotRightRenderView ]
#WriteAnimation('/Users/schmitt/paraview/scripts/testAnimation.jpg', Magnification=1, Quality=2, FrameRate=1.000000)

Render()
#WriteImage('/Users/schmitt/paraview/scripts/LRs_mhd_1995-03-21T04-20-00Z.png')

#### Animate from 1st time step to last
###AnimationScene.StartTime = vtkLfmReaderObject.TimestepValues.GetData()[0]
###AnimationScene.EndTime = vtkLfmReaderObject.TimestepValues.GetData()[-1]
###
###for idx, cur_time in enumerate(vtkLfmReaderObject.TimestepValues.GetData()):  
###    AnimationScene.AnimationTime = cur_time
###    vtkLfmReaderObject.UpdatePipelineInformation()
###
###    WriteImage("testAnimation_topLeft_%03d.png" % idx, TopLeftRenderView);
###    #WriteImage("testAnimation_topright_%03d.png" % idx, TopRightRenderView);
###    #WriteImage("testAnimation_botLeft_%03d.png" % idx, BotLeftRenderView);
###    #WriteImage("testAnimation_botRight_%03d.png" % idx, BotRightRenderView);
