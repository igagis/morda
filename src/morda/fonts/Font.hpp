#pragma once

#include <string>

#include <utki/Buf.hpp>
#include <unikod/utf8.hpp>

#include <kolme/Matrix4.hpp>
#include <kolme/Rectangle.hpp>

#include "../shaders/PosTexShader.hpp"

#include "../config.hpp"

namespace morda{

/**
 * @brief Basic class representing a font.
 */
class Font{
protected:
	//Bounding box holds the dimensions of the largest loaded glyph.
	morda::Rectr boundingBox_v;
	
	Font(){}
	
	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;
	
	/**
	 * @brief Render string of text.
	 * @param matrix - transformation matrix to use when rendering the text.
	 * @param str - string of text to render.
	 * @return An advance to the end of the rendered text string. It can be used to position the next text string when rendering.
	 */
	virtual real renderStringInternal(const morda::Matr4r& matrix, kolme::Vec4f color, const std::u32string& str)const = 0;
	
	/**
	 * @brief Get string advance.
	 * @param str - string of text to get advance for.
	 * @return Advance of the text string.
	 */
	virtual real stringAdvanceInternal(const std::u32string& str)const = 0;
	
	/**
	 * @brief Get bounding box of the string.
	 * @param str - string of text to get the bounding box for.
	 * @return Bounding box of the text string.
	 */
	virtual morda::Rectr stringBoundingBoxInternal(const std::u32string& str)const = 0;
public:
	virtual ~Font()noexcept{}
	
	/**
	 * @brief Render string of text.
	 * @param matrix - transformation matrix to use when rendering.
	 * @param str - string of text to render.
	 * @return Advance of the rendered text string. It can be used to position the next text string when rendering.
	 */
	real renderString(const morda::Matr4r& matrix, kolme::Vec4f color, unikod::Utf8Iterator str)const{
		return this->renderStringInternal(matrix, color, unikod::toUtf32(str));
	}
	
	/**
	 * @brief Render string of text.
	 * @param matrix - transformation matrix to use when rendering.
	 * @param str - string of text to render.
	 * @return Advance of the rendered text string. It can be used to position the next text string when rendering.
	 */
	real renderString(const morda::Matr4r& matrix, kolme::Vec4f color, const std::u32string& str)const{
		return this->renderStringInternal(matrix, color, str);
	}
	
	/**
	 * @brief Render string of text.
	 * @param matrix - transformation matrix to use when rendering.
	 * @param str - string of text to render.
	 * @return Advance of the rendered text string. It can be used to position the next text string when rendering.
	 */
	real renderString(const morda::Matr4r& matrix, kolme::Vec4f color, const char* str)const{
		return this->renderString(matrix, color, unikod::Utf8Iterator(str));
	}
	
	/**
	 * @brief Render string of text.
	 * @param matrix - transformation matrix to use when rendering.
	 * @param str - string of text to render.
	 * @return Advance of the rendered text string. It can be used to position the next text string when rendering.
	 */
	real renderString(const morda::Matr4r& matrix, kolme::Vec4f color, const std::string& str)const{
		return this->renderString(matrix, color, str.c_str());
	}
	
	
	/**
	 * @brief Get string advance.
	 * @param str - string to get advance for.
	 * @return Advance of the string of text.
	 */
	real stringAdvance(unikod::Utf8Iterator str)const{
		return this->stringAdvanceInternal(unikod::toUtf32(str));
	}
	
	/**
	 * @brief Get string advance.
	 * @param str - string to get advance for.
	 * @return Advance of the string of text.
	 */
	real stringAdvance(const std::u32string& str)const{
		return this->stringAdvanceInternal(str);
	}
	
	/**
	 * @brief Get string advance.
	 * @param str - string to get advance for.
	 * @return Advance of the string of text.
	 */
	real stringAdvance(const char* str)const{
		return this->stringAdvance(unikod::Utf8Iterator(str));
	}
	
	/**
	 * @brief Get string advance.
	 * @param str - string to get advance for.
	 * @return Advance of the string of text.
	 */
	real stringAdvance(const std::string& str)const{
		return this->stringAdvance(str.c_str());
	}
	
	
	/**
	 * @brief Get advance of the character.
	 * @param c - character to get advance for.
	 * @return Advance of the character.
	 */
	virtual real charAdvance(char32_t c)const = 0;
	
	
	/**
	 * @brief Get bounding box of the string.
	 * @param str - string of text to get the bounding box for.
	 * @return Bounding box of the text string.
	 */
	morda::Rectr stringBoundingBox(unikod::Utf8Iterator str)const{
		return this->stringBoundingBoxInternal(unikod::toUtf32(str));
	}
	
	/**
	 * @brief Get bounding box of the string.
	 * @param str - string of text to get the bounding box for.
	 * @return Bounding box of the text string.
	 */
	morda::Rectr stringBoundingBox(const std::u32string& str)const{
		return this->stringBoundingBoxInternal(str);
	}
	
	/**
	 * @brief Get bounding box of the string.
	 * @param str - string of text to get the bounding box for.
	 * @return Bounding box of the text string.
	 */
	morda::Rectr stringBoundingBox(const char* str)const{
		return this->stringBoundingBox(unikod::Utf8Iterator(str));
	}
	
	/**
	 * @brief Get bounding box of the string.
	 * @param str - string of text to get the bounding box for.
	 * @return Bounding box of the text string.
	 */
	morda::Rectr stringBoundingBox(const std::string& str)const{
		return this->stringBoundingBox(str.c_str());
	}
	
	/**
	 * @brief Get bounding box of the font.
	 * Bounding box of the font is an 'or' operation of all font's character bounding boxes.
	 * I.e. it is a bounding box into which any font's character will fit.
	 * @return Bounding box of the font.
	 */
	const morda::Rectr& boundingBox()const noexcept{
		return this->boundingBox_v;
	}
};

}
