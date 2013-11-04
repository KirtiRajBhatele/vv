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
===========================================================================**/
#ifndef CLITKImageLaplacianGENERICFILTER_H
#define CLITKImageLaplacianGENERICFILTER_H
#include "clitkIO.h"
#include "clitkImageToImageGenericFilter.h"

//--------------------------------------------------------------------
namespace clitk
{

template<class args_info_type>
class ITK_EXPORT ImageLaplacianGenericFilter:
        public ImageToImageGenericFilter<ImageLaplacianGenericFilter<args_info_type> >
{

public:

    //--------------------------------------------------------------------
    ImageLaplacianGenericFilter();

    //--------------------------------------------------------------------
    typedef ImageLaplacianGenericFilter         Self;
    typedef itk::SmartPointer<Self>            Pointer;
    typedef itk::SmartPointer<const Self>      ConstPointer;

    //--------------------------------------------------------------------
    // Method for creation through the object factory
    // and Run-time type information (and related methods)
    itkNewMacro(Self);
    itkTypeMacro(ImageLaplacianGenericFilter, LightObject);

    //--------------------------------------------------------------------
    void SetArgsInfo(const args_info_type & a);

    //--------------------------------------------------------------------
    // Main function called each time the filter is updated
    template<class InputImageType>
    void UpdateWithInputImageType();

protected:
    template<unsigned int Dim> void InitializeImageType();
    args_info_type mArgsInfo;

}; // end class
//--------------------------------------------------------------------

} // end namespace clitk

#ifndef ITK_MANUAL_INSTANTIATION
#include "clitkImageLaplacianGenericFilter.txx"
#endif

#endif // #define clitkImageLaplacianGenericFilter_h
