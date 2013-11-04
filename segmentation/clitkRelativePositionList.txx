/*=========================================================================
  Program:   vv                     http://www.creatis.insa-lyon.fr/rio/vv

  Authors belong to: 
  - University of LYON              http://www.universite-lyon.fr/
  - Léon Bérard cancer center       http://oncora1.lyon.fnclcc.fr
  - CREATIS CNRS laboratory         http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence

  - BSD        See included LICENSE.txt file
  - CeCILL-B   http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html
  ======================================================================-====*/

#include "clitkLabelImageOverlapMeasureFilter.h"

//--------------------------------------------------------------------
template <class TImageType>
clitk::RelativePositionList<TImageType>::
RelativePositionList():  
  clitk::FilterBase(),
  clitk::FilterWithAnatomicalFeatureDatabaseManagement(),
  itk::ImageToImageFilter<TImageType, TImageType>()
{
  ComputeOverlapFlagOff();
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class TImageType>
void
clitk::RelativePositionList<TImageType>::
SetReferenceImageForOverlapMeasure(ImagePointer ref) {
  m_reference = ref;
  ComputeOverlapFlagOn();
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class TImageType>
void
clitk::RelativePositionList<TImageType>::
Read(std::string filename) {
  /*
    The goal here is to read a text file that contains options for the
    RelativePosition filter. The text file contains options in the
    same form of the config file of the clitkRelativePosition tool. In
    this text file, each time a "object" option is found, a new set of
    options is considered.
   */

  // Open the file
  std::ifstream is;
  openFileForReading(is, filename);

  // Read input -> the structure name that will be used as input
  // (initial support)
  skipComment(is);
  std::string s;
  is >> s;
  if (s != "input") {
    FATAL("while reading RelativePositionList file. This file must start with 'input = '");
    exit(0);
  }
  is >> s; 
  if (s != "=") {
    FATAL("while reading RelativePositionList file. This file must start with 'input = '");
    exit(0);
  }
  std::string name;
  is >> name;
  skipComment(is);

  // Create a temporary filename 
  char n[] = "tmp_clitkRelativePosition_XXXXXX";
  mkstemp(n); // tmpnam(n); 
  std::string tmpfilename(n);
  
  // Loop on the file text ; Every time we see a "object" token, we
  // split the file.
  while (is) {
    bool stop=false;
    std::ostringstream ss;
    // first part of ss is the last 'object = ' found, stored in s
    ss << s << std::endl; 
    while (!stop) {
      skipComment(is);
      if (!is) stop = true;
      else {
        std::getline(is, s);
        if (s.find("object") != std::string::npos) stop=true;
        else ss << s << std::endl;
        if (!is) stop = true;
      }
    }
    std::string text = ss.str();
    if (text.size() > 10) { // if too small, it is not a list of option
      std::ofstream os;
      openFileForWriting(os, tmpfilename);
      os << text;
      os << "input = nothing" << std::endl;
      os << "output = nothing" << std::endl;
      os.close();

      // Create a struct to store options. I use two step to allow to
      // fill the args values with de default and then check
      // automatically the options.
      ArgsInfoType args_info;
      std::vector<char> writable(tmpfilename.size() + 1);
      std::copy(tmpfilename.begin(), tmpfilename.end(), writable.begin());
      char ** argv = new char*[1];
      argv[0] = new char[1];
      struct cmdline_parser_clitkRelativePosition_params params;
      params.override = 0;
      params.initialize = 1;
      params.check_required = 0;
      cmdline_parser_clitkRelativePosition_configfile(&writable[0], &args_info, 1, 1, 1);
      mArgsInfoList.push_back(args_info);
      
      // Delete the temporary file
      std::remove(tmpfilename.c_str());
    }
  }

  // Set the main input name
  SetInputName(name);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class TImageType>
void 
clitk::RelativePositionList<TImageType>::
GenerateInputRequestedRegion() 
{
  // Call default
  itk::ImageToImageFilter<ImageType, ImageType>::GenerateInputRequestedRegion();
  // Get input pointers and set requested region to common region
  ImagePointer input1 = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(0));
  input1->SetRequestedRegion(input1->GetLargestPossibleRegion());
}
//--------------------------------------------------------------------



//--------------------------------------------------------------------
template <class TImageType>
void
clitk::RelativePositionList<TImageType>::
GenerateOutputInformation() {

  // Get input
  m_working_input = dynamic_cast<ImageType*>(itk::ProcessObject::GetInput(0));
  std::string s = GetInputName();

  // Debug
  if (GetDisplayUsedStructuresOnlyFlag()) {
    for(uint i=0; i<mArgsInfoList.size(); i++) {
      // Check if we only want to display structure name
      AddUsedStructures(s, mArgsInfoList[i].object_arg);
    }
    return;
  }

  // Loop on RelativePositionList of operations
  for(uint i=0; i<mArgsInfoList.size(); i++) {

    // clitk::PrintMemory(true, "Start"); 
    // remove _S in station name
    std::string sname = s;
    clitk::findAndReplace<std::string>(sname, "_S", " ");
    std::string text = "["+sname+"] ";
    if (mArgsInfoList[i].orientation_given) text += std::string(mArgsInfoList[i].orientation_arg[0])+" ";
    else text = text+"("+toString(mArgsInfoList[i].angle1_arg)+" "+
           toString(mArgsInfoList[i].angle2_arg)+" "+
           (mArgsInfoList[i].inverse_flag?"true":"false")+") ";
    text = text+mArgsInfoList[i].object_arg+" "+toString(mArgsInfoList[i].threshold_arg);
    if (mArgsInfoList[i].sliceBySlice_flag) {
      text += " SbS";
    }
    else text += " 3D";
    text += " sp=" + toString(mArgsInfoList[i].spacing_arg)+" ";

    StartNewStep(text, false); // no endl
    typename RelPosFilterType::Pointer relPosFilter;

    // Is it slice by slice or 3D ?
    if (mArgsInfoList[i].sliceBySlice_flag) {
      typename SliceRelPosFilterType::Pointer f = SliceRelPosFilterType::New();
      relPosFilter = f;
      SetFilterOptions(relPosFilter, mArgsInfoList[i]);  
      f->SetDirection(2);
      // Set SbS specific options
      f->SetUniqueConnectedComponentBySliceFlag(mArgsInfoList[i].uniqueCCL_flag);
      f->SetObjectCCLSelectionFlag(mArgsInfoList[i].uniqueObjectCCL_flag);
      f->IgnoreEmptySliceObjectFlagOn();
      f->SetVerboseSlicesFlag(mArgsInfoList[i].verboseSlices_flag);
      //f->SetObjectCCLSelectionDimension(0);
      //f->SetObjectCCLSelectionDirection(-1);
      //f->SetAutoCropFlag(false);
      // Print if needed
      if (mArgsInfoList[i].verboseOptions_flag) f->PrintOptions();
    }
    else {
      relPosFilter = clitk::AddRelativePositionConstraintToLabelImageFilter<ImageType>::New();
      SetFilterOptions(relPosFilter, mArgsInfoList[i]);  
      // Print if needed
      if (mArgsInfoList[i].verboseOptions_flag) relPosFilter->PrintOptions();
    }

    // Set input
    relPosFilter->SetInput(m_working_input);  
  
    // Run the filter
    relPosFilter->Update();
    m_working_input = relPosFilter->GetOutput();  
    StopCurrentStep<ImageType>(m_working_input);

    // Compute overlap with reference if needed
    if (GetComputeOverlapFlag()) {
      typedef clitk::LabelImageOverlapMeasureFilter<ImageType> FilterType;
      typename FilterType::Pointer filter = FilterType::New();
      filter->SetInput(0, m_working_input);
      filter->SetInput(1, m_reference);
      filter->Update();
    }
    std::cout << std::endl;    
  }
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class TImageType>
void
clitk::RelativePositionList<TImageType>::
GenerateData() 
{
  // Final Step -> set output
  this->GraftOutput(m_working_input);
}
//--------------------------------------------------------------------


//--------------------------------------------------------------------
template <class TImageType>
void
clitk::RelativePositionList<TImageType>::
SetFilterOptions(typename RelPosFilterType::Pointer filter, ArgsInfoType & options) {

  if (options.orientation_given > 1) {
    clitkExceptionMacro("Error in the RelPos options. I need a single --orientation (for the moment)."
                        << " Current options are for obj = '" << options.object_arg
                        << "', threshold = " << options.threshold_arg << std::endl);
  }

  ImagePointer object = GetAFDB()->template GetImage<ImageType>(options.object_arg);
  filter->SetInputObject(object);
  filter->WriteStepFlagOff();
  filter->SetRadius(options.radius_arg);
  if (options.writeStep_flag) filter->WriteStepFlagOn();
  filter->SetVerboseImageSizeFlag(GetVerboseImageSizeFlag());
  filter->SetFuzzyThreshold(options.threshold_arg);
  filter->SetInverseOrientationFlag(options.inverse_flag); // MUST BE BEFORE AddOrientationTypeString
  filter->SetFastFlag(options.fastFlag_flag);

  if (options.orientation_given == 1)  {
    for(uint i=0; i<options.orientation_given; i++) 
      filter->AddOrientationTypeString(options.orientation_arg[i]);
  }
  else {
    if (options.angle1_given && options.angle2_given) {
      filter->AddAnglesInDeg(options.angle1_arg, options.angle2_arg);
    }
    else {
      clitkExceptionMacro("Error in the RelPos options. I need --orientation or (--angle1 and --angle2)."
                          << " Current options are for obj = '" << options.object_arg
                          << "', threshold = " << options.threshold_arg << std::endl);
    }
  }
  filter->SetIntermediateSpacing(options.spacing_arg);
  if (options.spacing_arg == -1) filter->IntermediateSpacingFlagOff();
  else filter->IntermediateSpacingFlagOn();
  filter->SetVerboseStepFlag(options.verboseStep_flag);
  filter->SetAutoCropFlag(!options.noAutoCrop_flag); 
}
//--------------------------------------------------------------------


