/*!=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/ 

#ifndef __vtkProbeCalibrationAlgo_h
#define __vtkProbeCalibrationAlgo_h

#include "vtkObject.h"

#include "vtkTransform.h"


#include "itkImage.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_vector.h"

#include <string>
#include <vector>
#include <deque>

class vtkHTMLGenerator; 
class vtkGnuplotExecuter;
class TrackedFrame; 
class vtkTrackedFrameList; 
class vtkTransformRepository;
class vtkXMLDataElement; 
struct NWire;

/*!
  \class vtkProbeCalibrationAlgo 
  \brief Probe calibration algorithm class
  \ingroup PlusLibCalibrationAlgorithm
*/
class vtkProbeCalibrationAlgo : public vtkObject
{
public:
	typedef unsigned char PixelType;
	typedef itk::Image< PixelType, 2 > ImageType;

	static vtkProbeCalibrationAlgo *New();
	vtkTypeRevisionMacro(vtkProbeCalibrationAlgo, vtkObject);
	virtual void PrintSelf(ostream& os, vtkIndent indent); 

	/*! Initialize the calibration controller interface */
	virtual PlusStatus Initialize(); 

	/*!
    Read XML based configuration of the calibration controller
    \param aConfig Root element of device set configuration data
  */
	virtual PlusStatus ReadConfiguration( vtkXMLDataElement* aConfig ); 

  /*!
	  Run calibration algorithm on the two input frame lists. It uses only a certain range of the input sequences (so it is possible to use the same sequence but different sections of it).
	  \param validationTrackedFrameList TrackedFrameList with segmentation results for the validation
    \param validationStartFrame First frame that is used from the validation tracked frame list for the validation (in case of -1 it starts with the first)
    \param validationEndFrame Last frame that is used from the validation tracked frame list for the validation (in case of -1 it starts with the last)
	  \param calibrationTrackedFrameList TrackedFrameList with segmentation results for the calibration
    \param calibrationStartFrame First frame that is used from the calibration tracked frame list for the calibration (in case of -1 it starts with the first)
    \param calibrationEndFrame Last frame that is used from the calibration tracked frame list for the calibration (in case of -1 it starts with the last)
    \param transformRepository Transform repository object to be able to get the default transform
    \param nWires NWire structure that contains the computed imaginary intersections. It used to determine the computed position
	*/
  virtual PlusStatus Calibrate( vtkTrackedFrameList* validationTrackedFrameList, int validationStartFrame, int validationEndFrame, vtkTrackedFrameList* calibrationTrackedFrameList, int calibrationStartFrame, int calibrationEndFrame, vtkTransformRepository* transformRepository, std::vector<NWire> &nWires ); 

  /*!
    Run calibration algorithm on the two input frame lists (uses every frame in the two sequences)
	  \param validationTrackedFrameList TrackedFrameList with segmentation results for the validation
	  \param calibrationTrackedFrameList TrackedFrameList with segmentation results for the calibration
    \param transformRepository Transform repository object to be able to get the default transform
    \param nWires NWire structure that contains the computed imaginary intersections. It used to determine the computed position
	*/
  virtual PlusStatus Calibrate( vtkTrackedFrameList* validationTrackedFrameList, vtkTrackedFrameList* calibrationTrackedFrameList, vtkTransformRepository* transformRepository, std::vector<NWire> &nWires ); 

  /*!
    Calculate and add positions of an individual image for calibration or validation
    \param trackedFrame The actual tracked frame (already segmented) to add for calibration or validation
    \param transformRepository Transform repository object to be able to get the default transform
    \param nWires NWire structure that contains the computed imaginary intersections. It used to determine the computed position
    \param isValidation Flag whether the added data is for calibration or validation
  */
	virtual PlusStatus AddPositionsPerImage( TrackedFrame* trackedFrame, vtkTransformRepository* transformRepository, std::vector<NWire> &nWires, bool isValidation );

	/*! Get flag to show the initialized state */
	vtkGetMacro(Initialized, bool);

  /*! Set/get the calibration date and time in string format */
	vtkSetStringMacro(CalibrationDate); 
  /*! Set/get the calibration date and time in string format */
	vtkGetStringMacro(CalibrationDate);

  /*! Flag to identify the calibration state */
	vtkGetMacro(CalibrationDone, bool);
  /*! Flag to identify the calibration state */
	vtkSetMacro(CalibrationDone, bool);
  /*! Flag to identify the calibration state */
	vtkBooleanMacro(CalibrationDone, bool);

  vtkGetStringMacro(ImageCoordinateFrame);
  vtkGetStringMacro(ProbeCoordinateFrame);
  vtkGetStringMacro(PhantomCoordinateFrame);
  vtkGetStringMacro(ReferenceCoordinateFrame);
  vtkGetStringMacro(TransducerOriginCoordinateFrame);
  vtkGetStringMacro(TransducerOriginPixelCoordinateFrame);

  /*! Reset data containers */
  void ResetDataContainers();

public: // Former ProbeCalibrationController and FreehandCalibraitonController functions

	/*!
    Computes the calibration results: 
	  - Compute the overall Point-Line Distance Error (PLDE)
	  - Print the final calibration results and error reports 
	  - Save the calibration results and error reports into a file 
	  - Save the PRE3D distribution plot to an image file
	  - Map the PRE3D distribution onto the US image 
  */
	virtual PlusStatus ComputeCalibrationResults();

  /*! Check user image home to probe home transform orthogonality */
  virtual bool IsImageToProbeTransformOrthogonal(); 
		
	/*! Update Line Reconstruction Error Analysis for the validation positions in the US probe frame. For details see member variable definitions */
  PlusStatus UpdateLineReconstructionErrorAnalysisVectors();  

  /*! Get Line Reconstruction Error Analysis for the validation positions in the US probe frame. For details see member variable definitions */
	PlusStatus GetLineReconstructionErrorAnalysisVector(int wireNumber, std::vector<double> &LRE);  

	/*!
    Line reconstruction error (LRE) matrix for validation positions in US probe frame
	  This keeps all the original PRE3Ds for the validation dataset with signs in
	  the US probe frame (from the Projected positions to the true positions).
	  FORMAT: matrix 4xN (with N being the total number of validation positions)
	  [ Row-0: PRE3Ds in x-axis from projected to true positions ]
	  [ Row-1: PRE3Ds in y-axis from projected to true positions ]
	  [ Row-2: should be all zeros ]
	  [ Row-3: Euclidean LRE distance ]
	  NOTE: this matrix can be obtained for statistical analysis if desired. 
  */
	vnl_matrix<double> GetLineReconstructionErrorMatrix(int wireNumber);

  /*!
	  Assembles the result string to display
	  \return String containing results
	*/
	std::string GetResultString();

	/*! Flag to set if the US 3D beamwidth data is sucessfully loaded */
	vtkGetMacro(US3DBeamwidthDataReady, bool);
  /*! Flag to set if the US 3D beamwidth data is sucessfully loaded */
	vtkSetMacro(US3DBeamwidthDataReady, bool);
  /*! Flag to set if the US 3D beamwidth data is sucessfully loaded */
	vtkBooleanMacro(US3DBeamwidthDataReady, bool);

	/*!
    Flags to incorporate the ultrasound 3D beam profile (beam width)
	  The flag is set when the 3D US beam width data is to be incorporated into
	  the calibration process (e.g., by adding weights to the least-squares 
	  optimization method w.r.t the beam width profile).
	  NOTE: if the flag is turned on (default is 0), then a US-3D-beam-
	  profile data file needs to be prepared for further processing.
	  OPTIONS: 0 - NO | 1 - BeamwidthVariance | 2 - BeamwidthRatio | 3 - BeamwidthVarianceAndThresholding
  */
	vtkGetMacro(IncorporatingUS3DBeamProfile, int);
  /*! Flags to incorporate the ultrasound 3D beam profile (beam width) */
	vtkSetMacro(IncorporatingUS3DBeamProfile, int);

	/*!
    Axial position of the crystal surface in the TRUS Image Frame
	  Typically, the US machine has a bright mark in the display of the US image 
	  indicating the actual position where the sound starts propagation.
	  NOTE: this position has been converted to the TRUS Image Frame being defined
	  by the user in the calibration configuration file.
  */
	vtkGetMacro(AxialPositionOfCrystalSurfaceInTRUSImageFrame, double);
  /*! Axial position of the crystal surface in the TRUS Image Frame */
	vtkSetMacro(AxialPositionOfCrystalSurfaceInTRUSImageFrame, double);

	/*! Track the current position ID of the output in PRE3D distribution data */
	vtkGetMacro(CurrentPRE3DdistributionID, int);
  /*! Track the current position ID of the output in PRE3D distribution data */
	vtkSetMacro(CurrentPRE3DdistributionID, int);
	
	/*! US 3D beam profile name and path */
	vtkGetStringMacro(US3DBeamProfileDataFileNameAndPath); 
  /*! US 3D beam profile name and path */
	vtkSetStringMacro(US3DBeamProfileDataFileNameAndPath); 

	/*! Get/set image home to transducer origin (pixel) constant transform. Should be defined in config file */
	vtkGetObjectMacro(TransformImageToTransducerOriginPixel, vtkTransform);
	/*! Get/set image home to transducer origin (pixel) constant transform. Should be defined in config file */
	vtkSetObjectMacro(TransformImageToTransducerOriginPixel, vtkTransform);
	
	/*! Get/set the iCAL calibration result transformation between the image and probe home position */
	vtkGetObjectMacro(TransformImageToProbe, vtkTransform);
  /*! Get/set the iCAL calibration result transformation between the image and probe home position */
	vtkSetObjectMacro(TransformImageToProbe, vtkTransform);
		
protected:
	vtkProbeCalibrationAlgo();
	virtual ~vtkProbeCalibrationAlgo();

protected:
	/*! Feed and tun the LSQR minimizer with the acquired and computed positions and conputes reconstruction errors */
	PlusStatus DoCalibration(); 

  /*! Set flag to show the initialized state */
	vtkBooleanMacro(Initialized, bool);
  /*! Set flag to show the initialized state */
  vtkSetMacro(Initialized, bool);

  vtkSetStringMacro(ImageCoordinateFrame);
  vtkSetStringMacro(ProbeCoordinateFrame);
  vtkSetStringMacro(PhantomCoordinateFrame);
  vtkSetStringMacro(ReferenceCoordinateFrame);
  vtkSetStringMacro(TransducerOriginCoordinateFrame);
  vtkSetStringMacro(TransducerOriginPixelCoordinateFrame);

protected: // from former Phantom class
	/*!
    This will construct the validation data matrices. Since the validation data set is separated from the calibration data and acquired before the calibration, the construction of
	  the validation data matrices needs to be done once and only once (e.g., before the 1st validation task is performed) with the flag set to true.  This would save the system runtime
	  during the iterative calibration/validation process
  */
	PlusStatus constructValidationDataMatrices();

	/*! Fill the ultrasound beamwidth profile and weight factors (see mUS3DBeamwidthAndWeightFactorsInUSImageFrameTable5xM member) */
  void FillUltrasoundBeamwidthAndWeightFactorsTable();

	/*! It calculates the 3D point reconstruction error (PRE3D) from the validation data sets if they are imported and ready. */
	virtual PlusStatus compute3DPointReconstructionError();

	/*!
    This operation also computes the Point-Line Distance Error (PLDE) from
	  the validation data sets.  It reconstructs the NWire point positions
	  in the N-wire Phantom space using the existing calibration parameters
	  and then calculates the point-to-line distance to the physical wire
	  location that has been precisly measured on the phantom geometry.
  */
	PlusStatus computeIndependentPointLineReconstructionError();

  /*! Compute beamwidth weight for a beamwidth magnitude at the given axial depth */
  double GetBeamwidthWeightForBeamwidthMagnitude(int aActualAxialDepth);

	/*!
    Get Line reconstruction error (LRE) vector for validation positions in US probe frame
	  FORMAT:
	  [ 0: PRE3Ds in x-axis from projected to true positions ]
	  [ 1: PRE3Ds in y-axis from projected to true positions ]
  */
	vnl_vector<double> getPointLineReconstructionError(vnl_vector<double> NWirePositionInUSImageFrame, vnl_vector<double> NWirePositionInUSProbeFrame);  

protected: // from former vtkProbeCalibrationControllerIO class
	
	/*! This operation writes the final calibration results to a file. */
	virtual void SaveCalibrationResultsAndErrorReportsToXML(); 

	/*! Read in the ultrasound 3D beam profile data from a file */
	virtual PlusStatus ReadUs3DBeamwidthDataFromFile(); 
	
	/*! Load the ultrasound 3D beam profile data */
	virtual PlusStatus LoadUS3DBeamProfileData();

protected:
	/*! Flag to show the initialized state */
	bool Initialized; 

  /*! Flag to identify the calibration state */
	bool CalibrationDone; 

  /*! Calibration date in string format */
  char* CalibrationDate; 

  /*! Name of the image coordinate frame (eg. Image) */
  char* ImageCoordinateFrame;

  /*! Name of the probe coordinate frame (eg. Probe) */
  char* ProbeCoordinateFrame;

  /*! Name of the phantom coordinate frame (eg. Phantom) */
  char* PhantomCoordinateFrame;

  /*! Name of the reference coordinate frame (eg. Reference) */
  char* ReferenceCoordinateFrame;

  /*! Name of the transducer origin (mm) coordinate frame (eg. TransducerOrigin) */
  char* TransducerOriginCoordinateFrame;

  /*! Name of the transducer origin (pixel) coordinate frame (eg. TransducerOriginPixel) */
  char* TransducerOriginPixelCoordinateFrame;

protected: // Former ProbeCalibrationController and FreehandCalibrationController members

	/*! Flag to set if the US 3D beamwidth data is sucessfully loaded */
	bool US3DBeamwidthDataReady;

	/*!
    Flags to incorporate the ultrasound 3D beam profile (beam width)
	  The flag is set when the 3D US beam width data is to be incorporated into
	  the calibration process (e.g., by adding weights to the least-squares 
	  optimization method w.r.t the beam width profile).
	  NOTE: if the flag is turned on (default is 0), then a US-3D-beam-
	  profile data file needs to be prepared for further processing.
	  OPTIONS: 0 - NO | 1 - BeamwidthVariance | 2 - BeamwidthRatio | 3 - BeamwidthVarianceAndThresholding
  */
	int IncorporatingUS3DBeamProfile;

	/*!
    Axial position of the crystal surface in the TRUS Image Frame
	  Typically, the US machine has a bright mark in the display of the US image 
	  indicating the actual position where the sound starts propagation.
	  NOTE: this position has been converted to the TRUS Image Frame being defined
	  by the user in the calibration configuration file.
  */
	double AxialPositionOfCrystalSurfaceInTRUSImageFrame;

	/*! Number of the US 3D beamwidth profile data. This is the total number of US 3D-beam-width data (samples) collected. */
	int NumUS3DBeamwidthProfileData;

	/*! Track the current position ID of the output in PRE3D distribution data */
	int CurrentPRE3DdistributionID;

	/*! US 3D beam profile name and path */
	char* US3DBeamProfileDataFileNameAndPath;
				
	/*!
    Minimum US elevation beamwidth and the focal zone in US Image Frame
	   1. For a typical 1-D linear-array transducer, the ultrasound beam can only be 
	      focused mechanically in the elevation (out-of-plane) axis by placing an 
	      ascoustic lens in front or curving the crystal surface.  
	   2. The elevation beam pattern are therefore solely determined by the size and 
	      curvature of the crystal or the acoustic lens, with sharp focus only 
	      possible at a narrow axial distance (the focal zone) to the transducer.
	   FORMAT: [FOCAL-ZONE (in US Image Frame), MINI-ELEVATION-WIDTH (in millimeters)]
  */
	vnl_vector<double> MinElevationBeamwidthAndFocalZoneInUSImageFrame;

	/*!
    US 3D beamwidth profile data in US Image Frame with weights factors along axial depth.

    1. 3D beam width samples are measured at various axial depth/distance away 
    from the transducer crystals surface, i.e., the starting position of 
    the sound propagation in an ultrasound image.

    2. We have three ways to incorporate the US beamidth to the calibration: Use the 
    variance of beamwidth (BWVar) to weight the calibration, use the beamwidth ratio
    (BWRatio) to weight the calibration, or use the beamwidth to threshold the input
    data (BWTHEVar) in order to eliminate potentially unreliable or error-prone data.

    This is determined by the choice of the input flag (Option-1, 2, or 3).

      [1] BWVar: This conforms to the standard way of applying weights to least squares, 
      where the weights should, ideally, be equal to the reciprocal of the variance of 
      the measurement of the data if they are uncorrelated.  Since we know the US beam
      width at a given axial depth, resonably assuming the data acquired by the sound 
      field is normally distributed, the standard deviation (Sigma) of the data can be
      roughly estimated as in the equation: Sigma = USBeamWidth/4 (for 95% of data).

      [2] BWRatio: The fifth row of the matrix is the overall weight defined at that axial 
      depth. The weight factor was calculated using: CurrentBeamwidth/MinimumBeamwidth.
      The weight is inversely proportional to the weight factor, as obviously, the larger 
      the beamwidth, the less reliable the data is than those with the minimum beamwidth.  
      We found the weight factor to be a good indicator for how reliable the data is, 
      because the larger the beamwidth the larger the uncertainties and errors in data 
      acquired from that US field.  E.g., at the axial depth where the beamwidth is two 
      times that of minimum beamwidth, the uncertainties are doubled than the imaging 
      region that has the minimum beamwidth.

      [3] BWTHEVar: This utilizes the beamwidth to quality control the input calibration 
      data, by filtering out those that has a larger beamwidth (e.g., larger than twice
      of the minimum beamwidth at the current imaging settings).  According to ultrasound
      physics, data acquired in the sound field that doubles the minimum beamwidth at
      the scanplane or elevation plane focal zone are typically much less reliable than
      those closer to the focal zone.  In addition, the filtered, remainng data would be
      weighted for calibration using their beamwidth (BWVar in [1]).

      FORMAT: each column in the matrices has the following rows:
        [0]:		Sorted in ascending axial depth in US Image Frame (in pixels);
        [1-3]:	Beamwith in axial, lateral and elevational axes respectively (in mm);
        [4]:		Weight Factor = CurrentBeamWidth/MininumBeamWidth (>=1).
  */
	vnl_matrix<double> SortedUS3DBeamwidthAndWeightFactorsInAscendingAxialDepthInUSImageFrameMatrix5xN;

	vnl_matrix<double> SortedUS3DBeamwidthInAscendingAxialDepth2CrystalsMatrixNx4; 

	/*!
    Interpolated US 3D beamwidth profile and weight calulated based on it
	  Here we used a simple linear interpolation between sampled data to obtain
	  the beamwidth at non-sampled axial depth.  In general, this approach works
	  fine as long as we have enough sample points that covers the entire US field.
	  FORMAT: each row of the matrix has the following columns:
	  [0]:		Ascending Axial Depth in US Image Frame (in pixels);
	  [1-3]:	Beamwith in axial, lateral and elevational axes respectively (in mm);
	  [4]:		Weight Factor = CurrentBeamWidth/MininumBeamWidth (>=1).
  */
	vnl_matrix<double> InterpUS3DBeamwidthAndWeightFactorsInUSImageFrameTable5xM;

	/*! Transform matrix from the original image frame to the transducer origin (pixel). Constant transform, read from file */
	vtkTransform * TransformImageToTransducerOriginPixel;

  /*! The result of the calibration */
	vtkTransform * TransformImageToProbe;

  /*! TODO: clean this up */
  std::map<int, std::vector<double> > LineReconstructionErrors; 

protected: // From former Phantom class

	/*! The flag to be set when the PRE3Ds for validation positions are ready */
	bool mArePRE3DsForValidationPositionsReady;
	/*! The flag to be set when the independent point/line reconstruction errors are ready */
	bool mAreIndependentPointLineReconErrorsReady;

	/*!
    The US 3D beamwidth profile data and weight factors based on it
     1. This contains the interpolated US 3D beamwidth profile data at various axial depths
        as well as the weight factors calculated based on the beamwidth for each depth.
     2. We have three ways to incorporate the US beamidth to the calibration: Use the 
        variance of beamwidth (BWVar) to weight the calibration, use the beamwidth ratio
        (BWRatio) to weight the calibration, or use the beamwidth to threshold the input
        data (BWTHEVar) in order to eliminate potentially unreliable or error-prone data.
        This is determined by the choice of the input flag (Option-1, 2, or 3).
        [1] BWVar: This conforms to the standard way of applying weights to least squares, 
            where the weights should, ideally, be equal to the reciprocal of the variance of 
            the measurement of the data if they are uncorrelated.  Since we know the US beam
            width at a given axial depth, resonably assuming the data acquired by the sound 
            field is normally distributed, the standard deviation (Sigma) of the data can be
            roughly estimated as in the equation: Sigma = USBeamWidth/4 (for 95% of data).
	      [2] BWRatio: The fifth row of the matrix is the overall weight defined at that axial 
            depth. The weight factor was calculated using: CurrentBeamwidth/MinimumBeamwidth.
            The weight is inversely proportional to the weight factor, as obviously, the larger 
            the beamwidth, the less reliable the data is than those with the minimum beamwidth.  
            We found the weight factor to be a good indicator for how reliable the data is, 
            because the larger the beamwidth the larger the uncertainties and errors in data 
            acquired from that US field.  E.g., at the axial depth where the beamwidth is two 
            times that of minimum beamwidth, the uncertainties are doubled than the imaging 
            region that has the minimum beamwidth.
	      [3] BWTHEVar: This utilizes the beamwidth to quality control the input calibration 
            data, by filtering out those that has a larger beamwidth (e.g., larger than twice
            of the minimum beamwidth at the current imaging settings).  According to ultrasound
            physics, data acquired in the sound field that doubles the minimum beamwidth at
            the scanplane or elevation plane focal zone are typically much less reliable than
            those closer to the focal zone.  In addition, the filtered, remainng data would be
            weighted for calibration using their beamwidth (BWVar in [1]).
     FORMAT: each row of the matrix table has the following columns:
     [COL-0]:			Ascending Axial Depth in US Image Frame (in pixels);
     [COL-1 to 3]:	Beamwith in axial, lateral and elevational axes respectively (in mm);
     [COL-4]:			Weight Factor = CurrentBeamWidth/MininumBeamWidth (>=1).
  */
	vnl_matrix<double> mUS3DBeamwidthAndWeightFactorsInUSImageFrameTable5xM;

	/*! This keeps a copy of the original non-interpolated US beamwidth profile. Mainly for the purposes of logging and future reference */
	vnl_matrix<double> mOrigUS3DBeamwidthAndWeightFactorsInUSImageFrameTable5xN;

	/*!
    The Minimum US elevation beamwidth and the focal zone in US Image Frame
     1. For a typical 1-D linear-array transducer, the ultrasound beam can only be 
        focused mechanically in the elevation (out-of-plane) axis by placing an 
        ascoustic lens in front or curving the crystal surface.  
     2. The elevation beam pattern are therefore solely determined by the size and 
        curvature of the crystal or the acoustic lens, with sharp focus only 
        possible at a narrow axial distance (the focal zone) to the transducer.
     FORMAT: [FOCAL-ZONE (in US Image Frame), MINI-ELEVATION-WIDTH (in millimeters)]
  */
	vnl_vector<double> mMinimumUSElevationBeamwidthAndFocalZoneInUSImageFrame;

	/*! The nearest axial depth in the table (for fast access) */
	int mTheNearestAxialDepthInUSBeamwidthAndWeightTable;
  /*! The farthest axial depth in the table (for fast access) */
	int mTheFarestAxialDepthInUSBeamwidthAndWeightTable;

	/*! The 3D beamwidth elements at the nearest axial depth */
	vnl_vector<double> mUS3DBeamwidthAtNearestAxialDepth;
  /*! The 3D beamwidth elements at the farthest axial depth */
	vnl_vector<double> mUS3DBeamwidthAtFarestAxialDepth;

	/*! The flag to be set if beamwidth and weight factors are set. */
	bool mIsUSBeamwidthAndWeightFactorsTableReady;

	/*!
    Weights for the data positions defined by prior knowledge of the imaging condition.
	  E.g., the ultrasound 3D beamwidth in axial, lateral and elevational axes.
  */
	std::vector<double> mWeightsForDataPositions;

	/*! Data positions collected as inputs for the US calibration */
	std::vector< vnl_vector<double> > mDataPositionsInUSProbeFrame;
  /*! Data positions collected as inputs for the US calibration */
	std::vector< vnl_vector<double> > mDataPositionsInUSImageFrame;
  /*! Data positions collected as inputs for the US calibration */
	std::vector<int> mOutlierDataPositions; 

	/*! Validation positions collected to validate the calibration accuracy */
	std::vector< vnl_vector<double> > mValidationPositionsInUSProbeFrame;
  /*! Validation positions collected to validate the calibration accuracy */
	std::vector< vnl_vector<double> > mValidationPositionsInUSImageFrame;

	/*! Validation positions for point-line distance */
	std::vector< vnl_vector<double> > mValidationPositionsNWireStartInUSProbeFrame;
  /*! Validation positions for point-line distance */
	std::vector< vnl_vector<double> > mValidationPositionsNWireEndInUSProbeFrame;

	/*! Validation positions for parallel wires in US image frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire1InUSImageFrame;
  /*! Validation positions for parallel wires in US image frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire3InUSImageFrame;
  /*! Validation positions for parallel wires in US image frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire4InUSImageFrame;
  /*! Validation positions for parallel wires in US image frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire6InUSImageFrame;

	/*! Validation positions for parallel wires in US probe frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire1InUSProbeFrame;
  /*! Validation positions for parallel wires in US probe frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire3InUSProbeFrame;
  /*! Validation positions for parallel wires in US probe frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire4InUSProbeFrame;
  /*! Validation positions for parallel wires in US probe frame */
	std::vector< vnl_vector<double> > mValidationPositionsNWire6InUSProbeFrame;

  /*!
    Validation Positions to compute point-reconstruction errors (PREs)
	  NOTE: since the validation data set is separated from the calibration 
	  data and acquired before the calibration, the construction of
	  the validation data matrices needs to be done once and only 
	  once (e.g., before the 1st validation task is performed) with
	  the flag set to true.  This would save the system runtime
	  during the iterative calibration/validation process.	  
  */
	vnl_matrix<double> mValidationPositionsInUSImageFrameMatrix4xN;
  /*! Validation Positions to compute point-reconstruction errors (PREs) */
	vnl_matrix<double> mValidationPositionsInUSProbeFrameMatrix4xN;

	/*! Validation Positions to compute independent point-line distance errors (PLDEs) */
	vnl_matrix<double> mValidationPositionsNWireStartInUSProbeFrame3xN;
  /*! Validation Positions to compute independent point-line distance errors (PLDEs) */
	vnl_matrix<double> mValidationPositionsNWireEndInUSProbeFrame3xN;

	/*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire1InUSImageFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire3InUSImageFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire4InUSImageFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire6InUSImageFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire1InUSProbeFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire3InUSProbeFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire4InUSProbeFrame4xN;
  /*! Validation Positions to compute independent parallel line reconstruction errors (LREs) */
	vnl_matrix<double> mValidationPositionsNWire6InUSProbeFrame4xN;

	/*! Flag to set when the validation data matrices are populated */
	bool mAreValidationDataMatricesConstructed;

	/*!
    Final calibration transform in vnl_matrix format.
	  The homogeneous transform matrix from the US image frame to the US probe frame
  */
	vnl_matrix<double> mTransformUSImageFrame2USProbeFrameMatrix4x4;

  /*!
    Validation data confidence level.
    This sets the confidence level (trusted zone) as a percentage
    of the independent validation data used to produce the final
    validation results.  It serves as an effective way to get rid
    of corrupted data (or outliers) in the validation dataset.
    Default value: 0.95 (or 95%), meaning the top ranked 95% of 
    the ascendingly-ordered PRE3D values from the validation data 
    would be accepted as the valid PRE3D values.
  */
	double mValidationDataConfidenceLevel;

	/*!
    3D point reconstruction error (PRE3D) Analysis for the validation positions in the US probe frame
    FORMAT: (all positions are in the US probe frame)
    [ vector 0 - 2:  PRE3D_X_mean, PRE3D_X_rms, PRE3D_X_std ]
    [ vector 3 - 5:  PRE3D_Y_mean, PRE3D_Y_rms, PRE3D_Y_std ]
    [ vector 6 - 8:  PRE3D_Z_mean, PRE3D_Z_rms, PRE3D_Z_std ]
    [ vector 9	 :	Validation data confidence level ]
    where: 
    - mean: linearly averaged;
    - rms : root mean square;
    - std : standard deviation.
    - validation data confidence level: this is a percentage of 
     the independent validation data used to produce the final
     validation results.  It serves as an effective way to get 
     rid of corrupted data (or outliers) in the validation 
     dataset.  Default value: 0.95 (or 95%), meaning the top 
     ranked 95% of the ascendingly-ordered results from the 
     validation data would be accepted as the valid error values.
  */
	std::vector<double> PointReconstructionErrorAnalysisVector;

	/*!
    3D point reconstruction error (PRE3D) matrix for validation positions in US probe frame
    This keeps all the original PRE3Ds for the validation dataset with signs in
    the US probe frame (from the Projected positions to the true positions).
    FORMAT: matrix 4xN (with N being the total number of validation positions)
    [ Row-0: PRE3Ds in x-axis from projected to true positions ]
    [ Row-1: PRE3Ds in y-axis from projected to true positions ]
    [ Row-2: PRE3Ds in z-axis from projected to true positions ]
    [ Row-3: should be all zeros ]
    NOTE: this matrix can be obtained for statistical analysis if desired.
  */
	vnl_matrix<double> PointReconstructionErrorMatrix;

	/*!
    Sorted 3D point reconstruction error (PRE3D) matrix for validation positions in US probe frame
    This matrix sorts all the original PRE3Ds for the validation dataset with 
    signs in the US probe frame in a ascending order with respect the 
    absolute PRE3D root-squared value (length of the PRE3D vector or the 
    euclidean distance between the true positions and the project positions).
    FORMAT: matrix 4xN (with N being the total number of validation positions)
    [ Row-0: PRE3Ds in x-axis from projected to true positions ]
    [ Row-1: PRE3Ds in y-axis from projected to true positions ]
    [ Row-2: PRE3Ds in z-axis from projected to true positions ]
    [ Row-3: Euclidean PRE3D distance sorted in ascending order]
    NOTE: this matrix can be obtained for statistical analysis if desired.
  */
	vnl_matrix<double> mSortedRawPRE3DsInAscendingOrderInUSProbeFrameMatrix4xN;

	/*!
    Point-Line Distance Error for validation positions in US probe frame
    This contains the Point-Line Distance Error (PLDE) for the validation dataset. 
    The PLDE was defined as the absolute point-line distance from the projected
    positions to the N-Wire (the ground truth), both in the US probe frame.  
    If there was no error, the PLDE would be zero and the projected postions
    would reside right on the N-Wire.  The physical position of the N-Wire
    was measured based on the phantom geometry and converted into the US
    US probe frame by the optical tracking device affixed on the phantom.
    NOTE: this data may be used for statistical analysis if desired.
    FORMAT: vector 1xN (with N being the total number of validation positions)
  */
	vnl_vector<double> PointLineDistanceErrorVector;
  /*! Point-Line Distance Error for validation positions in US probe frame */
	vnl_vector<double> PointLineDistanceErrorSortedVector;

	/*!
    Point-Line Distance Error Analysis for Validation Positions in US probe frame
     FORMAT: (all positions are in the US probe frame)
     [ vector 0 - 2:  PLDE_mean, PLDE_rms, PLDE_std ]
     [ vector 3	 :	Validation data confidence level ]
     where: 
     - mean: linearly averaged;
     - rms : root mean square;
     - std : standard deviation.
     - validation data confidence level: this is a percentage of 
       the independent validation data used to produce the final
       validation results.  It serves as an effective way to get 
       rid of corrupted data (or outliers) in the validation 
       dataset.  Default value: 0.95 (or 95%), meaning the top 
       ranked 95% of the ascendingly-ordered results from the 
       validation data would be accepted as the valid error values.
  */
	std::vector<double> PointLineDistanceErrorAnalysisVector;

	/*!
    Line reconstruction error (LRE) matrix for validation positions in US probe frame
	   This keeps all the original PRE3Ds for the validation dataset with signs in
	   the US probe frame (from the Projected positions to the true positions).
	   FORMAT: matrix 4xN (with N being the total number of validation positions)
	   [ Row-0: PRE3Ds in x-axis from projected to true positions ]
	   [ Row-1: PRE3Ds in y-axis from projected to true positions ]
	   [ Row-2: should be all zeros ]
	   [ Row-3: Euclidean LRE distance ]
	   NOTE: this matrix can be obtained for statistical analysis if desired.
  */
	vnl_matrix<double> mNWire1LREOrigInUSProbeFrameMatrix4xN;
  /*! Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire3LREOrigInUSProbeFrameMatrix4xN;
  /*! Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire4LREOrigInUSProbeFrameMatrix4xN;
  /*! Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire6LREOrigInUSProbeFrameMatrix4xN;

	/*!
    Sorted Line reconstruction error (LRE) matrix for validation positions in US probe frame
	   This matrix sorts all the original LREs for the validation dataset with 
	   signs in the US probe frame in a ascending order with respect the 
	   absolute LRE root-squared value (length of the LRE vector or the 
	   euclidean distance between the true positions and the project positions).
	   FORMAT: matrix 4xN (with N being the total number of validation positions)
	   [ Row-0: LREs in x-axis from projected to true positions ]
	   [ Row-1: LREs in y-axis from projected to true positions ]
	   [ Row-2: should be all zeros ]
	   [ Row-3: Euclidean LRE distance sorted in ascending order]
	   NOTE: this matrix can be obtained for statistical analysis if desired.
  */
	vnl_matrix<double> mNWire1LRESortedAscendingInUSProbeFrameMatrix4xN;
  /*! Sorted Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire3LRESortedAscendingInUSProbeFrameMatrix4xN;
  /*! Sorted Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire4LRESortedAscendingInUSProbeFrameMatrix4xN;
  /*! Sorted Line reconstruction error (LRE) matrix for validation positions in US probe frame */
	vnl_matrix<double> mNWire6LRESortedAscendingInUSProbeFrameMatrix4xN;

	/*!
    Line Reconstruction Error Analysis for the validation positions in the US probe frame
	   FORMAT: (all positions are in the US probe frame)
	   For parallel NWires N1, N3, N4, N6:
	   [ vector 0 - 1:  LRE_X_mean,   LRE_X_std   ]
	   [ vector 2 - 3:  LRE_Y_mean,   LRE_Y_std   ]
	   [ vector 4 - 5:  LRE_EUC_mean, LRE_EUC_std ]
	   [ vector 6	 :	Validation data confidence level ]
	   where: 
	   - mean: linearly averaged;
	   - std : standard deviation;
	   - EUC: Euclidean (norm) measurement.
	   - validation data confidence level: this is a percentage of 
	     the independent validation data used to produce the final
	     validation results.  It serves as an effective way to get 
	     rid of corrupted data (or outliers) in the validation 
	     dataset.  Default value: 0.95 (or 95%), meaning the top 
	     ranked 95% of the ascendingly-ordered results from the 
	     validation data would be accepted as the valid error values.
  */
	std::vector<double> mNWire1AbsLREAnalysisInUSProbeFrame;
  /*! Line Reconstruction Error Analysis for the validation positions in the US probe frame */
	std::vector<double> mNWire3AbsLREAnalysisInUSProbeFrame;
  /*! Line Reconstruction Error Analysis for the validation positions in the US probe frame */
	std::vector<double> mNWire4AbsLREAnalysisInUSProbeFrame;
  /*! Line Reconstruction Error Analysis for the validation positions in the US probe frame */
	std::vector<double> mNWire6AbsLREAnalysisInUSProbeFrame;

	/*! This is the threshold to filter out input data acquired at large beamwidth */
	double mNumOfTimesOfMinBeamWidth;

private:
	vtkProbeCalibrationAlgo(const vtkProbeCalibrationAlgo&);
	void operator=(const vtkProbeCalibrationAlgo&);
};

#endif //  __vtkProbeCalibrationAlgo_h