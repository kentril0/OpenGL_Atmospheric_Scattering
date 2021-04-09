#version 450 core

#define M_PI 3.1415926535897932384626433832795

in vec3 fsPosition;     // Position of the fragment
in vec3 fsNormal;
in vec2 fsTexCoord;

out vec4 finalColor;

// TODO other constants
uniform vec3 viewPos;   // Position of the viewer
uniform vec3 sunPos;    // Position of the sun, light direction

// Number of samples along the view ray and light ray
uniform int viewSamples;
uniform int lightSamples;

uniform float I_sun;    // Intensity of the sun
uniform float R_e;      // Radius of the planet [m]
uniform float R_a;      // Radius of the atmosphere [m]
uniform vec3  beta_R;   // Rayleigh scattering coefficient
uniform float beta_M;   // Mie scattering coefficient
uniform float H_R;      // Rayleigh scale height
uniform float H_M;      // Mie scale height
uniform float g;        // Mie scattering direction - 
                        //  - anisotropy of the medium

/**
 * @brief
 * @param o Origin of the ray
 * @param d Direction of the ray
 * @param r Radius of the sphere
 * @return Roots
 */
vec2 raySphereIntersection(vec3 o, vec3 d, float r)
{
    // Solving analytically as a quadratic function
    //  assumes that the sphere is centered at the origin
    // f(x) = a(x^2) + bx + c
    float a = dot(d, d);
    float b = 2.0 * dot(d, o);
    float c = dot(o, o) - r * r;

    // Discriminant or delta
    float delta = b * b - 4.0 * a * c;

    // Roots not found
    if (delta < 0.0) {
      // TODO
      return vec2(1e5, -1e5);
    }

    float sqrtDelta = sqrt(delta);
    // TODO order??
    return vec2((-b - sqrtDelta) / (2.0 * a),
                (-b + sqrtDelta) / (2.0 * a));
}

/**
 * @brief TODO
 * @param ray Ray direction norm(fsPosition - viewPos)
 * @param origin Origin of the ray "r"
 * @return 
 * TODO CONSTS
 */
vec3 computeSkyColor(vec3 ray, vec3 origin)
{
    // Normalize the light direction
    // TODO normalize beforehand
    vec3 sunDir = normalize(sunPos);

    vec2 t = raySphereIntersection(origin, ray, R_a);
    // Intersects behind
    if (t.x > t.y) {
        return vec3(0.0, 0.0, 0.0);
    }

    // Distance between samples - length of each segment
    t.y = min(t.y, raySphereIntersection(origin, ray, R_e).x);
    float segmentLen = (t.y - t.x) / float(viewSamples);

    // TODO t min
    float tCurrent = 0.0f; 

    // Rayleigh and Mie contribution
    vec3 sum_R = vec3(0);
    vec3 sum_M = vec3(0);

    // Optical depth 
    float optDepth_R = 0.0;
    float optDepth_M = 0.0;

    // Mu: the cosine angle between the sun and ray direction
    float mu = dot(ray, sunDir);
    float mu_2 = mu * mu;
    
    //--------------------------------
    // Rayleigh and Mie Phase functions
    float phase_R = 3.0 / (16.0 * M_PI) * (1.0 + mu_2);

    float g_2 = g * g;
    float phase_M = 3.0 / (8.0 * M_PI) * 
                          ((1.0 - g_2) * (1.0 + mu_2)) / 
                          ((2.0 + g_2) * pow(1.0 + g_2 - 2.0 * g * mu, 1.5));
    // Sample along the view ray
    for (int i = 0; i < viewSamples; ++i)
    {
        // Middle point of the sample position
        vec3 vSample = origin + ray * (tCurrent + segmentLen * 0.5);

        // Height of the sample above the planet
        float height = length(vSample) - R_e;

        // Optical depth for Rayleigh and Mie scattering for current sample
        float h_R = exp(-height / H_R) * segmentLen;
        float h_M = exp(-height / H_M) * segmentLen;
        optDepth_R += h_R;
        optDepth_M += h_M;

        //--------------------------------
        // Secondary - light ray
        
        float segmentLenLight = 
            raySphereIntersection(vSample, sunDir, R_a).y / float(lightSamples);
        float tCurrentLight = 0.0;

        // Light optical depth 
        float optDepthLight_R = 0.0;
        float optDepthLight_M = 0.0;

        // Sample along the light ray
        for (int j = 0; j < lightSamples; ++j)
        {
            // Position of the light ray sample
            vec3 lSample = tCurrent + sunDir * 
                           (tCurrentLight + segmentLenLight * 0.5);
            // Height of the light ray sample
            float heightLight = length(lSample) - R_e;

            // TODO check sample above the ground
            
            optDepthLight_R += exp(-heightLight / H_R) * segmentLenLight;
            optDepthLight_M += exp(-heightLight / H_M) * segmentLenLight;

            // Next light sample
            tCurrentLight += segmentLenLight;
        }
        // TODO check sample above ground

        // Attenuation of the light for both Rayleigh and Mie optical depth
        //  Mie extenction coeff. = 1.1 of the Mie scattering coeff.
        vec3 att = exp(-(beta_R * (optDepth_R + optDepthLight_R) + 
                         beta_M * 1.1 * (optDepth_M + optDepthLight_M)));
        // Accumulate the scattering 
        sum_R += att * h_R;
        sum_M += att * h_M;

        // Next view sample
        tCurrent += segmentLen;
    }
    return I_sun * (sum_R * beta_R * phase_R + sum_M * beta_M * phase_M);
}

void main()
{
    vec3 acolor = computeSkyColor(normalize(fsPosition - viewPos), viewPos);

    // Apply exposure
    // TODO tone mapping
    //acolor = 1.0 - exp(-1.0 * acolor);

    finalColor = vec4(acolor, 1.0);
    //sfinalColor = vec4(0.1, 0.1, 0.1, 1.0);
}

