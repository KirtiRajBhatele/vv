#ifndef __vvAnimatedGIFWriter_h
#define __vvAnimatedGIFWriter_h

#include <vector>

#include <vtkGenericMovieWriter.h>
#include <vtkSmartPointer.h>

class vtkImageAppend;

class VTK_IO_EXPORT vvAnimatedGIFWriter : public vtkGenericMovieWriter
{
public:
  static vvAnimatedGIFWriter *New();
  vtkTypeMacro(vvAnimatedGIFWriter,vtkGenericMovieWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // These methods start writing an Movie file, write a frame to the file
  // and then end the writing process.
  void Start();
  void Write();
  void End();

  // Description:
  // Set/Get the frame rate, in frame/s.
  vtkSetClampMacro(Rate, int, 1, 5000);
  vtkGetMacro(Rate, int);

  // Description:
  // Set/Get the number of loops,  0 means infinite
  vtkSetClampMacro(Loops, int, 0, 5000);
  vtkGetMacro(Loops, int);

protected:
  vvAnimatedGIFWriter();
  ~vvAnimatedGIFWriter();

  int Rate;
  int Loops;

  vtkSmartPointer<vtkImageAppend> RGBvolume;
  std::vector< vtkSmartPointer<vtkImageData> > RGBslices;

private:
  vvAnimatedGIFWriter(const vvAnimatedGIFWriter&); // Not implemented
  void operator=(const vvAnimatedGIFWriter&); // Not implemented
};

#endif
