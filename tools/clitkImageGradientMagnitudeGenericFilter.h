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
#ifndef CLITKImageGradientMagnitudeGENERICFILTER_H
#define CLITKImageGradientMagnitudeGENERICFILTER_H
#include "clitkIO.h"
#include "clitkImageToImageGenericFilter.h"

//--------------------------------------------------------------------
namespace clitk
{

template<class args_info_type>
class ITK_EXPORT ImageGradientMagnitudeGenericFilter:
        public ImageToImageGenericFilter<ImageGradientMagnitudeGenericFilter<args_info_type> >
{

public:

    //--------------------------------------------------------------------
    ImageGradientMagnitudeGenericFilter();

    //--------------------------------------------------------------------
    typedef ImageGradientMagnitudeGenericFilter         Self;
    typedef itk::SmartPointer<Self>                     Pointer;
    typedef itk::SmartPointer<const Self>               ConstPointer;

    //--------------------------------------------------------------------
    // Method for creation through the object factory
    // and Run-time type information (and related methods)
    itkNewMacro(Self);
    itkTypeMacro(ImageGradientMagnitudeGenericFilter, LightObject);

    //--------------------------------------------------------------------
    void SetArgsInfo(const args_info_type & a);

    //--------------------------------------------------------------------
    // Main function called each time the filter is updated
    template<class InputImageType>
    void UpdateWithInputImageType();

protected:
    template<unsigned int Dim> void InitializeImageType();
    args_info_type mArgsInfo;

private:
    bool m_NormalizeOutput;

}; // end class
//--------------------------------------------------------------------

} // end namespace clitk

#ifndef ITK_MANUAL_INSTANTIATION
#include "clitkImageGradientMagnitudeGenericFilter.txx"
#endif

#endif // #define clitkImageGradientMagnitudeGenericFilter_h
