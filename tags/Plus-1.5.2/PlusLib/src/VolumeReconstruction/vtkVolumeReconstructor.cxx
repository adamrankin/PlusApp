/*=Plus=header=begin======================================================
  Program: Plus
  Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
  See License.txt for details.
=========================================================Plus=header=end*/

#include "PlusConfigure.h"

#include <limits>

#include "vtkImageImport.h" 
#include "vtkImageData.h" 
#include "vtkImageViewer.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkTimerLog.h"
#include "vtkTrackerTool.h"
#include "vtkTrackerBuffer.h"
#include "vtkTransform.h"
#include "vtkVideoBuffer.h"
#include "vtkVolumeReconstructor.h"
#include "vtkXMLUtilities.h"
#include "vtkImageExtractComponents.h"

#include "vtkPasteSliceIntoVolume.h"
#include "vtkFillHolesInVolume.h"
#include "vtkTrackedFrameList.h"
#include "TrackedFrame.h"
#include "vtkTransformRepository.h"

vtkCxxRevisionMacro(vtkVolumeReconstructor, "$Revisions: 1.0 $");
vtkStandardNewMacro(vtkVolumeReconstructor);

//----------------------------------------------------------------------------
vtkVolumeReconstructor::vtkVolumeReconstructor()
{
  this->Reconstructor = vtkPasteSliceIntoVolume::New();  
  this->HoleFiller = vtkFillHolesInVolume::New();  
  this->FillHoles=0;
  this->SkipInterval=1;
}

//----------------------------------------------------------------------------
vtkVolumeReconstructor::~vtkVolumeReconstructor()
{
  if (this->Reconstructor!=NULL)
  {
    this->Reconstructor->Delete();
    this->Reconstructor=NULL;
    this->HoleFiller->Delete();
    this->HoleFiller=NULL;
  }
}

//----------------------------------------------------------------------------
void vtkVolumeReconstructor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
PlusStatus vtkVolumeReconstructor::ReadConfiguration(vtkXMLDataElement* config)
{
  vtkXMLDataElement* reconConfig = config->FindNestedElementWithName("VolumeReconstruction");
  if (reconConfig == NULL)
  {
    LOG_ERROR("No volume reconstruction is found in the XML tree!");
    return PLUS_FAIL;
  }

  // output volume parameters
  // origin and spacing is defined in the reference coordinate system
  double outputSpacing[3]={0,0,0};
  if (reconConfig->GetVectorAttribute("OutputSpacing", 3, outputSpacing))
  {
    this->Reconstructor->SetOutputSpacing(outputSpacing);
  }
  else
  {
    LOG_ERROR("OutputSpacing parameter is not found!");
    return PLUS_FAIL;
  }
  double outputOrigin[3]={0,0,0};
  if (reconConfig->GetVectorAttribute("OutputOrigin", 3, outputOrigin))
  {
    this->Reconstructor->SetOutputOrigin(outputOrigin);
  }
  int outputExtent[6]={0,0,0,0,0,0};
  if (reconConfig->GetVectorAttribute("OutputExtent", 6, outputExtent))
  {
    this->Reconstructor->SetOutputExtent(outputExtent);
  }

  // clipping parameters
  int clipRectangleOrigin[2]={0,0};
  if (reconConfig->GetVectorAttribute("ClipRectangleOrigin", 2, clipRectangleOrigin))
  {
    this->Reconstructor->SetClipRectangleOrigin(clipRectangleOrigin);
  }
  int clipRectangleSize[2]={0,0};
  if (reconConfig->GetVectorAttribute("ClipRectangleSize", 2, clipRectangleSize))
  {
    this->Reconstructor->SetClipRectangleSize(clipRectangleSize);
  }

  // fan parameters
  double fanAngles[2]={0,0};
  if (reconConfig->GetVectorAttribute("FanAngles", 2, fanAngles))
  {
    this->Reconstructor->SetFanAngles(fanAngles);
  }
  double fanOrigin[2]={0,0};
  if (reconConfig->GetVectorAttribute("FanOrigin", 2, fanOrigin))
  {
    this->Reconstructor->SetFanOrigin(fanOrigin);
  }
  double fanDepth=0;
  if (reconConfig->GetScalarAttribute("FanDepth", fanDepth))
  {
    this->Reconstructor->SetFanDepth(fanDepth);
  }

  if (reconConfig->GetScalarAttribute("SkipInterval",SkipInterval))
  {
    if (SkipInterval < 1) {
      LOG_WARNING("SkipInterval in the config file must be greater or equal to 1. Resetting to 1");
      SkipInterval = 1;
    }
  }

  // reconstruction options
  if (reconConfig->GetAttribute("Interpolation"))
  {
    if (STRCASECMP(reconConfig->GetAttribute("Interpolation"),
      this->Reconstructor->GetInterpolationModeAsString(vtkPasteSliceIntoVolume::LINEAR_INTERPOLATION)) == 0)
    {
      this->Reconstructor->SetInterpolationMode(vtkPasteSliceIntoVolume::LINEAR_INTERPOLATION);
    }
    else if (STRCASECMP(reconConfig->GetAttribute("Interpolation"), 
      this->Reconstructor->GetInterpolationModeAsString(vtkPasteSliceIntoVolume::NEAREST_NEIGHBOR_INTERPOLATION)) == 0)
    {
      this->Reconstructor->SetInterpolationMode(vtkPasteSliceIntoVolume::NEAREST_NEIGHBOR_INTERPOLATION);
    }
    else
    {
      LOG_ERROR("Unknown interpolation option: "<<reconConfig->GetAttribute("Interpolation")<<". Valid options: LINEAR, NEAREST_NEIGHBOR.");
    }
  }
  if (reconConfig->GetAttribute("Calculation"))
  {
    if (STRCASECMP(reconConfig->GetAttribute("Calculation"),
      this->Reconstructor->GetCalculationModeAsString(vtkPasteSliceIntoVolume::WEIGHTED_AVERAGE)) == 0)
    {
      this->Reconstructor->SetCalculationMode(vtkPasteSliceIntoVolume::WEIGHTED_AVERAGE);
    }
    else if (STRCASECMP(reconConfig->GetAttribute("Calculation"), 
      this->Reconstructor->GetCalculationModeAsString(vtkPasteSliceIntoVolume::MAXIMUM)) == 0)
    {
      this->Reconstructor->SetCalculationMode(vtkPasteSliceIntoVolume::MAXIMUM);
    }
    else
    {
      LOG_ERROR("Unknown calculation option: "<<reconConfig->GetAttribute("Calculation")<<". Valid options: WEIGHTED_AVERAGE, MAXIMUM.");
    }
  }
  if (reconConfig->GetAttribute("Optimization"))
  {
    if (STRCASECMP(reconConfig->GetAttribute("Optimization"), 
      this->Reconstructor->GetOptimizationModeAsString(vtkPasteSliceIntoVolume::FULL_OPTIMIZATION)) == 0)
    {
      this->Reconstructor->SetOptimization(vtkPasteSliceIntoVolume::FULL_OPTIMIZATION);
    }
    else if (STRCASECMP(reconConfig->GetAttribute("Optimization"), 
      this->Reconstructor->GetOptimizationModeAsString(vtkPasteSliceIntoVolume::PARTIAL_OPTIMIZATION)) == 0)
    {
      this->Reconstructor->SetOptimization(vtkPasteSliceIntoVolume::PARTIAL_OPTIMIZATION);
    }
    else if (STRCASECMP(reconConfig->GetAttribute("Optimization"), 
      this->Reconstructor->GetOptimizationModeAsString(vtkPasteSliceIntoVolume::NO_OPTIMIZATION)) == 0)
    {
      this->Reconstructor->SetOptimization(vtkPasteSliceIntoVolume::NO_OPTIMIZATION);
    }
    else
    {
      LOG_ERROR("Unknown optimization option: "<<reconConfig->GetAttribute("Optimization")<<". Valid options: FULL, PARTIAL, NONE.");
    }
  }
  if (reconConfig->GetAttribute("Compounding"))
  {
    ((STRCASECMP(reconConfig->GetAttribute("Compounding"), "On") == 0) ? 
      this->Reconstructor->SetCompounding(1) : this->Reconstructor->SetCompounding(0));
  }

  int numberOfThreads=0;
  if (reconConfig->GetScalarAttribute("NumberOfThreads", numberOfThreads))
  {
    this->Reconstructor->SetNumberOfThreads(numberOfThreads);
	this->HoleFiller->SetNumberOfThreads(numberOfThreads);
  }

  int fillHoles=0;
  if (reconConfig->GetAttribute("FillHoles"))
  {
    if (STRCASECMP(reconConfig->GetAttribute("FillHoles"), "On") == 0) 
    {
      this->FillHoles=1;
    }
    else
    {
      this->FillHoles=0;
    }
  }

  // Find and read kernels. First for loop counts the number of kernels to allocate, second for loop stores them
  if (this->FillHoles) 
  {
    // load input for kernel size, stdev, etc...
    vtkXMLDataElement* holeFilling = reconConfig->FindNestedElementWithName("HoleFilling");
    if ( this->FillHoles == 1 && holeFilling == NULL )
    {
      LOG_ERROR("Couldn't locate kernel parameters for hole filling!");
      return PLUS_FAIL;
    }
    // find the number of kernels
	int numKernels(0);
    for ( int nestedElementIndex = 0; nestedElementIndex < holeFilling->GetNumberOfNestedElements(); ++nestedElementIndex )
    {
      vtkXMLDataElement* nestedElement = holeFilling->GetNestedElement(nestedElementIndex);
      if ( STRCASECMP(nestedElement->GetName(), "Kernel" ) != 0 )
      {
        // Not a kernel element, skip it
        continue; 
      }
	  numKernels++;
	}
	// read each kernel into memory
	HoleFiller->SetNumKernels(numKernels);
	HoleFiller->AllocateKernels();
    int numberOfErrors(0);
	int currentKernelIndex(0);
    for ( int nestedElementIndex = 0; nestedElementIndex < holeFilling->GetNumberOfNestedElements(); ++nestedElementIndex )
	{
      vtkXMLDataElement* nestedElement = holeFilling->GetNestedElement(nestedElementIndex);
      if ( STRCASECMP(nestedElement->GetName(), "Kernel" ) != 0 )
      {
        // Not a kernel element, skip it
        continue; 
      }
      vtkFillHolesInVolumeKernel tempKernel;
	  // read size
      int size[3]={0}; 
      if ( nestedElement->GetVectorAttribute("Size", 3, size) )
      {
        tempKernel.size[0] = size[0];
        tempKernel.size[1] = size[1];
        tempKernel.size[2] = size[2];
      }
      else
      {
        LOG_ERROR("Unable to find \"Size\" attribute of kernel[" << nestedElementIndex <<"]"); 
        numberOfErrors++; 
        continue; 
      }
	  // read stdev
      float stdev[3]={0}; 
      if ( nestedElement->GetVectorAttribute("Stdev", 3, stdev) )
      {
        tempKernel.stdev[0] = stdev[0]; 
        tempKernel.stdev[1] = stdev[1]; 
        tempKernel.stdev[2] = stdev[2]; 
      }
      else
      {
        LOG_ERROR("Unable to find \"Stdev\" attribute of kernel[" << nestedElementIndex <<"]"); 
        numberOfErrors++; 
        continue; 
      }
      // read minRatio
      float minRatio = 0.; 
      if ( nestedElement->GetScalarAttribute("MinimumKnownVoxelsRatio", minRatio) )
      {
        tempKernel.minRatio = minRatio; 
      }
      else
      {
        LOG_ERROR("Unable to find \"MinimumKnownVoxelsRatio\" attribute of kernel[" << nestedElementIndex <<"]"); 
        numberOfErrors++; 
        continue; 
      }
	  HoleFiller->SetKernel(currentKernelIndex,tempKernel);
	  currentKernelIndex++;
    }
    if (numberOfErrors != 0) return PLUS_FAIL;
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
// Get the XML element describing the freehand object
PlusStatus vtkVolumeReconstructor::WriteConfiguration(vtkXMLDataElement *config)
{
  if ( config == NULL )
	{
		LOG_ERROR("Unable to write configuration from volume reconstructor! (XML data element is NULL)"); 
		return PLUS_FAIL; 
	}

	vtkXMLDataElement* reconConfig = config->FindNestedElementWithName("VolumeReconstruction");
	if (reconConfig == NULL)
  {
    vtkSmartPointer<vtkXMLDataElement> newReconConfig = vtkSmartPointer<vtkXMLDataElement>::New();
    newReconConfig->SetName("VolumeReconstruction");
    config->AddNestedElement(newReconConfig);
    reconConfig = config->FindNestedElementWithName("VolumeReconstruction");
    if (reconConfig == NULL)
    {
      LOG_ERROR("Failed to add VolumeReconstruction element");
		  return PLUS_FAIL;
    }
	}

	// output parameters
  reconConfig->SetVectorAttribute("OutputSpacing", 3, this->Reconstructor->GetOutputSpacing());
	reconConfig->SetVectorAttribute("OutputOrigin", 3, this->Reconstructor->GetOutputOrigin());
	reconConfig->SetVectorAttribute("OutputExtent", 6, this->Reconstructor->GetOutputExtent());

	// clipping parameters
	reconConfig->SetVectorAttribute("ClipRectangleOrigin", 2, this->Reconstructor->GetClipRectangleOrigin());
	reconConfig->SetVectorAttribute("ClipRectangleSize", 2, this->Reconstructor->GetClipRectangleSize());

	// fan parameters
  if (this->Reconstructor->FanClippingApplied())
  {
	  reconConfig->SetVectorAttribute("FanAngles", 2, this->Reconstructor->GetFanAngles());
	  reconConfig->SetVectorAttribute("FanOrigin", 2, this->Reconstructor->GetFanOrigin());
	  reconConfig->SetDoubleAttribute("FanDepth", this->Reconstructor->GetFanDepth());
  }
  else
  {
    reconConfig->RemoveAttribute("FanAngles");
    reconConfig->RemoveAttribute("FanOrigin");
    reconConfig->RemoveAttribute("FanDepth");
  }

	// reconstruction options
  reconConfig->SetAttribute("Interpolation", this->Reconstructor->GetInterpolationModeAsString(this->Reconstructor->GetInterpolationMode()));
  reconConfig->SetAttribute("Optimization", this->Reconstructor->GetOptimizationModeAsString(this->Reconstructor->GetOptimization()));
  reconConfig->SetAttribute("Compounding", this->Reconstructor->GetCompounding()?"On":"Off");

  if (this->Reconstructor->GetNumberOfThreads()>0)
  {
    reconConfig->SetIntAttribute("NumberOfThreads", this->Reconstructor->GetNumberOfThreads());
  }
  else
  {
    reconConfig->RemoveAttribute("NumberOfThreads");
  }
  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
void vtkVolumeReconstructor::AddImageToExtent( vtkImageData *image, vtkMatrix4x4* mImageToReference, double* extent_Ref)
{
  // Output volume is in the Reference coordinate system.

  // Prepare the four corner points of the input US image.
  int* frameExtent=image->GetExtent();
  std::vector< double* > corners_ImagePix;
  double c0[ 4 ] = { frameExtent[ 0 ], frameExtent[ 2 ], 0,  1 };
  double c1[ 4 ] = { frameExtent[ 0 ], frameExtent[ 3 ], 0,  1 };
  double c2[ 4 ] = { frameExtent[ 1 ], frameExtent[ 2 ], 0,  1 };
  double c3[ 4 ] = { frameExtent[ 1 ], frameExtent[ 3 ], 0,  1 };
  corners_ImagePix.push_back( c0 );
  corners_ImagePix.push_back( c1 );
  corners_ImagePix.push_back( c2 );
  corners_ImagePix.push_back( c3 );

  // Transform the corners to Reference and expand the extent if needed
  for ( unsigned int corner = 0; corner < corners_ImagePix.size(); ++corner )
  {
    double corner_Ref[ 4 ] = { 0, 0, 0, 1 }; // position of the corner in the Reference coordinate system
    mImageToReference->MultiplyPoint( corners_ImagePix[corner], corner_Ref );

    for ( int axis = 0; axis < 3; axis ++ )
    {
      if ( corner_Ref[axis] < extent_Ref[axis*2] )
      {
        // min extent along this coord axis has to be decreased
        extent_Ref[axis*2]=corner_Ref[axis];
      }
      if ( corner_Ref[axis] > extent_Ref[axis*2+1] )
      {
        // max extent along this coord axis has to be increased
        extent_Ref[axis*2+1]=corner_Ref[axis];
      }
    }
  }
} 

//----------------------------------------------------------------------------
PlusStatus vtkVolumeReconstructor::SetOutputExtentFromFrameList(vtkTrackedFrameList* trackedFrameList, vtkTransformRepository* transformRepository, PlusTransformName& imageToReferenceTransformName)
{
  if ( trackedFrameList == NULL )
  {
    LOG_ERROR("Failed to set output extent from tracked frame list - input frame list is NULL!"); 
    return PLUS_FAIL; 
  }
  if ( trackedFrameList->GetNumberOfTrackedFrames() == 0)
  {
    LOG_ERROR("Failed to set output extent from tracked frame list - input frame list is empty!"); 
    return PLUS_FAIL; 
  }

  if ( transformRepository == NULL )
  {
    LOG_ERROR("Failed to set output extent from tracked frame list - input transform repository is NULL!"); 
    return PLUS_FAIL; 
  }

  double extent_Ref[6]=
  {
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN,
    VTK_DOUBLE_MAX, VTK_DOUBLE_MIN
  };

  const int numberOfFrames = trackedFrameList->GetNumberOfTrackedFrames();
  for (int frameIndex = 0; frameIndex < numberOfFrames; ++frameIndex )
  {
    TrackedFrame* frame = trackedFrameList->GetTrackedFrame( frameIndex );
    
    if ( transformRepository->SetTransforms(*frame) != PLUS_SUCCESS )
    {
      LOG_ERROR("Failed to update transform repository with tracked frame!"); 
      return PLUS_FAIL; 
    }

    // Get transform
    bool isMatrixValid(false); 
    vtkSmartPointer<vtkMatrix4x4> imageToReferenceTransformMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
    if ( transformRepository->GetTransform(imageToReferenceTransformName, imageToReferenceTransformMatrix, &isMatrixValid ) != PLUS_SUCCESS )
    {
      std::string strImageToReferenceTransformName; 
      imageToReferenceTransformName.GetTransformName(strImageToReferenceTransformName); 
      LOG_ERROR("Failed to get transform '"<<strImageToReferenceTransformName<<"' from transform repository!"); 
      return PLUS_FAIL; 
    }

    if ( isMatrixValid )
    {
      // Get image (only the frame extents will be used)
      vtkImageData* frameImage=trackedFrameList->GetTrackedFrame(frameIndex)->GetImageData()->GetVtkImage();

      // Expand the extent_Ref to include this frame
      AddImageToExtent(frameImage, imageToReferenceTransformMatrix, extent_Ref);
    }
  }

  // Set the output extent from the current min and max values, using the user-defined image resolution.

  int outputExtent[ 6 ] = { 0, 0, 0, 0, 0, 0 };
  double* outputSpacing = this->Reconstructor->GetOutputSpacing();
  outputExtent[ 1 ] = int( ( extent_Ref[1] - extent_Ref[0] ) / outputSpacing[ 0 ] );
  outputExtent[ 3 ] = int( ( extent_Ref[3] - extent_Ref[2] ) / outputSpacing[ 1 ] );
  outputExtent[ 5 ] = int( ( extent_Ref[5] - extent_Ref[4] ) / outputSpacing[ 2 ] );

  this->Reconstructor->SetOutputScalarMode(trackedFrameList->GetTrackedFrame(0)->GetImageData()->GetVtkImage()->GetScalarType());
  this->Reconstructor->SetOutputExtent( outputExtent );
  this->Reconstructor->SetOutputOrigin( extent_Ref[0], extent_Ref[2], extent_Ref[4] ); 
  try
  {
    if (this->Reconstructor->ResetOutput()!=PLUS_SUCCESS) // :TODO: call this automatically
    {
      LOG_ERROR("Failed to initialize output of the reconstructor");
      return PLUS_FAIL;
    }
  }
  catch(vtkstd::bad_alloc& e)
  {
    cerr << e.what() << endl;
    LOG_ERROR("StartReconstruction failed due to out of memory. Try to reduce the size or increase spacing of the output volume.");
    return PLUS_FAIL;
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkVolumeReconstructor::AddTrackedFrame(TrackedFrame* frame, vtkTransformRepository* transformRepository, PlusTransformName& imageToReferenceTransformName, bool* insertedIntoVolume/*=NULL*/)
{
  if ( frame == NULL )
  {
    LOG_ERROR("Failed to add tracked frame to volume - input frame is NULL!"); 
    return PLUS_FAIL; 
  }

  if ( transformRepository == NULL )
  {
    LOG_ERROR("Failed to add tracked frame to volume - input transform repository is NULL!"); 
    return PLUS_FAIL; 
  }

  bool isMatrixValid(false); 
  vtkSmartPointer<vtkMatrix4x4> imageToReferenceTransformMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
  if ( transformRepository->GetTransform(imageToReferenceTransformName, imageToReferenceTransformMatrix, &isMatrixValid ) != PLUS_SUCCESS )
  {
    std::string strImageToReferenceTransformName; 
    imageToReferenceTransformName.GetTransformName(strImageToReferenceTransformName); 
    LOG_ERROR("Failed to get transform '"<<strImageToReferenceTransformName<<"' from transform repository!"); 
    return PLUS_FAIL; 
  }

  if ( insertedIntoVolume != NULL )
  {
    *insertedIntoVolume = isMatrixValid; 
  }

  if ( !isMatrixValid )
  {
    // Insert only valid frame into volume 
    return PLUS_SUCCESS; 
  }

  vtkImageData* frameImage=frame->GetImageData()->GetVtkImage();

  return this->Reconstructor->InsertSlice(frameImage, imageToReferenceTransformMatrix);
}

//----------------------------------------------------------------------------
PlusStatus vtkVolumeReconstructor::GetReconstructedVolume(vtkImageData* reconstructedVolume)
{  
  vtkSmartPointer<vtkImageExtractComponents> extract = vtkSmartPointer<vtkImageExtractComponents>::New();          
  // keep only first component (the other component is the alpha channel)
  extract->SetComponents(0);
  if (this->FillHoles)
  {
    HoleFiller->SetReconstructedVolume(this->Reconstructor->GetReconstructedVolume());
    HoleFiller->SetAccumulationBuffer(this->Reconstructor->GetAccumulationBuffer());
    HoleFiller->Update();
    extract->SetInput(HoleFiller->GetOutput());
  }
  else
  {
    extract->SetInput(this->Reconstructor->GetReconstructedVolume());
  }
  extract->Update();
  reconstructedVolume->ShallowCopy(extract->GetOutput());
  return PLUS_SUCCESS;
}

PlusStatus vtkVolumeReconstructor::GetReconstructedVolumeAlpha(vtkImageData* reconstructedVolume)
{
  vtkSmartPointer<vtkImageExtractComponents> extract = vtkSmartPointer<vtkImageExtractComponents>::New();          
  // extract the second component (the alpha channel)
  extract->SetComponents(1);
  extract->SetInput(this->Reconstructor->GetReconstructedVolume());
  extract->Update();
  reconstructedVolume->DeepCopy(extract->GetOutput());
  return PLUS_SUCCESS;
}