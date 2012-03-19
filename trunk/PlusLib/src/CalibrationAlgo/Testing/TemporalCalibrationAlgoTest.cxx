/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/ 

/*!
  \file TemporalCalibrationAlgoTest.cxx
  \brief This program tests the temporal calibration algorithm by computing the time by which the inputted tracker data lags the inputted US video data.
  
  The inputted data--video and tracker--is assumed to be collected by a US probe imaging a planar object; furthermore,
  it is assumed that the probe is undergoing uni-dirctional periodic motion in the direction perpendicular to the
  plane's face (E.g. moving the probe in a repeating up-and-down fashion while imaging the bottom of a water bath).
  The inputted data is assumed to contain at least ?five? full periods (although the algorithm may work for fewer periods
  it has not been tested under these conditions.

  \ingroup PlusLibCalibrationAlgorithm
*/ 

#include "TemporalCalibrationAlgo.h"

void SaveMetricPlot(const char* filename, vtkTable* videoPositionMetric, vtkTable* trackerPositionMetric)
{
  // Set up the view
  vtkSmartPointer<vtkContextView> uncalibratedView = vtkSmartPointer<vtkContextView>::New();
  uncalibratedView->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  vtkSmartPointer<vtkChartXY> uncalibratedChart =  vtkSmartPointer<vtkChartXY>::New();
  uncalibratedView->GetScene()->AddItem(uncalibratedChart);

  // Add the two line plots    
  vtkPlot *videoPositionMetricLine = uncalibratedChart->AddPlot(vtkChart::LINE);

  videoPositionMetricLine->SetInput(videoPositionMetric, 0, 1);

  videoPositionMetricLine->SetColor(0,0,1);
  videoPositionMetricLine->SetWidth(1.0);

  vtkPlot *uncalibratedTrackerMetricLine = uncalibratedChart->AddPlot(vtkChart::LINE);
  uncalibratedTrackerMetricLine->SetInput(trackerPositionMetric, 0, 1);
  uncalibratedTrackerMetricLine->SetColor(0,1,0);
  uncalibratedTrackerMetricLine->SetWidth(1.0);
  uncalibratedChart->SetShowLegend(true);

  // Save plot to file

  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(uncalibratedView->GetRenderer());
  renderWindow->SetSize(800,400);
  renderWindow->OffScreenRenderingOn(); 

  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName(filename);
  writer->SetInput(windowToImageFilter->GetOutput());
  writer->Write();
}

int main(int argc, char **argv)
{
  bool printHelp(false);
  bool plotResults(false);
  bool saveIntermediateImages(false);
  int verboseLevel = vtkPlusLogger::LOG_LEVEL_DEFAULT;
  std::string inputTrackerSequenceMetafile; // Raw-buffer tracker file
  std::string inputVideoSequenceMetafile; // Corresponding raw-buffer video file
  std::string intermediateFileOutputDirectory; // Directory into which the intermediate files are written
  double samplingResolutionSec = 0.001; //  Resolution used for re-sampling [s]

  vtksys::CommandLineArguments args;
  args.Initialize(argc, argv);

  args.AddArgument("--help",vtksys::CommandLineArguments::NO_ARGUMENT, &printHelp, "Print this help.");
  args.AddArgument("--input-video-sequence-metafile", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &inputVideoSequenceMetafile, "Input US image sequence metafile name with path");
  args.AddArgument("--input-tracker-sequence-metafile", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &inputTrackerSequenceMetafile, "Input tracker sequence metafile name with path");
  args.AddArgument("--plot-results",vtksys::CommandLineArguments::NO_ARGUMENT, &plotResults, "Plot results (display position vs. time plots without and with temporal calibration)");
  args.AddArgument("--verbose",vtksys::CommandLineArguments::EQUAL_ARGUMENT, &verboseLevel, "Verbose level (1=error only, 2=warning, 3=info, 4=debug, 5=trace)");
  args.AddArgument("--sampling-resolution-sec",vtksys::CommandLineArguments::EQUAL_ARGUMENT, &samplingResolutionSec, "Sampling resolution (in seconds, default is 0.001)");    
  args.AddArgument("--save-intermediate-images",vtksys::CommandLineArguments::NO_ARGUMENT, &saveIntermediateImages, "Save images of intermediate steps (scanlines used, and detected lines)");
  args.AddArgument("--intermediate-file-output-directory", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &intermediateFileOutputDirectory, "Directory into which the intermediate files are written");

  if ( !args.Parse() )
  {
    std::cerr << "Problem parsing arguments" << std::endl;
    std::cout << "Help: " << args.GetHelp() << std::endl;
    exit(EXIT_FAILURE);
  }

  if ( printHelp )
  {
    std::cout << "Help: " << args.GetHelp() << std::endl;
    exit(EXIT_SUCCESS);
  }

  vtkPlusLogger::Instance()->SetLogLevel(verboseLevel);

  if ( inputTrackerSequenceMetafile.empty() )
  {
    std::cerr << "input-tracker-sequence-metafile required argument!" << std::endl;
    std::cout << "Help: " << args.GetHelp() << std::endl;
    exit(EXIT_FAILURE);
  }

  if ( inputVideoSequenceMetafile.empty() )
  {
    std::cerr << "input-video-sequence-metafile required argument!" << std::endl;
    std::cout << "Help: " << args.GetHelp() << std::endl;
    exit(EXIT_FAILURE);
  }

  vtkSmartPointer<vtkTrackedFrameList> trackerFrames = vtkSmartPointer<vtkTrackedFrameList>::New();
  vtkSmartPointer<vtkTrackedFrameList> videoFrames = vtkSmartPointer<vtkTrackedFrameList>::New(); 
  
  //  Read tracker frames
  LOG_DEBUG("Read tracker data from "<<inputTrackerSequenceMetafile);
  if ( trackerFrames->ReadFromSequenceMetafile(inputTrackerSequenceMetafile.c_str()) != PLUS_SUCCESS )
  {
    LOG_ERROR("Failed to read tracked pose sequence metafile: " << inputTrackerSequenceMetafile << ". Exiting...");
    exit(EXIT_FAILURE);
  }

  //  Read US video frames
  LOG_DEBUG("Read video data from "<<inputVideoSequenceMetafile);
  if ( videoFrames->ReadFromSequenceMetafile(inputVideoSequenceMetafile.c_str()) != PLUS_SUCCESS )
  {
    LOG_ERROR("Failed to read US image sequence metafile: " << inputVideoSequenceMetafile << ". Exiting...");
    exit(EXIT_FAILURE);
  }
  
  //  Create temporal calibration object; Set pertinent parameters
  TemporalCalibration testTemporalCalibrationObject;
  testTemporalCalibrationObject.SetTrackerFrames(trackerFrames);
  testTemporalCalibrationObject.SetVideoFrames(videoFrames);
  testTemporalCalibrationObject.SetSamplingResolutionSec(0.001);
  testTemporalCalibrationObject.SetSaveIntermediateImagesToOn(saveIntermediateImages);
  testTemporalCalibrationObject.SetIntermediateFilesOutputDirectory(intermediateFileOutputDirectory);

  //  Calculate the time-offset
  if (testTemporalCalibrationObject.Update()!=PLUS_SUCCESS)
  {
    LOG_ERROR("Cannot determine tracker lag, temporal calibration failed");
    exit(EXIT_FAILURE);
  }

  double trackerLagSec=0;
  if (testTemporalCalibrationObject.GetTrackerLagSec(trackerLagSec)!=PLUS_SUCCESS)
  {
    LOG_ERROR("Cannot determine tracker lag, temporal calibration failed");
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Tracker lag: " << trackerLagSec << " sec (>0 if the tracker data lags)");

  std::ostrstream trackerLagOutputFilename;
  trackerLagOutputFilename << intermediateFileOutputDirectory << "\\TrackerLag.txt" << std::ends;
  ofstream myfile;
  myfile.open (trackerLagOutputFilename.str());
  myfile << trackerLagSec;
  myfile.close();

  if (plotResults)
  {
    vtkSmartPointer<vtkTable> videoPositionMetric=vtkSmartPointer<vtkTable>::New();
    testTemporalCalibrationObject.GetVideoPositionSignal(videoPositionMetric);

    // Uncalibrated
    vtkSmartPointer<vtkTable> uncalibratedTrackerPositionMetric=vtkSmartPointer<vtkTable>::New();
    testTemporalCalibrationObject.GetUncalibratedTrackerPositionSignal(uncalibratedTrackerPositionMetric);
    std::string filename=intermediateFileOutputDirectory + "\\MetricPlotUncalibrated.png";
    SaveMetricPlot(filename.c_str(), videoPositionMetric, uncalibratedTrackerPositionMetric);
  
    // Calibrated
    vtkSmartPointer<vtkTable> calibratedTrackerPositionMetric=vtkSmartPointer<vtkTable>::New();
    testTemporalCalibrationObject.GetCalibratedTrackerPositionSignal(calibratedTrackerPositionMetric);
    filename=intermediateFileOutputDirectory + "\\MetricPlotCalibrated.png";
    SaveMetricPlot(filename.c_str(), videoPositionMetric, calibratedTrackerPositionMetric);
  }

  return EXIT_SUCCESS;
}

