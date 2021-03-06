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
#ifndef clitkConditionalBinaryErodeImageFilter_txx
#define clitkConditionalBinaryErodeImageFilter_txx

/* =================================================
 * @file   clitkConditionalBinaryErodeImageFilter.txx
 * @author 
 * @date   
 * 
 * @brief 
 * 
 ===================================================*/

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkConstantBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"
#include "itkBinaryErodeImageFilter.h"

namespace clitk
{

  template <class TInputImage, class TOutputImage, class TKernel>
  ConditionalBinaryErodeImageFilter<TInputImage, TOutputImage, TKernel>
  ::ConditionalBinaryErodeImageFilter()
  {
    this->m_BoundaryToForeground = true;
  }


  template< class TInputImage, class TOutputImage, class TKernel>
  void
  ConditionalBinaryErodeImageFilter< TInputImage, TOutputImage, TKernel>
  ::GenerateData()
  {
    this->AllocateOutputs();

    unsigned int i,j;
    
    // Retrieve input and output pointers
    typename OutputImageType::Pointer output = this->GetOutput();
    typename InputImageType::ConstPointer input  = this->GetInput();
  
    // Get values from superclass
    InputPixelType foregroundValue = this->GetForegroundValue();
    InputPixelType backgroundValue = this->GetBackgroundValue();
    KernelType kernel = this->GetKernel();
    InputSizeType radius;
    radius.Fill(1);
    typename TInputImage::RegionType inputRegion = input->GetBufferedRegion();
    typename TOutputImage::RegionType outputRegion = output->GetBufferedRegion();
  
    // compute the size of the temp image. It is needed to create the progress
    // reporter.
    // The tmp image needs to be large enough to support:
    //   1. The size of the structuring element
    //   2. The size of the connectivity element (typically one)
    typename TInputImage::RegionType tmpRequestedRegion = outputRegion;
    typename TInputImage::RegionType paddedInputRegion
      = input->GetBufferedRegion();
    paddedInputRegion.PadByRadius( radius ); // to support boundary values
    InputSizeType padBy = radius;
    for (i=0; i < KernelDimension; ++i)
      {
	padBy[i] = (padBy[i]>kernel.GetRadius(i) ? padBy[i] : kernel.GetRadius(i));
      }
    tmpRequestedRegion.PadByRadius( padBy );
    tmpRequestedRegion.Crop( paddedInputRegion );

    typename TInputImage::RegionType requiredInputRegion
      = input->GetBufferedRegion();
    requiredInputRegion.Crop( tmpRequestedRegion );

    // Support progress methods/callbacks
    // Setup a progress reporter.  We have 4 stages to the algorithm so
    // pretend we have 4 times the number of pixels
    itk::ProgressReporter progress(this, 0,
			      outputRegion.GetNumberOfPixels() * 3
			      + tmpRequestedRegion.GetNumberOfPixels()
			      + requiredInputRegion.GetNumberOfPixels() );
  
    // Allocate and reset output. We copy the input to the output,
    // except for pixels with DilateValue.  These pixels are initially
    // replaced with BackgroundValue and potentially replaced later with
    // DilateValue as the Minkowski sums are performed.
    itk::ImageRegionIterator<OutputImageType> outIt( output, outputRegion );
    //ImageRegionConstIterator<InputImageType> inIt( input, outputRegion );

    for( outIt.GoToBegin(); !outIt.IsAtEnd(); ++outIt )
      {
	outIt.Set( foregroundValue );
	progress.CompletedPixel();
      }


    // Create the temp image for surface encoding
    // The temp image size is equal to the output requested region for thread
    // padded by max( connectivity neighborhood radius, SE kernel radius ).
    typedef itk::Image<unsigned char, TInputImage::ImageDimension> TempImageType;
    typename TempImageType::Pointer tmpImage = TempImageType::New();

    // Define regions of temp image
    tmpImage->SetRegions( tmpRequestedRegion );
  
    // Allocation.
    // Pay attention to the fact that here, the output is still not
    // allocated (so no extra memory needed for tmp image, if you
    // consider that you reserve som memory space for output)
    tmpImage->Allocate();
  
    // First Stage
    // Copy the input image to the tmp image.
    // Tag the tmp Image.
    //     zero means background
    //     one means pixel on but not treated
    //     two means border pixel
    //     three means inner pixel
    static const unsigned char backgroundTag  = 0;
    static const unsigned char onTag          = 1;
    static const unsigned char borderTag      = 2;
    static const unsigned char innerTag       = 3;
  
    if( !this->m_BoundaryToForeground )
      { tmpImage->FillBuffer( onTag ); }
    else
      { tmpImage->FillBuffer( backgroundTag ); }

    // Iterators on input and tmp image
    // iterator on input
    itk::ImageRegionConstIterator<TInputImage> iRegIt( input, requiredInputRegion );
    // iterator on tmp image
    itk::ImageRegionIterator<TempImageType> tmpRegIt( tmpImage, requiredInputRegion );

    for( iRegIt.GoToBegin(), tmpRegIt.GoToBegin();
	 !tmpRegIt.IsAtEnd();
	 ++iRegIt, ++tmpRegIt )
      {
	OutputPixelType pxl = iRegIt.Get();

	//JV pxl != foregroundValue
	if( pxl == backgroundValue )
	  { tmpRegIt.Set( onTag ); }
	else
	  {
	    // by default if it is not foreground, consider
	    // it as background
	    tmpRegIt.Set( backgroundTag ); 
	  }
	progress.CompletedPixel();
      }

    // Second stage
    // Border tracking and encoding

    // Need to record index, use an iterator with index
    // Define iterators that will traverse the OUTPUT requested region
    // for thread and not the padded one. The tmp image has been padded
    // because in that way we will take care carefully at boundary
    // pixels of output requested region.  Take care means that we will
    // check if a boundary pixel is or not a border pixel.
    itk::ImageRegionIteratorWithIndex<TempImageType>
      tmpRegIndexIt( tmpImage, tmpRequestedRegion );

    itk::ConstNeighborhoodIterator<TempImageType>
      oNeighbIt( radius, tmpImage, tmpRequestedRegion );
  
    // Define boundaries conditions
    itk::ConstantBoundaryCondition<TempImageType> cbc;
    cbc.SetConstant( backgroundTag );
    oNeighbIt.OverrideBoundaryCondition(&cbc);
  
    unsigned int neighborhoodSize = oNeighbIt.Size();
    unsigned int centerPixelCode = neighborhoodSize / 2;
  
    std::queue<IndexType> propagQueue;

    // Neighborhood iterators used to track the surface.
    //
    // Note the region specified for the first neighborhood iterator is
    // the requested region for the tmp image not the output image. This
    // is necessary because the NeighborhoodIterator relies on the
    // specified region to determine if you will ever query a boundary
    // condition pixel.  Since we call SetLocation on the neighbor of a
    // specified pixel, we have to set the region for the interator to
    // include any pixel we may set our location to.
    itk::NeighborhoodIterator<TempImageType>
      nit( radius, tmpImage, tmpRequestedRegion );
    nit.OverrideBoundaryCondition(&cbc);
    nit.GoToBegin();

    itk::ConstNeighborhoodIterator<TempImageType>
      nnit( radius, tmpImage, tmpRequestedRegion );
    nnit.OverrideBoundaryCondition(&cbc);
    nnit.GoToBegin();
  
    for( tmpRegIndexIt.GoToBegin(), oNeighbIt.GoToBegin();
	 !tmpRegIndexIt.IsAtEnd();
	 ++tmpRegIndexIt, ++oNeighbIt )
      {

	unsigned char tmpValue = tmpRegIndexIt.Get();

	// Test current pixel: it is active ( on ) or not?
	if( tmpValue == onTag )
	  {
	    // The current pixel has not been treated previously.  That
	    // means that we do not know that it is an inner pixel of a
	    // border pixel.
      
	    // Test current pixel: it is a border pixel or an inner pixel?
	    bool bIsOnContour = false;
      
	    for (i = 0; i < neighborhoodSize; ++i)
	      {
		// If at least one neighbour pixel is off the center pixel
		// belongs to contour
		if( oNeighbIt.GetPixel( i ) == backgroundTag )
		  {
		    bIsOnContour = true;
		    break;
		  }
	      }

	    if( bIsOnContour )
	      {
		// center pixel is a border pixel and due to the parsing, it is also
		// a pixel which belongs to a new border connected component
		// Now we will parse this border thanks to a burn procedure
 
		// mark pixel value as a border pixel
		tmpRegIndexIt.Set( borderTag );
 
		// add it to border container.
		// its code is center pixel code because it is the first pixel
		// of the connected component border

		// paint the structuring element
		typename NeighborIndexContainer::const_iterator itIdx;
		NeighborIndexContainer& idxDifferenceSet
		  = this->GetDifferenceSet( centerPixelCode );
		for( itIdx = idxDifferenceSet.begin();
		     itIdx != idxDifferenceSet.end();
		     ++itIdx )
		  {
		    IndexType idx = tmpRegIndexIt.GetIndex() + *itIdx;
		    if( outputRegion.IsInside( idx ) )
		      { output->SetPixel( idx, backgroundValue ); }
		  }
 
		// add it to queue
		propagQueue.push( tmpRegIndexIt.GetIndex() );
 
		// now find all the border pixels
		while ( !propagQueue.empty() )
		  {
		    // Extract pixel index from queue
		    IndexType currentIndex = propagQueue.front();
		    propagQueue.pop();
   
		    nit += currentIndex - nit.GetIndex();
   
		    for (i = 0; i < neighborhoodSize; ++i)
		      {
			// If pixel has not been already treated and it is a pixel
			// on, test if it is an inner pixel or a border pixel
     
			// Remark: all the pixels outside the image are set to
			// backgroundTag thanks to boundary conditions. That means that if
			// we enter in the next if-statement we are sure that the
			// current neighbour pixel is in the image
			if( nit.GetPixel( i ) == onTag )
			  {
			    // Check if it is an inner or border neighbour pixel
			    // Get index of current neighbour pixel
			    IndexType neighbIndex = nit.GetIndex( i );
       
			    // Force location of neighbour iterator
			    nnit += neighbIndex - nnit.GetIndex();

			    bool bIsOnBorder = false;
       
			    for( j = 0; j < neighborhoodSize; ++j)
			      {
				// If at least one neighbour pixel is off the center
				// pixel belongs to border
				if( nnit.GetPixel(j) == backgroundTag )
				  {
				    bIsOnBorder = true;
				    break;
				  } 
			      }
       
       
			    if( bIsOnBorder )
			      {
				// neighbour pixel is a border pixel
				// mark it
				bool status;
				nit.SetPixel( i, borderTag, status );

				// check whether we could set the pixel.  can only set
				// the pixel if it is within the tmpimage
				if (status) 
				  {
				    // add it to queue
				    propagQueue.push( neighbIndex );
    
				    // paint the structuring element
				    typename NeighborIndexContainer::const_iterator itIndex;
				    NeighborIndexContainer& indexDifferenceSet
				      = this->GetDifferenceSet( i );
				    for( itIndex = indexDifferenceSet.begin();
					 itIndex != indexDifferenceSet.end();
					 ++itIndex )
				      {
					IndexType idx = neighbIndex + *itIndex;
					if( outputRegion.IsInside( idx ) )
					  { output->SetPixel( idx, backgroundValue ); }
				      }
				  }
			      }
			    else
			      {
				// neighbour pixel is an inner pixel
				bool status;
				nit.SetPixel( i, innerTag, status );
			      }
              
			    progress.CompletedPixel();
			  } // if( nit.GetPixel( i ) == onTag )
     
		      } // for (i = 0; i < neighborhoodSize; ++i)
   
		  } // while ( !propagQueue.empty() )
 
	      } // if( bIsOnCountour )
	    else
	      { tmpRegIndexIt.Set( innerTag ); }

	    progress.CompletedPixel();
      
	  } // if( tmpRegIndexIt.Get() == onTag )
	else if( tmpValue == backgroundTag )
	  { progress.CompletedPixel(); }
	// Here, the pixel is a background pixel ( value at 0 ) or an
	// already treated pixel:
	//     2 for border pixel, 3 for inner pixel
      }
  

    // Deallocate tmpImage
    tmpImage->Initialize();
  
    // Third Stage
    // traverse structure of border and SE CCs, and paint output image
  
    // Let's consider the the set of the ON elements of the input image as X.
    // 
    // Let's consider the structuring element as B = {B0, B1, ..., Bn},
    // where Bi denotes a connected component of B.
    //
    // Let's consider bi, i in [0,n], an arbitrary point of Bi.
    //

    // We use hence the next property in order to compute minkoswki
    // addition ( which will be written (+) ):
    //
    // X (+) B = ( Xb0 UNION Xb1 UNION ... Xbn ) UNION ( BORDER(X) (+) B ),
    //
    // where Xbi is the set X translated with respect to vector bi :
    //
    // Xbi = { x + bi, x belongs to X } where BORDER(X) is the extracted
    // border of X ( 8 connectivity in 2D, 26 in 3D )
  
    // Define boundaries conditions
    itk::ConstantBoundaryCondition<TOutputImage> obc;
    obc.SetConstant( backgroundValue );
  
    itk::NeighborhoodIterator<OutputImageType>
      onit( kernel.GetRadius(), output, outputRegion );
    onit.OverrideBoundaryCondition(&obc);
    onit.GoToBegin();

  
    // Paint input image translated with respect to the SE CCs vectors
    // --> "( Xb0 UNION Xb1 UNION ... Xbn )"
    typename Superclass::ComponentVectorConstIterator vecIt;
    typename Superclass::ComponentVectorConstIterator vecBeginIt
      = this->KernelCCVectorBegin();
    typename Superclass::ComponentVectorConstIterator vecEndIt
      = this->KernelCCVectorEnd();
  
    // iterator on output image
    itk::ImageRegionIteratorWithIndex<OutputImageType>
      ouRegIndexIt(output, outputRegion );
    ouRegIndexIt.GoToBegin(); 
  
    // InputRegionForThread is the output region for thread padded by
    // kerne lradius We must traverse this padded region because some
    // border pixel in the added band ( the padded band is the region
    // added after padding ) may be responsible to the painting of some
    // pixel in the non padded region.  This happens typically when a
    // non centered SE is used, a kind of shift is done on the "on"
    // pixels of image. Consequently some pixels in the added band can
    // appear in the current region for thread due to shift effect.
    typename InputImageType::RegionType inputRegionForThread = outputRegion;
  
    // Pad the input region by the kernel
    inputRegionForThread.PadByRadius( kernel.GetRadius() );
    inputRegionForThread.Crop( input->GetBufferedRegion() );  

    if( !this->m_BoundaryToForeground )
      {
	while( !ouRegIndexIt.IsAtEnd() )
	  {
	    // Retrieve index of current output pixel
	    IndexType currentIndex  = ouRegIndexIt.GetIndex();
	    for( vecIt = vecBeginIt; vecIt != vecEndIt; ++vecIt )
	      {
		// Translate
		IndexType translatedIndex = currentIndex - *vecIt;
  
		// translated index now is an index in input image in the 
		// output requested region padded. Theoretically, this translated
		// index must be inside the padded region.
		// If the pixel in the input image at the translated index
		// has a value equal to the dilate one, this means
		// that the output pixel at currentIndex will be on in the output.
		//JV  != foregroundValue
		if( !inputRegionForThread.IsInside( translatedIndex )
		    || input->GetPixel( translatedIndex ) == backgroundValue )
		  {
		    ouRegIndexIt.Set( backgroundValue );
		    break; // Do not need to examine other offsets because at least one
		    // input pixel has been translated on current output pixel.
		  }
	      }
  
	    ++ouRegIndexIt;
	    progress.CompletedPixel();
	  }
      }
    else
      {
	while( !ouRegIndexIt.IsAtEnd() )
	  {
	    IndexType currentIndex  = ouRegIndexIt.GetIndex();
	    for( vecIt = vecBeginIt; vecIt != vecEndIt; ++vecIt )
	      {
		IndexType translatedIndex = currentIndex - *vecIt;
  
		if( inputRegionForThread.IsInside( translatedIndex )
		    //JV != foregroundValue
		    && input->GetPixel( translatedIndex ) == backgroundValue )
		  {
		    ouRegIndexIt.Set( backgroundValue );
		    break;
		  }
	      }
  
	    ++ouRegIndexIt;
	    progress.CompletedPixel();
	  }
      }

    // now, we must to restore the background values
    itk::ImageRegionConstIterator<InputImageType> inIt( input, outputRegion );

    for( inIt.GoToBegin(), outIt.GoToBegin(); !outIt.IsAtEnd(); ++outIt, ++inIt )
      {
	InputPixelType inValue = inIt.Get();
	OutputPixelType outValue = outIt.Get();
	// JV != foregroundValue
	if ( outValue == backgroundValue && inValue == backgroundValue )
	  { outIt.Set( static_cast<OutputPixelType>( inValue ) ); }
	progress.CompletedPixel();
      }


  }


  /**
   * Standard "PrintSelf" method
   */
  template <class TInputImage, class TOutput, class TKernel>
  void
  ConditionalBinaryErodeImageFilter<TInputImage, TOutput, TKernel>
  ::PrintSelf( std::ostream& os, itk::Indent indent) const
  {
    Superclass::PrintSelf( os, indent );
    os << indent << "Dilate Value: " << static_cast<typename itk::NumericTraits<InputPixelType>::PrintType>( this->GetForegroundValue() ) << std::endl;
  }

}//end clitk
 
#endif //#define clitkConditionalBinaryErodeImageFilter_txx
