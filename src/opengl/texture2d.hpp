/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file texture2d.hpp
 * @brief OpenGL 2D texture abstraction
 *********************************************************/

#pragma once

#include <glm/glm.hpp>
#include <string>


class Texture2D
{
public:

    // TODO texture is prebound after creation, expects to be used right after!
	/**
	 * @brief Creates an empty 2D texture object.
	 *     	  Defaults: Image format: RGBA, internal format: RGBA8
     *     	  Expects image to be loaded later.
	 */
	Texture2D(bool mipmaps = true);

    /**
     * @param alpha Whether alpha channel is used
     */
	Texture2D(const std::string& filename, 
			  bool alpha = true, 
			  bool mipmaps = true); 

	/**
	 * @brief Creates 2D texture object.
     * @param data Supplied image data
     * @param w Image width
     * @param h Image height
	 */
	Texture2D(const uint8_t* data, 
              uint32_t w, uint32_t h,
			  bool alpha = true, 
			  bool mipmaps = true); 

	~Texture2D();

	/**
	 * @brief Loads a 2D image as a texture, 
	 * @param filename Name of the texture image
 	 * @param alpha Whether the image has an alpha channel
 	 */
	void load(const std::string& filename, bool alpha = true);

	/**
	 * @brief Upload BYTE data to the texture object.
	 * @param data Data in the image format as already set.
     * @param width Width of the data
     * @param height Height of the data
 	 */
	void upload(const uint8_t* data, int width, int height);

    void upload(const float* data, int width, int height);

	/**
 	 * @brief Bind the texture object
	 */
	void bind() const;

	void unbind() const;
	
	/**
 	 * @brief Bind the texture to the texture unit
	 * @param unit Number of the texture unit
	 */
	void bind_unit(uint32_t unit) const;

    /**
     * @brief Activates texture unit 'unit' globally.
     */
    void activate(uint32_t unit) const;

    //------------------------------------------------------------
	// Setters
	void set_internal_format(uint32_t f) { m_internal_format = f; } 

	void set_image_format(uint32_t f) { m_image_format = f; }

    void set_repeat();

    void set_mirrored_repeat();

    void set_clamp_to_edge();

    void set_clamp_to_border(const glm::vec4& border_color);

    void set_custom_wrap(uint32_t wrap);

    void set_custom_wrap(uint32_t wrap_s, uint32_t wrap_t);

    void set_filtering(uint32_t min_f, uint32_t mag_f);

    void set_linear_filtering();

    //------------------------------------------------------------
	// Getters
	uint32_t ID() const { return m_id; }

	glm::uvec2 size() const { return glm::uvec2(m_width, m_height); }
private:

    void init_texture();

    // Initializes texture storage with the data
    void set_data_immutable(const uint8_t* data);

    void set_filtering();

    /**
     * @brief Generates mipmaps, expects bind call beforehand */
    void gen_mipmap();

private:
	uint32_t m_id;

	uint32_t m_width, m_height;
	uint32_t m_internal_format, m_image_format;

	bool m_mipmaps;             ///< Whether mipmaps are used
    uint32_t m_filterMin, m_filterMag;
};

