/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://www.centreleonberard.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
  ======================================================================-====*/

#ifndef CLITKBINARYIMAGETOMESHFILTER_H
#define CLITKMESHTOBINARYIMAGEFILTER_H

// clitk
#include "clitkCommon.h"
#include "vvFromITK.h"

// itk
#include <itkProcessObject.h>

// vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkImageClip.h>
#include <vtkMarchingSquares.h>
#include <vtkAppendPolyData.h>
#include <vtkDecimatePro.h>
#include <vtkStripper.h>

namespace clitk {
    
  /* --------------------------------------------------------------------     
     Convert a 3D binary image into a list of 2D contours (vtkpolydata)
     -------------------------------------------------------------------- */
  
  template<class ImageType>
  class ITK_EXPORT BinaryImageToMeshFilter:public itk::Object
  {

  public:
    /** Standard class typedefs. */
    typedef itk::ProcessObject            Superclass;
    typedef BinaryImageToMeshFilter       Self;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
       
    /** Method for creation through the object factory. */
    itkNewMacro(Self);
 
    /** Run-time type information (and related methods). */
    itkTypeMacro(BinaryImageToMeshFilter, Superclass);

    /** ImageDimension constants */
    itkStaticConstMacro(ImageDimension, unsigned int, ImageType::ImageDimension);

    // /** Some convenient typedefs. */
    typedef typename ImageType::Pointer      ImagePointer;
    typedef typename ImageType::PixelType    PixelType;

    itkSetMacro(Input, ImagePointer);
    itkGetConstMacro(Input, ImagePointer);
    itkGetMacro(OutputMesh, vtkSmartPointer<vtkPolyData>);
    itkSetMacro(ThresholdValue, PixelType);

    virtual void Update();

  protected:
    BinaryImageToMeshFilter();
    virtual ~BinaryImageToMeshFilter() {}
    
    ImagePointer m_Input;
    vtkSmartPointer<vtkPolyData> m_OutputMesh;
    PixelType m_ThresholdValue;

  private:
    BinaryImageToMeshFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented
    
  }; // end class
  //--------------------------------------------------------------------

} // end namespace clitk
//--------------------------------------------------------------------

#ifndef ITK_MANUAL_INSTANTIATION
#include "clitkBinaryImageToMeshFilter.txx"
#endif

#endif
