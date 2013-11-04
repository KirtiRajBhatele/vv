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

#ifndef CLITKCONNECTEDCOMPONENTLABELINGSGENERICFILTER_TXX
#define CLITKCONNECTEDCOMPONENTLABELINGSGENERICFILTER_TXX

// clitk
#include "clitkImageCommon.h"
#include "clitkSegmentationUtils.h"

// itk
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
  
//--------------------------------------------------------------------
template<class ArgsInfoType>
clitk::ConnectedComponentLabelingGenericFilter<ArgsInfoType>::ConnectedComponentLabelingGenericFilter():
  ImageToImageGenericFilter<Self>("ConnectedComponentLabeling") 
{
  InitializeImageType<2>();
  InitializeImageType<3>();
  //InitializeImageType<4>();
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template<class ArgsInfoType>
template<unsigned int Dim>
void clitk::ConnectedComponentLabelingGenericFilter<ArgsInfoType>::InitializeImageType() 
{  
  ADD_IMAGE_TYPE(Dim, uchar);
  ADD_IMAGE_TYPE(Dim, short);
  ADD_IMAGE_TYPE(Dim, ushort);
  // ADD_IMAGE_TYPE(Dim, int);
  ADD_IMAGE_TYPE(Dim, float);
}
//--------------------------------------------------------------------
  

//--------------------------------------------------------------------
template<class ArgsInfoType>
void clitk::ConnectedComponentLabelingGenericFilter<ArgsInfoType>::SetArgsInfo(const ArgsInfoType & a) 
{
  mArgsInfo=a;
  this->SetIOVerbose(mArgsInfo.verbose_flag);
  if (mArgsInfo.imagetypes_flag) this->PrintAvailableImageTypes();
  if (mArgsInfo.input_given)   this->AddInputFilename(mArgsInfo.input_arg);
  if (mArgsInfo.output_given)  this->SetOutputFilename(mArgsInfo.output_arg);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
// Update with the number of dimensions and the pixeltype
//--------------------------------------------------------------------
template<class ArgsInfoType>
template<class ImageType>
void clitk::ConnectedComponentLabelingGenericFilter<ArgsInfoType>::UpdateWithInputImageType() 
{ 
  // Reading input
  typename ImageType::Pointer input = this->template GetInput<ImageType>(0);

  // Output image type
  typedef itk::Image<int, ImageType::ImageDimension> OutputImageType;
  //PrintMemory(true, "initial");

  typename OutputImageType::Pointer output;
  {
    typename OutputImageType::Pointer temp;
    {
      // Create CCL filter
      typedef itk::ConnectedComponentImageFilter<ImageType, OutputImageType> ConnectFilterType;
      typename ConnectFilterType::Pointer connectFilter = ConnectFilterType::New();
      // connectFilter->ReleaseDataFlagOn(); // release earlier
      connectFilter->SetInput(input);
      connectFilter->SetBackgroundValue(mArgsInfo.inputBG_arg);
      connectFilter->SetFullyConnected(mArgsInfo.full_flag);
      //          connectFilter->SetNumberOfThreads(8);
      connectFilter->Update();
      temp = connectFilter->GetOutput();
      // PrintMemory(true, "after udpate");
    }
    // PrintMemory(true, "after CCL block");
    // DD(input->GetReferenceCount());
    // DD(temp->GetReferenceCount());
    
    // Sort by size and remove too small area.
    typedef itk::RelabelComponentImageFilter<OutputImageType, OutputImageType> RelabelFilterType;
    typename RelabelFilterType::Pointer relabelFilter = RelabelFilterType::New();
    //    relabelFilter->SetInput(connectFilter->GetOutput());
    relabelFilter->SetInput(temp);
    relabelFilter->SetMinimumObjectSize(mArgsInfo.minSize_arg);
    relabelFilter->Update();
    
    // DD(mArgsInfo.inputBG_arg);
    // DD(mArgsInfo.full_flag);
    // DD(mArgsInfo.minSize_arg);
    
    // Set information
    const std::vector<typename RelabelFilterType::ObjectSizeType> & a 
      = relabelFilter->GetSizeOfObjectsInPixels();
    m_SizeOfObjectsInPixels.resize(a.size());
    for(unsigned int i=0; i<a.size(); i++) m_SizeOfObjectsInPixels[i] = a[i];
    m_SizeOfObjectsInPhysicalUnits = relabelFilter->GetSizeOfObjectsInPhysicalUnits();
    m_OriginalNumberOfObjects = relabelFilter->GetOriginalNumberOfObjects();
    // DD(m_OriginalNumberOfObjects);
    // DD(m_SizeOfObjectsInPhysicalUnits.size());
    
    output = relabelFilter->GetOutput();
  }
  // PrintMemory(true, "after block");

  // Write/Save results
  this->template SetNextOutput<OutputImageType>(output); 
  // PrintMemory(true, "end filter ");
}
//--------------------------------------------------------------------

#endif //#define CLITKCONNECTEDCOMPONENTLABELINGSGENERICFILTER_TXX
