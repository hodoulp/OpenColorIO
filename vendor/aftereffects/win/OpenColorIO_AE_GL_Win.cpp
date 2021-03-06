// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenColorIO Project.


#include "OpenColorIO_AE_GL.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

static HWND         g_win = NULL;
static HDC          g_hdc = NULL;
static HGLRC        g_context = NULL;
static GLuint       g_framebuffer = GL_INVALID_VALUE;


static bool HaveRequiredExtensions()
{
    const GLubyte *strVersion = glGetString(GL_VERSION);
    const GLubyte *strExt = glGetString(GL_EXTENSIONS);
    
    if(strVersion == NULL)
        return false;

#define CheckExtension(N) glewIsExtensionSupported(N)

    return (GLEW_VERSION_2_0 &&
            CheckExtension("GL_ARB_color_buffer_float") &&
            CheckExtension("GL_ARB_texture_float") &&
            CheckExtension("GL_ARB_vertex_program") &&
            CheckExtension("GL_ARB_vertex_shader") &&
            CheckExtension("GL_ARB_texture_cube_map") &&
            CheckExtension("GL_ARB_fragment_shader") &&
            CheckExtension("GL_ARB_draw_buffers") &&
            CheckExtension("GL_ARB_framebuffer_object") );
}


void GlobalSetup_GL()
{
    GLenum init = glewInit();

    if(init != GLEW_OK)
        return;


    WNDCLASSEX winClass; 
    MSG        uMsg;

    memset(&uMsg,0,sizeof(uMsg));

    winClass.lpszClassName = "OpenColorIO_AE_Win_Class";
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc   = DefWindowProc;
    winClass.hInstance     = NULL;
    winClass.hIcon         = NULL;
    winClass.hIconSm       = NULL;
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;
    
    if( !( RegisterClassEx(&winClass) ) )
        return;

    g_win = CreateWindowEx( NULL, "OpenColorIO_AE_Win_Class", 
                             "OpenGL-using FBOs in AE",
                                0, 0, 
                                0, 50, 50,
                                NULL, 
                                NULL, 
                                NULL,
                                NULL    );

    if(g_win == NULL)
        return;

    g_hdc = GetDC(g_win);


    int pixelFormat;

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory( &pfd, sizeof( pfd ) );

    pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion        = 1;
    pfd.dwFlags         = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType      = PFD_TYPE_RGBA;
    pfd.cColorBits      = 128;
    pfd.cDepthBits      = 32;
    pfd.cStencilBits    = 32;
    pfd.iLayerType      = PFD_MAIN_PLANE;
    
    pixelFormat = ChoosePixelFormat(g_hdc, &pfd);

    BOOL set_format = SetPixelFormat(g_hdc, pixelFormat, &pfd);
    
    if(!set_format)
    {
        GlobalSetdown_GL();
        return;
    }

    g_context = wglCreateContext(g_hdc);

    glFlush();

    if(g_context == NULL)
    {
        GlobalSetdown_GL();
        return;
    }
    

    SetPluginContext();
    

    GLint units;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &units);
    
    
    if( !HaveRequiredExtensions() || units < 2)
    {
        GlobalSetdown_GL();
        SetAEContext();
        return;
    }
    
    glGenFramebuffersEXT(1, &g_framebuffer);
    
    
    SetAEContext();
}


bool HaveOpenGL()
{
    return (g_context != NULL && g_win != NULL);
}


static HDC g_ae_hdc;
static HGLRC g_ae_context;

void SetPluginContext()
{
    g_ae_hdc = wglGetCurrentDC();
    g_ae_context = wglGetCurrentContext();

    wglMakeCurrent(g_hdc, g_context);
}


void SetAEContext()
{
    wglMakeCurrent(g_ae_hdc, g_ae_context);
}


GLuint GetFrameBuffer()
{
    return g_framebuffer;
}


void GlobalSetdown_GL()
{
    if(g_framebuffer != GL_INVALID_VALUE)
    {
        glDeleteFramebuffersEXT(1, &g_framebuffer);
        g_framebuffer = GL_INVALID_VALUE;
    }
    
    if(g_context)
    {
        wglDeleteContext(g_context);
        g_context = NULL;
    }

    if(g_win)
    {
        ReleaseDC(g_win, g_hdc);
        g_win = NULL;
        g_hdc = NULL;

        UnregisterClass("OpenColorIO_AE_Win_Class", NULL);
    }
}
