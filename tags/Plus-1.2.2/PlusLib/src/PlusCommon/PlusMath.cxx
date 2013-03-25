#include "PlusMath.h"

#include "vnl/vnl_sparse_matrix_linear_system.h"  
#include "vnl/algo/vnl_lsqr.h"  
#include "vnl/vnl_cross.h"  

#include "vtkMath.h"
#include "vtkTransform.h"

//----------------------------------------------------------------------------
PlusMath::PlusMath()
{

}

//----------------------------------------------------------------------------
PlusMath::~PlusMath()
{

}

//----------------------------------------------------------------------------
PlusStatus PlusMath::LSQRMinimize(const std::vector< std::vector<double> > &aMatrix, const std::vector<double> &bVector, vnl_vector<double> &resultVector)
{
  LOG_TRACE("PlusMath::LSQRMinimize"); 

  if (aMatrix.size()==0)
  {
    LOG_ERROR("LSQRMinimize: A matrix is empty");
    resultVector.clear();
    return PLUS_FAIL;
  }
  if (bVector.size()==0)
  {
    LOG_ERROR("LSQRMinimize: b vector is empty");
    resultVector.clear();
    return PLUS_FAIL;
  }

  // The coefficient matrix aMatrix should be m-by-n and the column vector bVector must have length m.
  const int n = aMatrix.begin()->size(); 
  const int m = bVector.size();

  std::vector<vnl_vector<double>> aMatrixVnl(m); 
  vnl_vector<double> row(n); 
  for ( unsigned int i = 0; i < aMatrix.size(); ++i )
  {
    for ( unsigned int r = 0; r < aMatrix[i].size(); ++r )
    {
      row[r] = aMatrix[i][r]; 
    }
    aMatrixVnl.push_back(row); 
  }

  return PlusMath::LSQRMinimize(aMatrixVnl, bVector, resultVector); 
}

//----------------------------------------------------------------------------
PlusStatus PlusMath::LSQRMinimize(const std::vector<vnl_vector<double>> &aMatrix, const std::vector<double> &bVector, vnl_vector<double> &resultVector)
{
  LOG_TRACE("PlusMath::LSQRMinimize"); 

  if (aMatrix.size()==0)
  {
    LOG_ERROR("LSQRMinimize: A matrix is empty");
    resultVector.clear();
    return PLUS_FAIL;
  }
  if (bVector.size()==0)
  {
    LOG_ERROR("LSQRMinimize: b vector is empty");
    resultVector.clear();
    return PLUS_FAIL;
  }

  // The coefficient matrix aMatrix should be m-by-n and the column vector bVector must have length m.
  const int n = aMatrix.begin()->size(); 
  const int m = bVector.size();

  vnl_sparse_matrix<double> sparseMatrixLeftSide(m, n);
  vnl_vector<double> vectorRightSide(m);

  for(int row = 0; row < m; row++)
  {
    // Populate the sparse matrix
    for ( int i = 0; i < n; i++)
    {
      sparseMatrixLeftSide(row,i) = aMatrix[row].get(i);
    }

    // Populate the vector
    vectorRightSide.put(row, bVector[row]);
  }

  return PlusMath::LSQRMinimize(sparseMatrixLeftSide, vectorRightSide, resultVector); 
}


//----------------------------------------------------------------------------
PlusStatus PlusMath::LSQRMinimize(const vnl_sparse_matrix<double> &sparseMatrixLeftSide, const vnl_vector<double> &vectorRightSide, vnl_vector<double> &resultVector)
{
  LOG_TRACE("PlusMath::LSQRMinimize"); 

  // Construct linear system defined in VNL
  vnl_sparse_matrix_linear_system<double> linearSystem( sparseMatrixLeftSide, vectorRightSide );

  // Instantiate the LSQR solver
  vnl_lsqr lsqr( linearSystem );

  // call minimize on the solver
  int returnCode=lsqr.minimize( resultVector );

  switch (returnCode)
  {
  case 0: // x = 0  is the exact solution. No iterations were performed.
	return PLUS_SUCCESS;
  case 1: // The equations A*x = b are probably compatible.  "
		// Norm(A*x - b) is sufficiently small, given the "
		// "values of ATOL and BTOL.",
	  return PLUS_SUCCESS;
  case 2: // "The system A*x = b is probably not compatible.  "
		// "A least-squares solution has been obtained that is "
		// "sufficiently accurate, given the value of ATOL.",
	  return PLUS_SUCCESS;
  case 3: // "An estimate of cond(Abar) has exceeded CONLIM.  "
		//"The system A*x = b appears to be ill-conditioned.  "
		// "Otherwise, there could be an error in subroutine APROD.",
	  LOG_WARNING("LSQR fit may be inaccurate, CONLIM exceeded");
	  return PLUS_SUCCESS;
  case 4: // "The equations A*x = b are probably compatible.  "
		// "Norm(A*x - b) is as small as seems reasonable on this machine.",
	  return PLUS_SUCCESS;
  case 5: // "The system A*x = b is probably not compatible.  A least-squares "
		// "solution has been obtained that is as accurate as seems "
		// "reasonable on this machine.",
	  return PLUS_SUCCESS;
  case 6: // "Cond(Abar) seems to be so large that there is no point in doing further "
		// "iterations, given the precision of this machine. "
		// "There could be an error in subroutine APROD.",
	  LOG_ERROR("LSQR fit may be inaccurate, ill-conditioned matrix");
	  return PLUS_FAIL;
  case 7: // "The iteration limit ITNLIM was reached."
	  LOG_WARNING("LSQR fit may be inaccurate, ITNLIM was reached");
	  return PLUS_SUCCESS;
  default:
	  LOG_ERROR("Unkown LSQR return code "<<returnCode);
	  return PLUS_FAIL;
  }
}

//----------------------------------------------------------------------------
double PlusMath::GetPositionDifference(vtkMatrix4x4* aMatrix, vtkMatrix4x4* bMatrix)
{
  LOG_TRACE("PlusMath::GetPositionDifference"); 
  vtkSmartPointer<vtkTransform> aTransform = vtkSmartPointer<vtkTransform>::New(); 
  aTransform->SetMatrix(aMatrix); 

  vtkSmartPointer<vtkTransform> bTransform = vtkSmartPointer<vtkTransform>::New(); 
  bTransform->SetMatrix(bMatrix); 

  double ax = aTransform->GetPosition()[0]; 
  double ay = aTransform->GetPosition()[1]; 
  double az = aTransform->GetPosition()[2]; 

  double bx = bTransform->GetPosition()[0]; 
  double by = bTransform->GetPosition()[1]; 
  double bz = bTransform->GetPosition()[2]; 

  // Euclidean distance
  double distance = sqrt( pow(ax-bx,2) + pow(ay-by,2) + pow(az-bz,2) ); 

  return distance; 
}

//----------------------------------------------------------------------------
double PlusMath::GetOrientationDifference(vtkMatrix4x4* aMatrix, vtkMatrix4x4* bMatrix)
{
  LOG_TRACE("PlusMath::GetOrientationDifference"); 

  vtkSmartPointer<vtkMatrix4x4> diffMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 
  vtkSmartPointer<vtkMatrix4x4> invBmatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 

  vtkMatrix4x4::Invert(bMatrix, invBmatrix);  

  vtkMatrix4x4::Multiply4x4(aMatrix, invBmatrix, diffMatrix); 

  vtkSmartPointer<vtkTransform> diffTransform = vtkSmartPointer<vtkTransform>::New(); 
  diffTransform->SetMatrix(diffMatrix); 

  double angleDiff_rad= vtkMath::RadiansFromDegrees(diffTransform->GetOrientationWXYZ()[0]);

  double normalizedAngleDiff_rad = atan2( sin(angleDiff_rad), cos(angleDiff_rad) ); // normalize angle to domain -pi, pi 

  return vtkMath::DegreesFromRadians(normalizedAngleDiff_rad);
}

//----------------------------------------------------------------------------
// Spherical linear interpolation between two rotation quaternions.
// t is a value between 0 and 1 that interpolates between from and to (t=0 means the results is the same as "from").
// Precondition: no aliasing problems to worry about ("result" can be "from" or "to" param).
// Parameters: adjustSign - If true, then slerp will operate by adjusting the sign of the slerp to take shortest path
// References: From Adv Anim and Rendering Tech. Pg 364
void PlusMath::Slerp(double *result, double t, double *from, double *to, bool adjustSign /*= true*/) 	
{
  const double* p = from; // just an alias to match q

  // calc cosine theta
  double cosom = from[0]*to[0]+from[1]*to[1]+from[2]*to[2]+from[3]*to[3]; // dot( from, to )

  // adjust signs (if necessary)
  double q[4];
  if (adjustSign && (cosom < (double)0.0))
  {
    cosom = -cosom;
    q[0] = -to[0];   // Reverse all signs
    q[1] = -to[1];
    q[2] = -to[2];
    q[3] = -to[3];
  }
  else
  {
    q[0] = to[0];
    q[1] = to[1];
    q[2] = to[2];
    q[3] = to[3];
  }

  // Calculate coefficients
  double sclp, sclq;
  if (((double)1.0 - cosom) > (double)0.0001) // 0.0001 -> some epsillon
  {
    // Standard case (slerp)
    double omega, sinom;
    omega = acos( cosom ); // extract theta from dot product's cos theta
    sinom = sin( omega );
    sclp  = sin( ((double)1.0 - t) * omega ) / sinom;
    sclq  = sin( t * omega ) / sinom;
  }
  else
  {
    // Very close, do linear interp (because it's faster)
    sclp = (double)1.0 - t;
    sclq = t;
  }

  for (int i=0; i<4; i++)
  {
    result[i] = sclp * p[i] + sclq * q[i];
  }
}

//----------------------------------------------------------------------------

std::string PlusMath::GetTransformParametersString(vtkTransform* transform)
{
  double rotation[3];
  double translation[3];
  double scale[3];
  transform->GetOrientation(rotation);
  transform->GetPosition(translation);
  transform->GetScale(scale);

  std::ostringstream result;
  result << std::setprecision(4) << "Rotation: (" << rotation[0] << ", " << rotation[1] << ", " << rotation[2]
    << ")  Translation: (" << translation[0] << ", " << translation[1] << ", " << translation[2]
    << ")  Scale: (" << scale[0] << ", " << scale[1] << ", " << scale[2] << ")";

  return result.str();
}