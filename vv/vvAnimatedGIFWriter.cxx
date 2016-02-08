#include "vvAnimatedGIFWriter.h"
#include "clitkDD.h"

#include <vtkImageData.h>
#include <vtkImageQuantizeRGBToIndex.h>
#include <vtkImageAppend.h>
#include <vtkImageCast.h>
#include <vtkObjectFactory.h>
#include <vtkLookupTable.h>

#include "ximagif.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro(vvAnimatedGIFWriter);

//---------------------------------------------------------------------------
vvAnimatedGIFWriter::vvAnimatedGIFWriter()
{
  Rate = 5;
  Loops = 0;
  Dither = false;
}

//---------------------------------------------------------------------------
vvAnimatedGIFWriter::~vvAnimatedGIFWriter()
{
}

//---------------------------------------------------------------------------
void vvAnimatedGIFWriter::Start()
{
  // Create one volume with all slices
  RGBvolume = vtkSmartPointer<vtkImageAppend>::New();
  RGBvolume->SetAppendAxis(2);
  RGBslices.clear();
}

//---------------------------------------------------------------------------
void vvAnimatedGIFWriter::Write()
{
  // get the data
  this->GetInput()->UpdateInformation();
  int *wExtent = this->GetInput()->GetWholeExtent();
  this->GetInput()->SetUpdateExtent(wExtent);
  this->GetInput()->Update();

  RGBslices.push_back( vtkSmartPointer<vtkImageData>::New() );
  RGBslices.back()->ShallowCopy(this->GetInput());
  RGBvolume->AddInput(RGBslices.back());
}

//---------------------------------------------------------------------------
void vvAnimatedGIFWriter::End()
{
  RGBvolume->Update();

  // Quantize to 8 bit colors
  vtkSmartPointer<vtkImageQuantizeRGBToIndex> quant = vtkSmartPointer<vtkImageQuantizeRGBToIndex>::New();
  quant->SetNumberOfColors(256);
  quant->SetInput(RGBvolume->GetOutput());
  quant->Update();

  // Convert to 8 bit image
  vtkSmartPointer<vtkImageCast> cast =  vtkSmartPointer<vtkImageCast>::New();
  cast->SetInput( quant->GetOutput() );
  cast->SetOutputScalarTypeToUnsignedChar();
  cast->Update();

  // Create palette for CxImage => Swap r and b in LUT
  RGBQUAD pal[256];
  memcpy(pal, (RGBQUAD*)(quant->GetLookupTable()->GetPointer(0)), sizeof(RGBQUAD)*256);
  for(unsigned int j=0; j<256; j++)
    std::swap(pal[j].rgbBlue, pal[j].rgbRed);

  // Create a stack of CxImages
  DWORD width = cast->GetOutput()->GetExtent()[1]-cast->GetOutput()->GetExtent()[0]+1;
  DWORD height = cast->GetOutput()->GetExtent()[3]-cast->GetOutput()->GetExtent()[2]+1;
  std::vector<CxImage*> cximages( RGBslices.size() );
  for(unsigned int i=0; i<RGBslices.size(); i++) {
    cximages[i] = new CxImage;
    cximages[i]->SetFrameDelay(100/Rate);
    if(Dither) {
      cximages[i]->CreateFromArray((BYTE *)RGBvolume->GetOutput()->GetScalarPointer(0,0,i),
                                   width, height, 24, width*3, false);
      cximages[i]->SwapRGB2BGR();
      cximages[i]->DecreaseBpp(8, true, pal);
    }
    else {
      cximages[i]->CreateFromArray((BYTE *)cast->GetOutput()->GetScalarPointer(0,0,i),
                                   width, height, 8, width, false);
      cximages[i]->SetPalette(pal);
    }
  }

  // Create gif
  FILE * pFile;
  pFile = fopen (this->FileName, "wb");
  if(pFile==NULL) {
    vtkErrorMacro("Error in vvAnimatedGIFWriter::End: could not open " << this->FileName );
    return;
  }
  CxImageGIF cximagegif;
  cximagegif.SetLoops(Loops);
  bool result = cximagegif.Encode(pFile,&(cximages[0]), (int)RGBslices.size(), true);

  // Cleanup
  fclose(pFile);
  for(unsigned int i=0; i<RGBslices.size(); i++)
    delete cximages[i];
  if(!result) {
    vtkErrorMacro("Error in CxImage: " << cximagegif.GetLastError() );
  }
}

//---------------------------------------------------------------------------
void vvAnimatedGIFWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
