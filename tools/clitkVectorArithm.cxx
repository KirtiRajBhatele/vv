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
#ifndef CLITKVECTORARITHM_CXX
#define CLITKVECTORARITHM_CXX
/**
   -------------------------------------------------
   * @file   clitkVectorArithm.cxx
   * @author David Sarrut <David.Sarrut@creatis.insa-lyon.fr>
   * @date   23 Feb 2008 08:37:53
   -------------------------------------------------*/

// clitk include
#include "clitkVectorArithm_ggo.h"
#include "clitkVectorArithmGenericFilter.h"
#include "clitkIO.h"

//--------------------------------------------------------------------
int main(int argc, char * argv[])
{

  // Init command line
  GGO(clitkVectorArithm, args_info);
  CLITK_INIT;

  // Creation of a generic filter
  typedef clitk::VectorArithmGenericFilter<args_info_clitkVectorArithm> FilterType;
  FilterType::Pointer filter = FilterType::New();

  // Go !
  filter->SetArgsInfo(args_info);
  CLITK_TRY_CATCH_EXIT(filter->Update());

  // this is the end my friend
  return EXIT_SUCCESS;
} // end main

#endif //define CLITKVECTORARITHM_CXX
