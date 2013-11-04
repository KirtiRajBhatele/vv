/*=========================================================================
  Program:         vv http://www.creatis.insa-lyon.fr/rio/vv
  Main authors :   XX XX XX

  Authors belongs to: 
  - University of LYON           http://www.universite-lyon.fr/
  - Léon Bérard cancer center    http://www.centreleonberard.fr
  - CREATIS CNRS laboratory      http://www.creatis.insa-lyon.fr

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the copyright notices for more information.

  It is distributed under dual licence
  - BSD       http://www.opensource.org/licenses/bsd-license.php
  - CeCILL-B  http://www.cecill.info/licences/Licence_CeCILL-B_V1-en.html

  =========================================================================*/

// clitk
#include "clitkImage2DicomRTStructFilter.h"
#include "clitkImage2DicomRTStruct_ggo.h"

//--------------------------------------------------------------------
int main(int argc, char * argv[]) {

  // Init command line
  GGO(clitkImage2DicomRTStruct, args_info);

  // Set initial 3D image filenames
  std::vector<std::string> filenames;
  for(unsigned int i=0; i< args_info.input_given; i++)
    filenames.push_back(args_info.input_arg[i]);

  // Create a filter to convert image into dicomRTStruct and write to disk
  typedef float PixelType;
  clitk::Image2DicomRTStructFilter<PixelType> filter;
  filter.SetVerboseFlag(args_info.verbose_flag);
  filter.SetInputFilenames(filenames);
  filter.SetDicomFolder(args_info.dicom_arg);
  filter.SetStructureSetFilename(args_info.rtstruct_arg);
  filter.SetOutputFilename(args_info.output_arg);
  filter.SetROIType(args_info.roitype_arg);
  filter.SetThresholdValue(args_info.threshold_arg);
  filter.SetSkipInitialStructuresFlag(args_info.skip_flag);
  filter.Update();

  // This is the end my friend 
  return 0;
}
//--------------------------------------------------------------------
