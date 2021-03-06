// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenColorIO Project.


#ifndef INCLUDED_OCIO_GLSL_H_
#define INCLUDED_OCIO_GLSL_H_

#include <vector>

#include <OpenColorIO/OpenColorIO.h>


OCIO_NAMESPACE_ENTER
{

class OpenGLBuilder;
typedef OCIO_SHARED_PTR<OpenGLBuilder> OpenGLBuilderRcPtr;


// This is a reference implementation showing how to do the texture upload & allocation, 
// and the program compilation for the GLSL shader language.

class OpenGLBuilder
{
    struct TextureId
    {
        unsigned m_id = -1;
        std::string m_name;
        unsigned m_type = -1;

        TextureId(unsigned id, const std::string& name, unsigned type)
            : m_id(id)
            , m_name(name)
            , m_type(type)
        {}
    };

    typedef std::vector<TextureId> TextureIds;

    // Uniform are used for dynamic parameters.
    class Uniform
    {
    public:
        Uniform(const std::string & name,
                DynamicPropertyRcPtr v)
            : m_name(name)
            , m_value(v)
            , m_handle(0)
        {
        }

        void setUp(unsigned program);

        void use();

        DynamicPropertyRcPtr & getValue();

    private:
        Uniform() = delete;
        std::string m_name;
        DynamicPropertyRcPtr m_value;
        GLuint m_handle;
    };
    typedef std::vector<Uniform> Uniforms;

public:
    // Create an OpenGL builder using the GPU shader information from a specific processor
    static OpenGLBuilderRcPtr Create(const GpuShaderDescRcPtr & gpuShader);

    ~OpenGLBuilder();

    inline void setVerbose(bool verbose) { m_verbose = verbose; }
    inline bool isVerbose() const { return m_verbose; }

    // Allocate & upload all the needed textures
    //  (i.e. the index is the first available index for any kind of textures).
    void allocateAllTextures(unsigned startIndex);
    void useAllTextures();

    // Update all uniforms.
    void useAllUniforms();

    // Build the complete shader program which includes the OCIO shader program 
    // and the client shader program.
    unsigned buildProgram(const std::string & clientShaderProgram);
    void useProgram();
    unsigned getProgramHandle();

    // Determine the maximum width value of a texture
    // depending of the graphic card and its driver.
    static unsigned GetTextureMaxWidth();

protected:
    OpenGLBuilder(const GpuShaderDescRcPtr & gpuShader);

    // Prepare all the needed uniforms.
    void linkAllUniforms();

    void deleteAllTextures();
    void deleteAllUniforms();

private:
    OpenGLBuilder();
    OpenGLBuilder(const OpenGLBuilder&);
    OpenGLBuilder& operator=(const OpenGLBuilder&);

    const GpuShaderDescRcPtr m_shaderDesc; // Description of the fragment shader to create
    unsigned m_startIndex;                 // Starting index for texture allocations
    TextureIds m_textureIds;               // Texture ids of all needed textures
    Uniforms m_uniforms;                   // Vector of dynamic parameters
    unsigned m_fragShader;                 // Fragment shader identifier
    unsigned m_program;                    // Program identifier
    std::string m_shaderCacheID;           // Current shader program key
    bool m_verbose;                        // Print shader code to std::cout for debugging purposes
};

}
OCIO_NAMESPACE_EXIT

#endif // INCLUDED_OCIO_GLSL_H_

