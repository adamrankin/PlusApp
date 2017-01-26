#include "PlusConfigure.h"
#include "vtkFreehandController.h"
#include "StylusCalibrationController.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include "vtksys/CommandLineArguments.hxx" 
#include "vtksys/SystemTools.hxx"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"

#include <stdlib.h>
#include <iostream>

///////////////////////////////////////////////////////////////////
const double ERROR_THRESHOLD = 0.05; // error threshold is 5% 

int CompareCalibrationResultsWithBaseline(const char* baselineFileName, const char* currentResultFileName); 

int main (int argc, char* argv[])
{ 
	std::string inputConfigFileName;
	std::string inputBaselineFileName;

	int numberOfAcquiredPoints=100;
	int verboseLevel=PlusLogger::LOG_LEVEL_INFO;

	vtksys::CommandLineArguments cmdargs;
	cmdargs.Initialize(argc, argv);

	cmdargs.AddArgument("--input-config-file-name", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &inputConfigFileName, "Configuration file name");
	cmdargs.AddArgument("--input-baseline-file-name", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &inputBaselineFileName, "Name of file storing baseline calibration results");
	cmdargs.AddArgument("--number-of-acquired-points", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &numberOfAcquiredPoints, "Number of acquired points during the pivot calibration");
	cmdargs.AddArgument("--verbose", vtksys::CommandLineArguments::EQUAL_ARGUMENT, &verboseLevel, "Verbose level (1=error only, 2=warning, 3=info, 4=debug)");	

	if ( !cmdargs.Parse() )
	{
		std::cerr << "Problem parsing arguments" << std::endl;
		std::cout << "Help: " << cmdargs.GetHelp() << std::endl;
		exit(EXIT_FAILURE);
	}

	PlusLogger::Instance()->SetLogLevel(verboseLevel);

	VTK_LOG_TO_CONSOLE_ON; 

	std::string programPath("./"), errorMsg; 
	if ( !vtksys::SystemTools::FindProgramPath(argv[0], programPath, errorMsg) )
	{
		LOG_ERROR(errorMsg); 
	}
	programPath = vtksys::SystemTools::GetParentDirectory(programPath.c_str()); 

	LOG_INFO("Initialize"); 

	// Initialize the controllers
	vtkSmartPointer<vtkFreehandController> controller = vtkFreehandController::GetInstance();
	controller->SetInputConfigFileName(inputConfigFileName.c_str());
	controller->Initialize();
	controller->TrackingOnlyOn();
	controller->StartDataCollection();

	StylusCalibrationController* stylusCalibrationController = StylusCalibrationController::GetInstance();
	stylusCalibrationController->Initialize();
	stylusCalibrationController->SetNumberOfPoints(numberOfAcquiredPoints);
	stylusCalibrationController->Start();

	// Acquire positions for pivot calibration
	do {
		vtksys::SystemTools::Delay(50);
		PlusLogger::PrintProgressbar((100.0 * stylusCalibrationController->GetCurrentPointNumber()) / numberOfAcquiredPoints); 

		if (stylusCalibrationController->GetCurrentPointNumber() == numberOfAcquiredPoints - 1) {
			PlusLogger::PrintProgressbar(100.0);
		}

		// Acquire point and do registration at last point
		stylusCalibrationController->DoAcquisition();
	} while (stylusCalibrationController->GetCurrentPointNumber() < numberOfAcquiredPoints);

	// Save result
	vtkstd::string calibrationResultFileName = "StylusCalibrationTest.xml";
	stylusCalibrationController->SaveStylusCalibrationToFile(calibrationResultFileName);

	if ( CompareCalibrationResultsWithBaseline( inputBaselineFileName.c_str(), calibrationResultFileName.c_str() ) !=0 )
	{
		LOG_ERROR("Comparison of calibration data to baseline failed");
		std::cout << "Exit failure!!!" << std::endl;

		delete stylusCalibrationController;

		return EXIT_FAILURE;
	}

	delete stylusCalibrationController;

	std::cout << "Exit success!!!" << std::endl; 
	return EXIT_SUCCESS; 
}

//-----------------------------------------------------------------------------

// return the number of differences
int CompareCalibrationResultsWithBaseline(const char* baselineFileName, const char* currentResultFileName)
{
	int numberOfFailures=0;

	double* transformCurrent = new double[16]; 
	double* transformBaseline = new double[16]; 

	// Load current stylus calibration
	vtkSmartPointer<vtkXMLDataElement> stylusCalibrationCurrent = vtkXMLUtilities::ReadElementFromFile(currentResultFileName);

	if (stylusCalibrationCurrent == NULL) {	
		LOG_ERROR("Current stylus calibration file not found!"); 
		numberOfFailures++;
	}

	vtkXMLDataElement* stylusToStylusTipTransformCurrent = stylusCalibrationCurrent->FindNestedElementWithName("StylusToStylusTipTransform"); 

	if (stylusToStylusTipTransformCurrent == NULL) {
		LOG_ERROR("Stylus calibration transform not found!");
		numberOfFailures++;
	} else {
		stylusToStylusTipTransformCurrent->GetVectorAttribute("Transform", 16, transformCurrent);
	}

	// Load baseline styus calibration
	vtkSmartPointer<vtkXMLDataElement> stylusCalibrationBaseline = vtkXMLUtilities::ReadElementFromFile(baselineFileName);

	if (stylusCalibrationBaseline == NULL) {	
		LOG_ERROR("Baseline stylus calibration file not found!"); 
		numberOfFailures++;
	}

	vtkXMLDataElement* stylusToStylusTipTransformBaseline = stylusCalibrationBaseline->FindNestedElementWithName("StylusToStylusTipTransform"); 

	if (stylusToStylusTipTransformBaseline == NULL) {
		LOG_ERROR("Stylus calibration transform not found!");
		numberOfFailures++;
	} else {
		stylusToStylusTipTransformBaseline->GetVectorAttribute("Transform", 16, transformBaseline);
	}

	if (numberOfFailures > 0) {
		delete[] transformCurrent;
		delete[] transformBaseline;

		return numberOfFailures;
	}

	// Compare the transforms
	for (int i=0; i<16; ++i) {
		double ratio = 1.0 * transformCurrent[i] / transformBaseline[i];
		double diff = fabs(transformCurrent[i] - transformBaseline[i]);

		if ( (ratio > 1 + ERROR_THRESHOLD || ratio < 1 - ERROR_THRESHOLD) && (diff > 10.0 * ERROR_THRESHOLD) ) // error has to be greater than 5% and also greater than 0.5mm
		{
			LOG_ERROR("Transform element (" << i << ") mismatch: current=" << transformCurrent[i]<< ", baseline=" << transformBaseline[i]);
			numberOfFailures++;
		}
	}

	delete[] transformCurrent;
	delete[] transformBaseline;

	return numberOfFailures;
}