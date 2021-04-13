/**********************************************************
 * < Interactive Atmospheric Scattering >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date April, 2021
 * @file atmosphere.hpp
 * @brief Representation of an atmoshpere.
 *********************************************************/

#pragma once

#include "opengl/shader.hpp"
#include "scene/mesh.hpp"

#include <memory>


/**
 * @brief Represnts the properties of an atmosphere along with methods
 *  to render it.
 */
class Atmosphere
{
public:
    Atmosphere(const std::shared_ptr<Shader>& drawMeshProgram, 
               Mesh* sphereModel)   // TODO Dangerous
        : m_drawMeshProgram(drawMeshProgram),
          m_sphereModel(sphereModel)
    { 
        set_defaults();

        m_atmosphereProgram = std::make_unique<Shader>("shaders/draw_atmosphere.vert",
                                                       "shaders/draw_atmosphere.frag");
    }

    // @brief Sets defaut to Earth-like atmosphere
    void set_defaults()
    {
        viewSamples = defViewSamples;
        lightSamples = defLightSamples;

        sunDir = defSunDir;
        set_sunAngle(defSunAngle); //M_PI * 0.5f;
        I_sun = e_I_sun;
        set_earthRadius(e_R_e);
        set_atmosRadius(e_R_a);
        beta_R = e_beta_R;
        beta_M = e_beta_M;
        H_R = e_H_R;
        H_M = e_H_M;
        g = e_g;
    }
    void set_sunDefaults()
    {
        m_animateSun = false;
        sunDir = defSunDir;
        set_sunAngle(defSunAngle);
        I_sun = e_I_sun;
    }

    void set_rayleighDefaults()
    {
        beta_R = e_beta_R;
        H_R = e_H_R;
    }

    void set_mieDefaults()
    {
        beta_M = e_beta_M;
        H_M = e_H_M;
        g = e_g;
    }

    void set_sizeDefaults()
    {
        set_earthRadius(e_R_e);
        set_atmosRadius(e_R_a);
        m_renderEarth = false;
    }

    void draw(float delta)
    {
        // 1. draw the Earth (or any like planet)
        if (m_renderEarth)
        {
            m_drawMeshProgram->use();
            //m_drawMeshProgram->set_mat4("MVP",  m_projView * m_modelEarth);
            m_drawMeshProgram->set_mat4("MVP",  m_proj * m_view * m_modelEarth);
            m_sphereModel->draw();
        }

        // 2. Setup properties of the atmosphere
        m_atmosphereProgram->use();
        m_atmosphereProgram->set_mat4("M", m_modelAtmos);
        //m_atmosphereProgram->set_mat4("MVP", m_projView * m_modelAtmos);
        m_atmosphereProgram->set_mat4("MVP", m_proj * m_view * m_modelAtmos);

        m_atmosphereProgram->set_vec3("viewPos", m_viewPos);
        m_atmosphereProgram->set_int("viewSamples", viewSamples);
        m_atmosphereProgram->set_int("lightSamples", lightSamples);

        m_atmosphereProgram->set_float("I_sun", I_sun);
        m_atmosphereProgram->set_float("R_e", R_e);
        m_atmosphereProgram->set_float("R_a", R_a);
        m_atmosphereProgram->set_vec3("beta_R", beta_R);
        m_atmosphereProgram->set_float("beta_M", beta_M);
        m_atmosphereProgram->set_float("H_R", H_R);
        m_atmosphereProgram->set_float("H_M", H_M);
        m_atmosphereProgram->set_float("g", g);

        // 3. Handle GUI stuff
        m_atmosphereProgram->set_float("toneMappingFactor", m_toneMapping * 1.0);
        if (m_animateSun)
        {
            m_sunAngle = glm::mod(m_sunAngle + 0.5 * delta, M_PI + glm::radians(20.f));
            sunDir.y = glm::sin(m_sunAngle);
            sunDir.z = -glm::cos(m_sunAngle);
        }

        m_atmosphereProgram->set_vec3("sunPos", sunDir);

        // 4. draw the atmosphere
        m_sphereModel->draw();
    }

    // ----------------------------------------------------------------------------
    // Getters
    // ----------------------------------------------------------------------------
    const glm::vec3& get_viewPos() { return m_viewPos; }
    int get_viewSamples() { return viewSamples; }
    int get_lightSamples() { return lightSamples; }

    bool is_toneMapping() { return m_toneMapping; }
    bool is_animateSun() { return m_animateSun; }
    float get_sunAngle() { return m_sunAngle; }

    float get_earthRadius() { return R_e; }
    float get_atmosRadius() { return R_a; }

    const glm::vec3& get_sunDir() { return sunDir; }
    float get_sunIntensity() { return I_sun; }

    const glm::vec3& get_rayleighScattering() { return beta_R; }
    float get_rayleighScaleHeight() { return H_R; }

    float get_mieScattering() { return beta_M; }
    float get_mieScaleHeight() { return H_M; }
    float get_mieScatteringDir() { return g; }

    bool is_renderEarth() { return m_renderEarth; }

    // ----------------------------------------------------------------------------
    // Setters
    // ----------------------------------------------------------------------------
    void set_projView(const glm::mat4& proj, const glm::mat4& view)
    {
        //m_projView = projView;
        m_proj = proj;
        m_view = view;
    }

    void set_viewPos(const glm::vec3& cameraPos)
    {
        m_viewPos = cameraPos;
    }

    void set_viewSamples(int samples) { viewSamples = samples; }
    void set_lightSamples(int samples) { lightSamples = samples; }

    void set_toneMapping(bool b) { m_toneMapping = b; }
    void set_animateSun(bool b) { m_animateSun = b; }
    // @param angle in radians
    void set_sunAngle(float angle) {
        // m_sunAngle = glm::mod(angle, M_PI + 0.1); TODO
        m_sunAngle = angle;
        sunDir.y = glm::sin(m_sunAngle);
        sunDir.z = -glm::cos(m_sunAngle);
    }

    void set_sunDir(const glm::vec3 dir) { sunDir = dir; }
    void set_sunIntensity(float I) { I_sun = I; }

    void set_earthRadius(float R)
    {
        R_e = R;
        modelEarth();
    }
    
    void set_atmosRadius(float R)
    {
        R_a = R;
        modelAtmos();
    }

    void set_rayleighScattering(const glm::vec3 beta_s) { beta_R = beta_s; }
    void set_rayleighScaleHeight(float H) { H_R = H; }

    void set_mieScattering(float beta_s) { beta_M = beta_s; }
    void set_mieScaleHeight(float H) { H_M = H; }
    void set_mieScatteringDir(float d) { g = d; }

    void set_renderEarth(bool b) { m_renderEarth = b; }

private:
    // ----------------------------------------------------------------------------
    // Rendering 

    std::unique_ptr<Shader> m_atmosphereProgram; 
    std::shared_ptr<Shader> m_drawMeshProgram;
    const Mesh* m_sphereModel;

    glm::mat4 m_modelAtmos;    ///< Model matrix for the atmossphere
    glm::mat4 m_modelEarth;    ///< Model matrix for the planet
    //glm::mat4 m_projView;      ///< Camera's projection view matrix
    glm::mat4 m_proj;
    glm::mat4 m_view;

    void modelAtmos() {
        m_modelAtmos = glm::scale(glm::mat4(1.0f), glm::vec3(R_a, R_a, R_a));
    }
    void modelEarth() {
        m_modelEarth = glm::scale(glm::mat4(1.0f), glm::vec3(R_e, R_e, R_e));
    }

    glm::vec3 m_viewPos;    ///< Position of the viewer, camera
    int viewSamples;        ///< Number of samples along the view (primary) ray
    int lightSamples;       ///< Number of samples along the light (secondary) ray

    // ----------------------------------------------------------------------------
    // GUI stuff
    bool m_toneMapping = true;  ///< Whether tone mapping function is applied

    bool m_animateSun = false;
    float m_sunAngle;

    bool m_renderEarth = false;

    // ----------------------------------------------------------------------------
    // Atmospheric constants
    // ----------------------------------------------------------------------------
    glm::vec3 sunDir;       ///< Direction of the light rays
    float I_sun;            ///< Intensity of the sun
    float R_e;              ///< Radius of the Earth
    float R_a;              ///< Radius of the atmosphere

    glm::vec3 beta_R;       ///< Rayleigh scattering coefficient
    float H_R;              ///< Rayleigh scale height

    float beta_M;           ///< Mie scattering coefficient
    float H_M;              ///< Mie scale height
    float g;                ///< Mie scattering direction - anisotropy of the medium

    // ----------------------------------------------------------------------------
    // Defaults
    const int defViewSamples = 16;
    const int defLightSamples = 8;
    const float defSunAngle = glm::radians(1.f);

    const glm::vec3 defSunDir = glm::vec3(0, 1, 0);

    // Earth presets in [km]
    const float e_I_sun = 20.f;
    const float e_R_e = 6360.;       // 6360e3
    const float e_R_a = 6420.;       // 6420e3;
    const glm::vec3 e_beta_R = glm::vec3(3.8e-3f, 13.5e-3f, 33.1e-3f);
        //(3.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel implementation
        //(5.8e-6f, 13.5e-6f, 33.1e-6f);    // scrathapixel web
        //(5.5e-3f, 15.0e-3f, 22.4e-3f);    // 
    const float e_beta_M = 21e-3f;          // 21e-6f
    const float e_H_R = 7.994;            // 7994, 100
    const float e_H_M = 1.200;            // 1200, 20
    const float e_g = 0.888;

    // Conversions
    const float M_2_KM = 0.001;
    const float KM_2_M = 1000.0;
};