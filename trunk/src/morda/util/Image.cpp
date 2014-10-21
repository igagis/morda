/* The MIT License:

Copyright (c) 2008-2014 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

/**
 * @file Image class
 * @author Ivan Gagis <igagis@gmail.com>
 */


#include <cstring>
#include <algorithm>

#include <ting/types.hpp>
#include <ting/math.hpp>
#include <ting/Exc.hpp>

#include <png.h>

//JPEG lib does not have 'extern "C"{}' :-(, so we put it outside of their .h
//or will have linking problems otherwise because
//of "_" symbol in front of C-function names
extern "C"{
#include <jpeglib.h>
}



#include "Image.hpp"



using namespace morda;



void Image::Init(unsigned width, unsigned height, EType typeOfImage){
	this->Reset();
	this->w = width;
	this->h = height;
	this->type = typeOfImage;
	this->buf.resize(this->w * this->h * this->NumChannels());
}



Image::Image(unsigned width, unsigned height, EType typeOfImage, const std::uint8_t* srcBuf){
	ASSERT(srcBuf)
	this->Init(width, height, typeOfImage);
	memcpy(&*this->buf.begin(), srcBuf, this->buf.size() * sizeof(this->buf[0]));
}



Image::Image(unsigned x, unsigned y, unsigned width, unsigned height, const Image& src){
	if(src.Width() == 0 || src.Height() == 0)
		throw ting::Exc("Image::Image(): source image has zero dimensions");

	if( src.Width() <= x || src.Height() <= y || src.Width() < (x + width) || src.Height() < (y + height) )
		throw ting::Exc("Image::Image(): incorrect dimensions of given images");

	this->Init(width, height, src.Type());

	//copy image data
	throw ting::Exc("Image::Image(unsigned x, unsigned y, unsigned width, unsigned height, const Image& src): is not implemented");
	//for(unsigned j=0; j<this->H(); ++j)
	//    memcpy(data+j*w*channels,src->data+((j+y)*src->w+x)*channels,w*channels);
}



//copy constructor
Image::Image(const Image& im){
	this->Init(im.Width(), im.Height(), im.Type());
	ASSERT(this->buf.size() * sizeof(this->buf[0]) == im.buf.size() * sizeof(im.buf[0]))
	memcpy(this->Buf().begin(), im.Buf().begin(), this->buf.size() * sizeof(this->buf[0]));
}



//destructor
Image::~Image(){
	//nothing to do here yet
}



//Fills image buffer with zeroes
void Image::Clear(std::uint8_t  val){
	memset(&*this->buf.begin(), val, this->buf.size() * sizeof(this->buf[0]));
}



void Image::Clear(unsigned chan, std::uint8_t val){
	for(unsigned i = 0; i < this->Width() * this->Height(); ++i){
		this->buf[i * this->NumChannels() + chan] = val;
	}
}



//Null all data
void Image::Reset(){
	this->w = 0;
	this->h = 0;
	this->type = UNKNOWN;
	this->buf.clear();
}



//====================================================
//============Flip Image vertical method==============
//====================================================
//Flips vertically current image
void Image::FlipVertical(){
	if(!this->buf.size()){
		return;//nothing to flip
	}

	unsigned stride = this->NumChannels() * this->Width();//stride
	std::vector<std::uint8_t> line(stride);

	//TODO: use iterators
	for(unsigned i = 0; i < this->Height() / 2; ++i){
		memcpy(&*line.begin(), &*this->buf.begin() + stride * i, stride);//move line to temp
		memcpy(&*this->buf.begin() + stride * i, &*this->buf.begin() + stride * (this->Height() - i - 1), stride);//move bottom line to top
		memcpy(&*this->buf.begin() + stride * (this->Height() - i - 1), &*line.begin(), stride);
	}
}



void Image::Blit(unsigned x, unsigned y, const Image& src){
	ASSERT(this->buf.size())
	if(this->Type() != src.Type()){
		throw ting::Exc("Image::Blit(): bits per pixel values do not match");
	}

	unsigned blitAreaW = std::min(src.Width(), this->Width() - x);
	unsigned blitAreaH = std::min(src.Height(), this->Height() - y);

	//TODO: implement blitting for all image types
	switch(this->Type()){
		case GREY:
			for(unsigned j = 0; j < blitAreaH; ++j){
				for(unsigned i = 0; i < blitAreaW; ++i){
					this->PixChan(i + x, j + y, 0) = src.PixChan(i, j, 0);
				}
			}
			break;
		case GREYA:
			for(unsigned j = 0; j < blitAreaH; ++j){
				for(unsigned i = 0; i < blitAreaW; ++i){
					this->PixChan(i + x, j + y, 0) = src.PixChan(i, j, 0);
					this->PixChan(i + x, j + y, 1) = src.PixChan(i, j, 1);
				}
			}
			break;
		default:
			ASSERT_INFO(false, "Image::Blit(): unknown image type")
			break;
	}//~switch(type)
}



void Image::Blit(unsigned x, unsigned y, const Image& src, unsigned dstChan, unsigned srcChan){
	ASSERT(this->buf.size())
	if(dstChan >= this->NumChannels()){
		throw ting::Exc("Image::Blit(): destination channel index is greater than number of channels in the image");
	}

	if(srcChan >= src.NumChannels()){
		throw ting::Exc("Image::Blit(): source channel index is greater than number of channels in the image");
	}

	unsigned blitAreaW = std::min(src.Width(), this->Width() - x);
	unsigned blitAreaH = std::min(src.Height(), this->Height() - y);

	for(unsigned j = 0; j < blitAreaH; ++j){
		for(unsigned i = 0; i < blitAreaW; ++i){
			this->PixChan(i + x, j + y, dstChan) = src.PixChan(i, j, srcChan);
		}
	}
}



//================================\      /====\      /=PPPP===N===N===GGGG====|
//=================================\    /======\    /==P===P==NN==N==G========|
//======Read PNG file method========|--|========|--|===PPPP===N=N=N==G==GG====|
//=================================/    \======/    \==P======N==NN==G===G====|
//================================/      \====/      \=P======N===N===GGG=====|
//custom file read function for PNG
namespace{

void PNG_CustomReadFunction(png_structp pngPtr, png_bytep data, png_size_t length){
	ting::fs::File* fi = reinterpret_cast<ting::fs::File*>(png_get_io_ptr(pngPtr));
	ASSERT(fi)
//	TRACE(<< "PNG_CustomReadFunction: fi = " << fi << " pngPtr = " << pngPtr << " data = " << std::hex << data << " length = " << length << std::endl)
	try{
		ting::Buffer<png_byte> bufWrapper(data, size_t(length));
		fi->Read(bufWrapper);
//		TRACE(<< "PNG_CustomReadFunction: fi->Read() finished" << std::endl)
	}catch(...){
		//do not let any exception get out of this function
//		TRACE(<< "PNG_CustomReadFunction: fi->Read() failed" << std::endl)
	}
}

}//~namespace



//Read PNG file method
void Image::LoadPNG(const ting::fs::File& fi){
	ASSERT(!fi.IsOpened())

	if(this->buf.size() > 0){
		this->Reset();
	}

	ting::fs::File::Guard fileGuard(fi);//this will guarantee that the file will be closed upon exit
//	TRACE(<< "Image::LoadPNG(): file opened" << std::endl)

#define PNGSIGSIZE 8 //The size of PNG signature (max 8 bytes)
	std::array<png_byte, PNGSIGSIZE> sig;
	memset(&*sig.begin(), 0, sig.size() * sizeof(sig[0]));

	{
#ifdef DEBUG
		unsigned ret = //TODO:???
#endif
		fi.Read(sig);
		ASSERT(ret == sig.size() * sizeof(sig[0]))
	}

	if(png_sig_cmp(&*sig.begin(), 0, sig.size() * sizeof(sig[0])) != 0){//if it is not a PNG-file
		throw Image::Exc("Image::LoadPNG(): not a PNG file");
	}

	//Great!!! We have a PNG-file!
//	TRACE(<< "Image::LoadPNG(): file is a PNG" << std::endl)

	//Create internal PNG-structure to work with PNG file
	//(no warning and error callbacks)
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

	png_infop infoPtr = png_create_info_struct(pngPtr);//Create structure with file info

	png_set_sig_bytes(pngPtr, PNGSIGSIZE);//We've already read PNGSIGSIZE bytes

	//Set custom "ReadFromFile" function
	png_set_read_fn(pngPtr, const_cast<ting::fs::File*>(&fi), PNG_CustomReadFunction);

	png_read_info(pngPtr, infoPtr);//Read in all information about file

	//Get information from infoPtr
	png_uint_32 width = 0;
	png_uint_32 height = 0;
	int bitDepth = 0;
	int colorType = 0;
	png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, 0, 0, 0);

	//Strip 16bit png  to 8bit
	if(bitDepth == 16){
		png_set_strip_16(pngPtr);
	}
	//Convert paletted PNG to RGB image
	if(colorType == PNG_COLOR_TYPE_PALETTE){
		png_set_palette_to_rgb(pngPtr);
	}
	//Convert grayscale PNG to 8bit greyscale PNG
	if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8){
		png_set_expand_gray_1_2_4_to_8(pngPtr);
	}
	//if(png_get_valid(pngPtr, infoPtr,PNG_INFO_tRNS)) png_set_tRNS_to_alpha(pngPtr);

	//set gamma information
	double gamma = 0.0f;

	//if there's gamma info in the file, set it to 2.2
	if(png_get_gAMA(pngPtr, infoPtr, &gamma)){
		png_set_gamma(pngPtr, 2.2, gamma);
	}else{
		png_set_gamma(pngPtr, 2.2, 0.45455);//set to 0.45455 otherwise (good guess for GIF images on PCs)
	}

	//update info after all transformations
	png_read_update_info(pngPtr, infoPtr);
	//get all dimensions and color info again
	png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, &colorType, 0, 0, 0);
	ASSERT(bitDepth == 8)

	//Set image type
	Image::EType imageType;
	switch(colorType){
		case PNG_COLOR_TYPE_GRAY:
			imageType = Image::GREY;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			imageType = Image::GREYA;
			break;
		case PNG_COLOR_TYPE_RGB:
			imageType = Image::RGB;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			imageType = Image::RGBA;
			break;
		default:
			throw Image::Exc("Image::LoadPNG(): unknown colorType");
			break;
	}
	//Great! Number of channels and bits per pixel are initialized now!

	//set image dimensions and set buffer size
	this->Init(width, height, imageType);//Set buf array size (allocate memory)
	//Great! height and width are initialized and buffer memory allocated

//	TRACE(<< "Image::LoadPNG(): memory for image allocated" << std::endl)

	//Read image data
	png_uint_32 bytesPerRow = png_get_rowbytes(pngPtr, infoPtr);//get bytes per row

	//check that our expectations are correct
	if(bytesPerRow != this->Width() * this->NumChannels()){
		throw Image::Exc("Image::LoadPNG(): number of bytes per row does not match expected value");
	}

	ASSERT((bytesPerRow * height) == this->buf.size())

//	TRACE(<< "Image::LoadPNG(): going to read in the data" << std::endl)
	{
		ASSERT(this->Height() && this->buf.size())
		std::vector<png_bytep> rows(this->Height());
		//initialize row pointers
//		M_IMAGE_PRINT(<< "Image::LoadPNG(): this->buf.Buf() = " << std::hex << this->buf.Buf() << std::endl)
		for(unsigned i = 0; i < this->Height(); ++i){
			rows[i] = &*this->buf.begin() + i * bytesPerRow;
//			M_IMAGE_PRINT(<< "Image::LoadPNG(): rows[i] = " << std::hex << rows[i] << std::endl)
		}
//		TRACE(<< "Image::LoadPNG(): row pointers are set" << std::endl)
		//Read in image data!
		png_read_image(pngPtr, &*rows.begin());
//		TRACE(<< "Image::LoadPNG(): image data read" << std::endl)
	}

	png_destroy_read_struct(&pngPtr,0,0);//free libpng memory
}//~Image::LoadPNG()









//`````````````````````````````````````````````JJJJJJJ``PPPPPPP````GGGGGGG
//------------------------------------------------J-----P      P--G       G
//================================================J=====P      P==G
//============JPG image read method===============J=====PPPPPPP===G    GGGG
//============================================J   J=====P=========G    G  G
//--------------------------------------------J   J-----P---------G       G
//`````````````````````````````````````````````JJJ``````P``````````GGGGGGG
namespace{


const size_t DJpegInputBufferSize = 4096;



struct DataManagerJPEGSource{
	jpeg_source_mgr pub;
	ting::fs::File *fi;
	JOCTET *buffer;
	bool sof;//true if the file was just opened
};



void JPEG_InitSource(j_decompress_ptr cinfo){
	ASSERT(cinfo)
	ASS(reinterpret_cast<DataManagerJPEGSource*>(cinfo->src))->sof = true;
}



//This function is calld when variable "bytes_in_buffer" reaches 0 and
//the necessarity in new portion of information appears.
//RETURNS: TRUE if the buffer is successfuly filled.
//         FALSE if i/o error occured
boolean JPEG_FillInputBuffer(j_decompress_ptr cinfo){
	DataManagerJPEGSource* src = reinterpret_cast<DataManagerJPEGSource*>(cinfo->src);
	ASSERT(src)

	//Read in JPEGINPUTBUFFERSIZE JOCTET's
	int nbytes;

	try{
		ting::Buffer<std::uint8_t> bufWrapper(src->buffer, sizeof(JOCTET) * DJpegInputBufferSize);
		nbytes = ASS(src->fi)->Read(bufWrapper);
	}catch(ting::fs::File::Exc&){
		if(src->sof){
			return FALSE;//the specified file is empty
		}
		//we read the data before. Insert End Of File info into the buffer
		src->buffer[0] = (JOCTET)(0xFF);
		src->buffer[1] = (JOCTET)(JPEG_EOI);
		nbytes = 2;
	}catch(...){
		return FALSE;//error
	}

	//Set nexit input byte for JPEG and number of bytes read
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->sof = false;//the file is not empty since we read some data
	return TRUE;//Operation successful
}



//Skip num_bytes (seek forward)
void JPEG_SkipInputData(j_decompress_ptr cinfo, long numBytes){
	ASSERT(cinfo)
	DataManagerJPEGSource* src = reinterpret_cast<DataManagerJPEGSource*>(cinfo->src);
	ASSERT(src)
	if(numBytes <= 0){
		return;//nothing to skip
	}

	//read "numBytes" bytes and waste them away
	while(numBytes > long(src->pub.bytes_in_buffer)){
		numBytes -= long(src->pub.bytes_in_buffer);
		JPEG_FillInputBuffer(cinfo);
	}

	//update current JPEG read position
	src->pub.next_input_byte += size_t(numBytes);
	src->pub.bytes_in_buffer -= size_t(numBytes);
}



//Terminate source when decompress is finished
//(nothing to do in this function in our case)
void JPEG_TermSource(j_decompress_ptr cinfo){}


}//~namespace



//Read JPEG function
void Image::LoadJPG(const ting::fs::File& fi){
	ASSERT(!fi.IsOpened())

//	TRACE(<< "Image::LoadJPG(): enter" << std::endl)
	if(this->buf.size()){
		this->Reset();
	}
	
	ting::fs::File::Guard fileGuard(fi);//this will guarantee that the file will be closed upon exit
//	TRACE(<< "Image::LoadJPG(): file opened" << std::endl)

	//Required JPEG structures
	jpeg_decompress_struct cinfo;//decompression object
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);//creat decompress object

	DataManagerJPEGSource* src = 0;

	//Check if memory for JPEG-decompressor manager is allocated.
	//It is possible that several libraries accessing the source
	if(cinfo.src == 0){
		//Allocate memory for our manager and set a pointer of global library
		//structure to it. We use JPEG library memory manager, this means that
		//the library will take care of memory freeing for us.
		//JPOOL_PERMANENT means that the memory is allocated for a whole
		//time  of working with the library.
		cinfo.src = reinterpret_cast<jpeg_source_mgr*>(
				(ASS(cinfo.mem)->alloc_small)(
						j_common_ptr(&cinfo),
						JPOOL_PERMANENT,
						sizeof(DataManagerJPEGSource)
					)
			);
		src = reinterpret_cast<DataManagerJPEGSource*>(cinfo.src);
		if(!src){
			throw Image::Exc("Image::LoadJPG(): memory alloc failed");
		}
		//Allocate memory for read data
		ASS(src)->buffer = reinterpret_cast<JOCTET*>(
				(cinfo.mem->alloc_small)(
						j_common_ptr(&cinfo),
						JPOOL_PERMANENT,
						DJpegInputBufferSize * sizeof(JOCTET)
					)
			);

		if(!src->buffer){
			throw Image::Exc("Image::LoadJPG(): memory alloc failed");
		}

		memset(src->buffer, 0, DJpegInputBufferSize * sizeof(JOCTET));
	}else{
		src = reinterpret_cast<DataManagerJPEGSource*>(cinfo.src);
	}

	//set handler functions
	src->pub.init_source = &JPEG_InitSource;
	src->pub.fill_input_buffer = &JPEG_FillInputBuffer;
	src->pub.skip_input_data = &JPEG_SkipInputData;
	src->pub.resync_to_restart = &jpeg_resync_to_restart;// use default func
	src->pub.term_source = &JPEG_TermSource;
	//Set the fields of our structure
	src->fi = const_cast<ting::fs::File*>(&fi);
	//set pointers to the buffers
	src->pub.bytes_in_buffer = 0;//forces fill_input_buffer on first read
	src->pub.next_input_byte = 0;//until buffer loaded
	//WARNING!!! there's a little bug in the JPEG library. If "infile" is set
	//to 0 the "jpeg_start_decompress()" refuses to work. Set it to 1 then.
//    src->pub..infile=(FILE*)1;//pointer to a file for a standard manager
//    src->pub.buffer=0;//buffer of a standard manager, we do not use it

	jpeg_read_header(&cinfo, TRUE);//read parametrs of a JPEG file

	jpeg_start_decompress(&cinfo);//start decompression

	//TODO:remove this comment
	//Save image information in this Image object
	//this->w=cinfo.output_width;
	//this->h=cinfo.output_height;
	//this->SetNumChannels((Image::E_ImageChannelsNum)cinfo.output_components);
	//Great! Number of channels and bits per pixel are initialized now

	Image::EType imageType;
	switch(cinfo.output_components){
		case 1:
			imageType = Image::GREY;
			break;
		case 2:
			imageType = Image::GREYA;
			break;
		case 3:
			imageType = Image::RGB;
			break;
		case 4:
			imageType = Image::RGBA;
			break;
		default:
			ASSERT_INFO(false, "Image::LoadJPG(): unknown number of components")
			return;
	}
	
	//Set buffer size (allocate memory for image)
	this->Init(cinfo.output_width, cinfo.output_height, imageType);

	//calculate the size of a row in bytes
	int bytesRow = this->Width() * this->NumChannels();

	//TODO: remove this comment
	//Allocate memory for the pic
	//data=(byte*)malloc(h*bytes_row);//new byte[h*bytes_row];

	//Allocate memory for one row. It is an array of rows which
	//contains only one row. JPOOL_IMAGE means that the memory is allocated
	//only for time of this image reading. So, no need to free the memory explicitly.
	JSAMPARRAY buffer = (cinfo.mem->alloc_sarray)(
			j_common_ptr(&cinfo),
			JPOOL_IMAGE,
			bytesRow,
			1
		);
	memset(*buffer, 0, sizeof(JSAMPLE) * bytesRow);

	int y = 0;
	while(cinfo.output_scanline < h){
		//read the string into buffer
		jpeg_read_scanlines(&cinfo, buffer, 1);
		//copy the data to an image
		memcpy(&*this->buf.begin() + bytesRow * y, buffer[0], bytesRow);
		++y;
	}

	jpeg_finish_decompress(&cinfo);//finish file decompression
	jpeg_destroy_decompress(&cinfo);//clean decompression object
}//~Image::LoadJPG()








//=========================================,,   TTTTTTTTT   GGGGGG     AAAAAAA
//===========================================--,,,,,T,,,,,,G,,,,,,G,,,A,,,    A
//=============================================-----T------G----------A-----  A
//====================TGA image read method======---T------G---GGGGG--AAAAAAAAA
//=============================================-----T------G---G--G---A-----  A
//===========================================--'''''T''''''G''''''G'''A'''    A
//=========================================''       T       GGGGGG    A       A
//TODO:
/*
#define M_TGA_RGB 2// This tells us it's a normal RGB (really BGR) file
#define M_TGA_A 3// This tells us it's a ALPHA file (Grayscale I think)
#define M_TGA_RLE 10// This tells us that the targa is Run-Length Encoded (RLE)



void Image::LoadTGA(File& fi){
	ASSERT(!fi.IsOpened())

	M_IMAGE_PRINT(<< "Image::LoadTGA: enter" << std::endl)

	int tmp;//temporary var

	if(this->buf.Size()){
		this->Reset();
	}
	fi.Open(File::READ);
	File::Guard fileCloser(fi);//this will guarant that the file will be closed upon exit
	M_IMAGE_PRINT(<< "Image::LoadTGA: file opened" << std::endl)

	// Read in the length in bytes from the header to the pixel data
	std::uint8_t length = 0;//The length in bytes to the pixels
	{
		ting::Buffer<std::uint8_t> bufWrapper(&length, sizeof(length));
		ASSERT_EXEC(fi.Read(bufWrapper) == bufWrapper.SizeInBytes())
	}

	// Jump over one byte
	fi.SeekFwd(1);
	//fi.Read(&tmp, 1);

	// Read in the imageType (RLE, RGB, etc...)
	std::uint8_t imageType = 0;//The image type (RLE, RGB, Alpha...)
	{
		ting::Buffer<std::uint8_t> bufWrapper(&imageType, sizeof(imageType));
		ASSERT_EXEC(fi.Read(bufWrapper) == bufWrapper.SizeInBytes())
	}

	// Skip past general information we don't care about
	//fi.Read(&tmp, 4);
	fi.SeekFwd(4);
	//fi.Read(&tmp, 4);
	fi.SeekFwd(4);
	//fi.Read(&tmp, 1);
	fi.SeekFwd(1);
	

	//Read the width, height and bits per pixel (16, 24 or 32)
	std::uint8_t bits = 0;//The bits per pixel for the image (16, 24, 32)
	{
		std::uint16_t width = 0;
		std::uint16_t height = 0;
		fi.Read(&width, sizeof(std::uint16_t));
		fi.Read(&height, sizeof(std::uint16_t));
		fi.Read(&bits, sizeof(std::uint8_t));
		unsigned bitspp = bits;
		if(bitspp < 24){
			bitspp = 24;
		}
		
		//TODO: use image type instead of bpp
		this->Init(width, height, (Image::EType)(bitspp >> 3));//also allocate memory for image data
	}

	//Now we move the file pointer to the pixel data (Seek forward)
	for(uint j=0; j<(uint(length)+1); ++j) fi.Read(&tmp, 1);

	uint stride=0;//The stride (channels*width)

	//Check if the image is RLE compressed or not
	if(imageType != M_TGA_RLE){
		M_IMAGE_PRINT(<<"Image::LoadTGA: image is not RLE compressed"<<std::endl)
		// Check if the image is a 24 or 32-bit image
		if(bits==24 || bits==32){
			M_IMAGE_PRINT(<<"Image::LoadTGA: image is 8 bits per channel"<<std::endl)
			//channels=bits/8;// Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
			//TODO: move stride out of the IF because it is calculated in the else too
			stride=this->NumChannels()*this->W();// Next, we calculate the stride
			//data=((unsigned char*)malloc(sizeof(byte)*stride*h));//Allocate memory for the pixels.
			M_ASSERT(this->buf.Size() == stride*this->H())

			M_IMAGE_PRINT(<<"Image::LoadTGA: this->buf.Buf()="<<((void*)(this->buf.Buf()))<<std::endl
						  <<"    this->buf.Buf()+this->buf.Size()="<<(this->buf.Buf()+this->buf.Size())<<std::endl)

			// Load in all the pixel data line by line
			for(uint y=0; y < this->H(); ++y){
				// Store a pointer to the current line of pixels
				byte *pLine=this->buf.Buf()+stride*y;
				M_ASSERT( (this->buf.Buf()+this->buf.Size())>pLine && this->buf.Buf()<=pLine)
				// Read in the current line of pixels
				M_IMAGE_PRINT(<<"Image::LoadTGA: going to read image line "<<y<<std::endl)
				fi.Read(pLine, stride);
				M_IMAGE_PRINT(<<"Image::LoadTGA: image line read finished"<<std::endl)

				// Go through all of the pixels and swap the B and R values since TGA
				// files are stored as BGR instead of RGB (or use GL_BGR_EXT versus GL_RGB)
				for(uint i=0; i<stride; i+=this->NumChannels()){
					//byte temp=pLine[i];
					//pLine[i]=pLine[i+2];
					//pLine[i+2]=temp;
					Exchange(pLine[i], pLine[i+2]);
				}
			}
		}else if(bits==16){//Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
			M_IMAGE_PRINT(<<"Image::LoadTGA: image is 16 bits per pixel"<<std::endl)
			u16 pixels=0;
			u16 r=0, g=0, b=0;

			// Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
			// We then calculate the stride and allocate memory for the pixels.
			//channels=3;
			stride=this->NumChannels()*this->W();
			//data=(unsigned char*)malloc(sizeof(byte)*stride*h);

			//Load in all the pixel data pixel by pixel
			for(uint i=0; i< (this->W()*this->H()); ++i){
				// Read in the current pixel
				fi.Read(&pixels, sizeof(u16));

				// Convert the 16-bit pixel into an RGB
				b=(pixels&0x1f)<<3;
				g=((pixels>>5)&0x1f)<<3;
				r=((pixels>>10)&0x1f)<<3;

				// This essentially assigns the color to our array and swaps the
				// B and R values at the same time.
				this->buf[i*3+0]=(u8)r;
				this->buf[i*3+1]=(u8)g;
				this->buf[i*3+2]=(u8)b;
			}
		}else
			throw ImageException("unsupported pixel format");// Else return ERR for a bad or unsupported pixel format
	}else{// Else, it must be Run-Length Encoded (RLE)
		M_IMAGE_PRINT(<<"Image::LoadTGA: image is RLE compressed"<<std::endl)
		//Create some variables to hold the rleID, current colors read, channels, & stride.
		byte rleID=0;
		int colorsRead=0;
		//channels=bits/8;
		stride=this->NumChannels()*this->W();

		// Next we want to allocate the memory for the pixels and create an array,
		// depending on the channel count, to read in for each pixel.
		//data=(unsigned char*)malloc(sizeof(byte)*stride*h);
		byte pColors[Image::Image_MAX_CHANNELS_PLUS_ONE-1];//=(byte*)malloc(sizeof(byte)*channels);

		// Load in all the pixel data
		uint i=0;
		while(i<this->W()*this->H()){
			// Read in the current color count + 1
			fi.Read(&rleID, sizeof(byte));

			// Check if we don't have an encoded string of colors
			if(rleID<128){
				// Increase the count by 1
				rleID++;

				// Go through and read all the unique colors found
				while(rleID){
					// Read in the current color
					fi.Read(pColors, sizeof(byte)*this->NumChannels());

					// Store the current pixel in our image array
					this->buf[colorsRead+0]=pColors[2];
					this->buf[colorsRead+1]=pColors[1];
					this->buf[colorsRead+2]=pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits==32) this->buf[colorsRead+3]=pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					++i;
					--rleID;
					colorsRead+=this->NumChannels();
				}
			}else{// Else, let's read in a string of the same character
				// Minus the 128 ID + 1 (127) to get the color count that needs to be read
				rleID-=127;

				//Read in the current color, which is the same for a while
				fi.Read(pColors, sizeof(byte)*this->NumChannels());

				// Go and read as many pixels as are the same
				while(rleID){
					// Assign the current pixel to the current index in our pixel array
					this->buf[colorsRead+0]=pColors[2];
					this->buf[colorsRead+1]=pColors[1];
					this->buf[colorsRead+2]=pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits==32) this->buf[colorsRead+3]=pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					++i;
					--rleID;
					colorsRead+=this->NumChannels();
				}
			}
		}
	}
	fi.Close();//Close the file pointer that opened the file
}//~Image::LoadTGA()
*/



void Image::Load(const ting::fs::File& fi){
	std::string ext = fi.Ext();

	if(ext == "png"){
//		TRACE(<< "Image::Load(): loading PNG image" << std::endl)
		this->LoadPNG(fi);
	}else if(ext == "jpg"){
//		TRACE(<< "Image::Load(): loading JPG image" << std::endl)
		this->LoadJPG(fi);
	}/*else if(ext == "tga"){
//		TRACE(<< "Image::Load(): loading TGA image" << std::endl)
		this->LoadTGA(fi);
	}*/else{
		throw Image::Exc("Image::Load(): unknown image format");
	}
}

