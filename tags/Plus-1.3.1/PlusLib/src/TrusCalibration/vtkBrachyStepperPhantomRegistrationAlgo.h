/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/ 

#ifndef __vtkBrachyStepperPhantomRegistrationAlgo_h
#define __vtkBrachyStepperPhantomRegistrationAlgo_h

#include "PlusConfigure.h"
#include "vtkObject.h"
#include "vtkTrackedFrameList.h"
#include "vtkTable.h"
#include "FidPatternRecognitionCommon.h"

class vtkHTMLGenerator; 
class vtkGnuplotExecuter; 
class vtkTransform;

/*!
  \class vtkBrachyStepperPhantomRegistrationAlgo 
  \brief Phantom registration algorithm for image to probe calibration with brachy stepper 

  This algorithm determines the phantom to reference transform (the spatial relationship
  between the phantom and the stepper coordinate system).
  The images shall be taken of a calibration phantom and the frames shall be segmented
  (the fiducial point coordinates shall be computed) before calling this algorithm.

  \ingroup PlusLibCalibrationAlgorithm
*/ 
class VTK_EXPORT vtkBrachyStepperPhantomRegistrationAlgo : public vtkObject
{
public:

  static vtkBrachyStepperPhantomRegistrationAlgo *New();
  vtkTypeRevisionMacro(vtkBrachyStepperPhantomRegistrationAlgo , vtkObject);
  virtual void PrintSelf(ostream& os, vtkIndent indent); 

  /*!
    Set all algorithm inputs.
    \param trackedFrameList Tracked frames with segmentation results 
    \param spacing Image spacing (mm/px)
    \param centerOfRotationPx Ultrasound image rotation center in px
    \param nWires Phantom definition structure 
  */
  virtual void SetInputs( vtkTrackedFrameList* trackedFrameList, double spacing[2], double centerOfRotationPx[2], const std::vector<NWire>& nWires ); 

  /*! Get phantom to reference transform */
  virtual PlusStatus GetPhantomToReferenceTransform( vtkTransform* phantomToReferenceTransform);

  /*! \deprecated TODO: remove it just use it for getting the same result as the baseline */
  vtkSetObjectMacro(TransformTemplateHolderToPhantom, vtkTransform); 

  /*! \deprecated TODO: remove it just use it for getting the same result as the baseline */
  vtkGetObjectMacro(TransformTemplateHolderToPhantom, vtkTransform); 

  /*! \deprecated TODO: remove it just use it for getting the same result as the baseline */
  vtkGetObjectMacro(TransformReferenceToTemplateHolder, vtkTransform); 
  
protected:
  vtkBrachyStepperPhantomRegistrationAlgo();
  virtual ~vtkBrachyStepperPhantomRegistrationAlgo(); 

  /*! Bring this algorithm's outputs up-to-date. */
  virtual PlusStatus Update();

  /*! Set the input tracked frame list */
  vtkSetObjectMacro(TrackedFrameList, vtkTrackedFrameList); 

  /*! Get the input tracked frame list */
  vtkGetObjectMacro(TrackedFrameList, vtkTrackedFrameList); 

  /*! Set spacing */
  vtkSetVector2Macro(Spacing, double); 

  /*! Set center of rotation in px  */
  vtkSetVector2Macro(CenterOfRotationPx, double); 

  /*! Image spacing (mm/pixel). Spacing[0]: lateral axis, Spacing[1]: axial axis */
  double Spacing[2];

  /*! Rotation center position of the image in px */
  double CenterOfRotationPx[2]; 

  /*! Tracked frame list with segmentation results */
  vtkTrackedFrameList* TrackedFrameList; 

  /*! Phantom definition structure */
  std::vector<NWire> NWires; 

  /*! Phantom to Reference transform */
  vtkTransform* PhantomToReferenceTransform; 
  
  /*! \deprecated TODO: remove it! */
  vtkTransform* TransformTemplateHolderToPhantom; 

  /*! \deprecated TODO: remove it! */
  vtkTransform* TransformReferenceToTemplateHolder; 

  /*! When the results were computed. The result is recomputed only if the inputs changed more recently than UpdateTime. */
  vtkTimeStamp UpdateTime;  

}; 

#endif