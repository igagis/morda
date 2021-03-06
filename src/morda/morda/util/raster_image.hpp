#pragma once

#include <utki/span.hpp>

#include <papki/file.hpp>

#include <r4/vector.hpp>

namespace morda{

/**
 * @brief Utility class for loading and manipulating raster images.
 */
class raster_image final{
public:
	/**
	 * @brief Image color depth.
	 */
	enum class color_depth{
		unknown = 0,
		grey = 1, // 1 channel. Only Grey channel
		grey_alpha = 2, // 2 channels. Grey with Alpha channel
		rgb = 3, // 3 channels. Red Green Blue channels
		rgba = 4  // 4 channels. RGBA format (4 channels)
	};

private:
	color_depth colorDepth_v;
	r4::vector2<unsigned> dims_v = r4::vector2<unsigned>(0);
	std::vector<std::uint8_t> buf_v;//image pixels data

public:
	/**
	 * @brief Default constructor.
	 * Creates uninitialized Image object.
	 */
	raster_image() :
			colorDepth_v(color_depth::unknown)
	{}

	raster_image(const raster_image& im) = default;

	/**
	 * @brief Constructor.
	 * Creates image with given parameters, but uninitialized contents.
	 * @param dimensions - image dimensions.
	 * @param colorDepth - color depth.
	 */
	raster_image(r4::vector2<unsigned> dimensions, color_depth colorDepth){
		this->init(dimensions, colorDepth);
	}

	/**
	 * @brief Constructor.
	 * Creates an image with given parameters and initializes image data from given memory buffer.
	 * @param dimensions - image dimensions.
	 * @param colorDepth - color depth.
	 * @param srcBuf - pointer to memory buffer to take image data from.
	 */
	raster_image(r4::vector2<unsigned> dimensions, color_depth colorDepth, const std::uint8_t* srcBuf);

	/**
	 * @brief Constructor.
	 * Creates Image object with given dimensions and copies a region from source image to it.
	 * @param pos - position on the source image to copy from.
	 * @param dimensions - dimensions of the area to copy.
	 * @param src - source image to copy area from.
	 */
	raster_image(r4::vector2<unsigned> pos, r4::vector2<unsigned> dimensions, const raster_image& src);

	/**
	 * @brief Constructor.
	 * Creates an image by loading it from file. Supported file types are PNG and JPG.
	 * @param f - file to load image from.
	 */
	raster_image(const papki::file& f){
		this->load(f);
	}

	/**
	 * @brief Get image dimensions.
	 * @return Image dimensions.
	 */
	const r4::vector2<unsigned>& dims()const noexcept{
		return this->dims_v;
	}

	/**
	 * @brief Get color depth.
	 * @return Bits per pixel.
	 */
	unsigned bits_per_pixel()const{
		return this->num_channels() * 8;
	}

	/**
	 * @brief Get color depth.
	 * @return Number of color channels.
	 */
	unsigned num_channels()const{
		return unsigned(this->colorDepth_v);
	}

	/**
	 * @brief Get color depth.
	 * @return Color depth type.
	 */
	color_depth depth()const{
		return this->colorDepth_v;
	}

	/**
	 * @brief Get pixel data.
	 * @return Pixel data of the image.
	 */
	utki::span<std::uint8_t> pixels(){
		return utki::make_span(this->buf_v);
	}

	/**
	 * @brief Get pixel data.
	 * @return Pixel data of the image.
	 */
	utki::span<const std::uint8_t> pixels()const{
		return utki::make_span(this->buf_v);
	}

public:
	/**
	 * @brief Initialize this image object with given parameters.
	 * Pixel data remains uninitialized.
	 * @param dimensions - image dimensions.
	 * @param colorDepth - color depth.
	 */
	void init(r4::vector2<unsigned> dimensions, color_depth colorDepth);


	/**
	 * @brief Reset this Image object to uninitialized state.
	 * All resources are freed.
	 */
	void reset();

	/**
	 * @brief Fill each image channel with specified value.
	 * @param val - value to use when filling pixel data.
	 */
	void clear(std::uint8_t  val = 0);

	/**
	 * @brief Fill specified color channel with given value.
	 * @param chan - index of color channel to clear.
	 * @param val - value to use for filling.
	 */
	void clear(unsigned chan, std::uint8_t val = 0);

	/**
	 * @brief Flip image vertically.
	 */
	void flip_vertical();

	/**
	 * @brief Blit another image to this image.
	 * Copy the whole given image to specified location on this image.
	 * @param pos - destination position.
	 * @param src - image to copy to this image.
	 */
	void blit(r4::vector2<unsigned> pos, const raster_image& src);

	/**
	 * @brief Blit another image to this image for desired color channels only.
	 * Copy specified color channel of the whole given image to specified color
	 * channel and specified location on this image.
	 * @param pos - destination position.
	 * @param src - image to copy to this image.
	 * @param dstChan - index of destination color channel.
	 * @param srcChan - index of source color channel.
	 */
	void blit(r4::vector2<unsigned> pos, const raster_image& src, unsigned dstChan, unsigned srcChan);

	/**
	 * @brief Get reference to specific channel for given pixel.
	 * @param x - X pixel location.
	 * @param y - Y pixel location.
	 * @param chan - channel index to get reference to.
	 * @return Reference to uint8_t representing a single color channel of given pixel.
	 */
	const std::uint8_t& pix_chan(unsigned x, unsigned y, unsigned chan)const{
		auto i = (y * this->dims().x() + x) * this->num_channels() + chan;
		ASSERT(i < this->buf_v.size())
		return this->buf_v[i];
	}

	/**
	 * @brief Get reference to specific channel for given pixel.
	 * @param x - X pixel location.
	 * @param y - Y pixel location.
	 * @param chan - channel number to get reference to.
	 * @return Reference to uint8_t representing a single color channel of given pixel.
	 */
	std::uint8_t& pix_chan(unsigned x, unsigned y, unsigned chan){
		auto i = (y * this->dims().x() + x) * this->num_channels() + chan;
		ASSERT(i < this->buf_v.size())
		return this->buf_v[i];
	}

	/**
	 * @brief Load image from PNG file.
	 * @param f - PNG file.
	 */
	void load_png(const papki::file& f);

	/**
	 * @brief Load image from JPG file.
	 * @param f - JPG file.
	 */
	void load_jpg(const papki::file& f);

	/**
	 * @brief Load image from file.
	 * It will try to determine the file type from file name.
	 * @param f - file to load image from.
	 */
	void load(const papki::file& f);
};



}
