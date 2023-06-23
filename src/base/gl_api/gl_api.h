#pragma once

// This file provides a configurable OpenGL API.  You can make it mean whatever
//  you want by defining DECLARE_GL_FUNCTION(name, ret, params) and
//  USE_GL_FUNCTION(p_name).
//
// As an example, the following defines make pointers to functions that use
//  C++ trickery to only manifest functions that are required by the program,
//  thus minimizing overhead.
//
//     #define DECLARE_GL_FUNCTION(name, Ret, params) \     // (stopping multi-
//     template <int = 0> \                                 // line comment
//     Ret(APIENTRY* p_##name )params noexcept = \          // warnings)
//         (register_gl_function(&p_##name<>, #name), nullptr);
//     #define USE_GL_FUNCTION(p_name) p_name<>
//
// Only the core profile is included, with no extensions and no deprecated
//  functions.  See https://www.khronos.org/registry/OpenGL-Refpages/gl4/
//  for reference.

#ifndef DECLARE_GL_FUNCTION
#error gl_api.h was included without defining DECLARE_GL_FUNCTION first
#endif
#ifndef USE_GL_FUNCTION
#error gl_api.h was included without defining USE_GL_FUNCTION first
#endif

#include <stdint.h>

#ifndef APIENTRY
#ifdef _WIN32
#define APIENTRY __stdcall
#else
#define APIENTRY
#endif
#endif

//////////////////////////////
//
// The following was processed from a copy of glcorearb.h the Khronos Group's
//  OpenGL-Registry repo at
// https://github.com/KhronosGroup/OpenGL-Registry/blob/master/api/GL/glcorearb.h
//
// Original license statement:
//     Copyright 2013-2020 The Khronos Group Inc.
//     SPDX-License-Identifier: MIT
//
//////////////////////////////

#ifndef GL_VERSION_1_0
#define GL_VERSION_1_0 1
typedef void GLvoid;
typedef unsigned int GLenum;
typedef float GLfloat;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLbitfield;
typedef double GLdouble;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef uint8_t GLubyte;
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000
#define GL_FALSE                          0
#define GL_TRUE                           1
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006
#define GL_QUADS                          0x0007
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
#define GL_NONE                           0
#define GL_FRONT_LEFT                     0x0400
#define GL_FRONT_RIGHT                    0x0401
#define GL_BACK_LEFT                      0x0402
#define GL_BACK_RIGHT                     0x0403
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_LEFT                           0x0406
#define GL_RIGHT                          0x0407
#define GL_FRONT_AND_BACK                 0x0408
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505
#define GL_CW                             0x0900
#define GL_CCW                            0x0901
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_VIEWPORT                       0x0BA2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_WIDTH                  0x1000
#define GL_TEXTURE_HEIGHT                 0x1001
#define GL_TEXTURE_BORDER_COLOR           0x1004
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F
#define GL_TEXTURE                        0x1702
#define GL_COLOR                          0x1800
#define GL_DEPTH                          0x1801
#define GL_STENCIL                        0x1802
#define GL_STENCIL_INDEX                  0x1901
#define GL_DEPTH_COMPONENT                0x1902
#define GL_RED                            0x1903
#define GL_GREEN                          0x1904
#define GL_BLUE                           0x1905
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_POINT                          0x1B00
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
DECLARE_GL_FUNCTION(glCullFace, void, (GLenum mode))
#define glCullFace USE_GL_FUNCTION(p_glCullFace)
DECLARE_GL_FUNCTION(glFrontFace, void, (GLenum mode))
#define glFrontFace USE_GL_FUNCTION(p_glFrontFace)
DECLARE_GL_FUNCTION(glHint, void, (GLenum target, GLenum mode))
#define glHint USE_GL_FUNCTION(p_glHint)
DECLARE_GL_FUNCTION(glLineWidth, void, (GLfloat width))
#define glLineWidth USE_GL_FUNCTION(p_glLineWidth)
DECLARE_GL_FUNCTION(glPointSize, void, (GLfloat size))
#define glPointSize USE_GL_FUNCTION(p_glPointSize)
DECLARE_GL_FUNCTION(glPolygonMode, void, (GLenum face, GLenum mode))
#define glPolygonMode USE_GL_FUNCTION(p_glPolygonMode)
DECLARE_GL_FUNCTION(glScissor, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#define glScissor USE_GL_FUNCTION(p_glScissor)
DECLARE_GL_FUNCTION(glTexParameterf, void, (GLenum target, GLenum pname, GLfloat param))
#define glTexParameterf USE_GL_FUNCTION(p_glTexParameterf)
DECLARE_GL_FUNCTION(glTexParameterfv, void, (GLenum target, GLenum pname, const GLfloat *params))
#define glTexParameterfv USE_GL_FUNCTION(p_glTexParameterfv)
DECLARE_GL_FUNCTION(glTexParameteri, void, (GLenum target, GLenum pname, GLint param))
#define glTexParameteri USE_GL_FUNCTION(p_glTexParameteri)
DECLARE_GL_FUNCTION(glTexParameteriv, void, (GLenum target, GLenum pname, const GLint *params))
#define glTexParameteriv USE_GL_FUNCTION(p_glTexParameteriv)
DECLARE_GL_FUNCTION(glTexImage1D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels))
#define glTexImage1D USE_GL_FUNCTION(p_glTexImage1D)
DECLARE_GL_FUNCTION(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels))
#define glTexImage2D USE_GL_FUNCTION(p_glTexImage2D)
DECLARE_GL_FUNCTION(glDrawBuffer, void, (GLenum buf))
#define glDrawBuffer USE_GL_FUNCTION(p_glDrawBuffer)
DECLARE_GL_FUNCTION(glClear, void, (GLbitfield mask))
#define glClear USE_GL_FUNCTION(p_glClear)
DECLARE_GL_FUNCTION(glClearColor, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
#define glClearColor USE_GL_FUNCTION(p_glClearColor)
DECLARE_GL_FUNCTION(glClearStencil, void, (GLint s))
#define glClearStencil USE_GL_FUNCTION(p_glClearStencil)
DECLARE_GL_FUNCTION(glClearDepth, void, (GLdouble depth))
#define glClearDepth USE_GL_FUNCTION(p_glClearDepth)
DECLARE_GL_FUNCTION(glStencilMask, void, (GLuint mask))
#define glStencilMask USE_GL_FUNCTION(p_glStencilMask)
DECLARE_GL_FUNCTION(glColorMask, void, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))
#define glColorMask USE_GL_FUNCTION(p_glColorMask)
DECLARE_GL_FUNCTION(glDepthMask, void, (GLboolean flag))
#define glDepthMask USE_GL_FUNCTION(p_glDepthMask)
DECLARE_GL_FUNCTION(glDisable, void, (GLenum cap))
#define glDisable USE_GL_FUNCTION(p_glDisable)
DECLARE_GL_FUNCTION(glEnable, void, (GLenum cap))
#define glEnable USE_GL_FUNCTION(p_glEnable)
DECLARE_GL_FUNCTION(glFinish, void, (void))
#define glFinish USE_GL_FUNCTION(p_glFinish)
DECLARE_GL_FUNCTION(glFlush, void, (void))
#define glFlush USE_GL_FUNCTION(p_glFlush)
DECLARE_GL_FUNCTION(glBlendFunc, void, (GLenum sfactor, GLenum dfactor))
#define glBlendFunc USE_GL_FUNCTION(p_glBlendFunc)
DECLARE_GL_FUNCTION(glLogicOp, void, (GLenum opcode))
#define glLogicOp USE_GL_FUNCTION(p_glLogicOp)
DECLARE_GL_FUNCTION(glStencilFunc, void, (GLenum func, GLint ref, GLuint mask))
#define glStencilFunc USE_GL_FUNCTION(p_glStencilFunc)
DECLARE_GL_FUNCTION(glStencilOp, void, (GLenum fail, GLenum zfail, GLenum zpass))
#define glStencilOp USE_GL_FUNCTION(p_glStencilOp)
DECLARE_GL_FUNCTION(glDepthFunc, void, (GLenum func))
#define glDepthFunc USE_GL_FUNCTION(p_glDepthFunc)
DECLARE_GL_FUNCTION(glPixelStoref, void, (GLenum pname, GLfloat param))
#define glPixelStoref USE_GL_FUNCTION(p_glPixelStoref)
DECLARE_GL_FUNCTION(glPixelStorei, void, (GLenum pname, GLint param))
#define glPixelStorei USE_GL_FUNCTION(p_glPixelStorei)
DECLARE_GL_FUNCTION(glReadBuffer, void, (GLenum src))
#define glReadBuffer USE_GL_FUNCTION(p_glReadBuffer)
DECLARE_GL_FUNCTION(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels))
#define glReadPixels USE_GL_FUNCTION(p_glReadPixels)
DECLARE_GL_FUNCTION(glGetBooleanv, void, (GLenum pname, GLboolean *data))
#define glGetBooleanv USE_GL_FUNCTION(p_glGetBooleanv)
DECLARE_GL_FUNCTION(glGetDoublev, void, (GLenum pname, GLdouble *data))
#define glGetDoublev USE_GL_FUNCTION(p_glGetDoublev)
DECLARE_GL_FUNCTION(glGetError, GLenum, (void))
#define glGetError USE_GL_FUNCTION(p_glGetError)
DECLARE_GL_FUNCTION(glGetFloatv, void, (GLenum pname, GLfloat *data))
#define glGetFloatv USE_GL_FUNCTION(p_glGetFloatv)
DECLARE_GL_FUNCTION(glGetIntegerv, void, (GLenum pname, GLint *data))
#define glGetIntegerv USE_GL_FUNCTION(p_glGetIntegerv)
DECLARE_GL_FUNCTION(glGetString, const GLubyte *, (GLenum name))
#define glGetString USE_GL_FUNCTION(p_glGetString)
DECLARE_GL_FUNCTION(glGetTexImage, void, (GLenum target, GLint level, GLenum format, GLenum type, void *pixels))
#define glGetTexImage USE_GL_FUNCTION(p_glGetTexImage)
DECLARE_GL_FUNCTION(glGetTexParameterfv, void, (GLenum target, GLenum pname, GLfloat *params))
#define glGetTexParameterfv USE_GL_FUNCTION(p_glGetTexParameterfv)
DECLARE_GL_FUNCTION(glGetTexParameteriv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetTexParameteriv USE_GL_FUNCTION(p_glGetTexParameteriv)
DECLARE_GL_FUNCTION(glGetTexLevelParameterfv, void, (GLenum target, GLint level, GLenum pname, GLfloat *params))
#define glGetTexLevelParameterfv USE_GL_FUNCTION(p_glGetTexLevelParameterfv)
DECLARE_GL_FUNCTION(glGetTexLevelParameteriv, void, (GLenum target, GLint level, GLenum pname, GLint *params))
#define glGetTexLevelParameteriv USE_GL_FUNCTION(p_glGetTexLevelParameteriv)
DECLARE_GL_FUNCTION(glIsEnabled, GLboolean, (GLenum cap))
#define glIsEnabled USE_GL_FUNCTION(p_glIsEnabled)
DECLARE_GL_FUNCTION(glDepthRange, void, (GLdouble n, GLdouble f))
#define glDepthRange USE_GL_FUNCTION(p_glDepthRange)
DECLARE_GL_FUNCTION(glViewport, void, (GLint x, GLint y, GLsizei width, GLsizei height))
#define glViewport USE_GL_FUNCTION(p_glViewport)
#endif /* GL_VERSION_1_0 */

#ifndef GL_VERSION_1_1
#define GL_VERSION_1_1 1
typedef float GLclampf;
typedef double GLclampd;
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_POINT           0x2A01
#define GL_POLYGON_OFFSET_LINE            0x2A02
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_1D             0x8068
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_TEXTURE_INTERNAL_FORMAT        0x1003
#define GL_TEXTURE_RED_SIZE               0x805C
#define GL_TEXTURE_GREEN_SIZE             0x805D
#define GL_TEXTURE_BLUE_SIZE              0x805E
#define GL_TEXTURE_ALPHA_SIZE             0x805F
#define GL_DOUBLE                         0x140A
#define GL_PROXY_TEXTURE_1D               0x8063
#define GL_PROXY_TEXTURE_2D               0x8064
#define GL_R3_G3_B2                       0x2A10
#define GL_RGB4                           0x804F
#define GL_RGB5                           0x8050
#define GL_RGB8                           0x8051
#define GL_RGB10                          0x8052
#define GL_RGB12                          0x8053
#define GL_RGB16                          0x8054
#define GL_RGBA2                          0x8055
#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGBA8                          0x8058
#define GL_RGB10_A2                       0x8059
#define GL_RGBA12                         0x805A
#define GL_RGBA16                         0x805B
#define GL_VERTEX_ARRAY                   0x8074
DECLARE_GL_FUNCTION(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))
#define glDrawArrays USE_GL_FUNCTION(p_glDrawArrays)
DECLARE_GL_FUNCTION(glDrawElements, void, (GLenum mode, GLsizei count, GLenum type, const void *indices))
#define glDrawElements USE_GL_FUNCTION(p_glDrawElements)
DECLARE_GL_FUNCTION(glGetPointerv, void, (GLenum pname, void **params))
#define glGetPointerv USE_GL_FUNCTION(p_glGetPointerv)
DECLARE_GL_FUNCTION(glPolygonOffset, void, (GLfloat factor, GLfloat units))
#define glPolygonOffset USE_GL_FUNCTION(p_glPolygonOffset)
DECLARE_GL_FUNCTION(glCopyTexImage1D, void, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border))
#define glCopyTexImage1D USE_GL_FUNCTION(p_glCopyTexImage1D)
DECLARE_GL_FUNCTION(glCopyTexImage2D, void, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border))
#define glCopyTexImage2D USE_GL_FUNCTION(p_glCopyTexImage2D)
DECLARE_GL_FUNCTION(glCopyTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width))
#define glCopyTexSubImage1D USE_GL_FUNCTION(p_glCopyTexSubImage1D)
DECLARE_GL_FUNCTION(glCopyTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height))
#define glCopyTexSubImage2D USE_GL_FUNCTION(p_glCopyTexSubImage2D)
DECLARE_GL_FUNCTION(glTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels))
#define glTexSubImage1D USE_GL_FUNCTION(p_glTexSubImage1D)
DECLARE_GL_FUNCTION(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
#define glTexSubImage2D USE_GL_FUNCTION(p_glTexSubImage2D)
DECLARE_GL_FUNCTION(glBindTexture, void, (GLenum target, GLuint texture))
#define glBindTexture USE_GL_FUNCTION(p_glBindTexture)
DECLARE_GL_FUNCTION(glDeleteTextures, void, (GLsizei n, const GLuint *textures))
#define glDeleteTextures USE_GL_FUNCTION(p_glDeleteTextures)
DECLARE_GL_FUNCTION(glGenTextures, void, (GLsizei n, GLuint *textures))
#define glGenTextures USE_GL_FUNCTION(p_glGenTextures)
DECLARE_GL_FUNCTION(glIsTexture, GLboolean, (GLuint texture))
#define glIsTexture USE_GL_FUNCTION(p_glIsTexture)
#endif /* GL_VERSION_1_1 */

#ifndef GL_VERSION_1_2
#define GL_VERSION_1_2 1
#define GL_UNSIGNED_BYTE_3_3_2            0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_INT_8_8_8_8           0x8035
#define GL_UNSIGNED_INT_10_10_10_2        0x8036
#define GL_TEXTURE_BINDING_3D             0x806A
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
#define GL_UNSIGNED_SHORT_5_6_5           0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
#define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
#define GL_BGR                            0x80E0
#define GL_BGRA                           0x80E1
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
DECLARE_GL_FUNCTION(glDrawRangeElements, void, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices))
#define glDrawRangeElements USE_GL_FUNCTION(p_glDrawRangeElements)
DECLARE_GL_FUNCTION(glTexImage3D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels))
#define glTexImage3D USE_GL_FUNCTION(p_glTexImage3D)
DECLARE_GL_FUNCTION(glTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels))
#define glTexSubImage3D USE_GL_FUNCTION(p_glTexSubImage3D)
DECLARE_GL_FUNCTION(glCopyTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))
#define glCopyTexSubImage3D USE_GL_FUNCTION(p_glCopyTexSubImage3D)
#endif /* GL_VERSION_1_2 */

#ifndef GL_VERSION_1_3
#define GL_VERSION_1_3 1
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
#define GL_COMPRESSED_RGB                 0x84ED
#define GL_COMPRESSED_RGBA                0x84EE
#define GL_TEXTURE_COMPRESSION_HINT       0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
#define GL_TEXTURE_COMPRESSED             0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
#define GL_CLAMP_TO_BORDER                0x812D
DECLARE_GL_FUNCTION(glActiveTexture, void, (GLenum texture))
#define glActiveTexture USE_GL_FUNCTION(p_glActiveTexture)
DECLARE_GL_FUNCTION(glSampleCoverage, void, (GLfloat value, GLboolean invert))
#define glSampleCoverage USE_GL_FUNCTION(p_glSampleCoverage)
DECLARE_GL_FUNCTION(glCompressedTexImage3D, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data))
#define glCompressedTexImage3D USE_GL_FUNCTION(p_glCompressedTexImage3D)
DECLARE_GL_FUNCTION(glCompressedTexImage2D, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data))
#define glCompressedTexImage2D USE_GL_FUNCTION(p_glCompressedTexImage2D)
DECLARE_GL_FUNCTION(glCompressedTexImage1D, void, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data))
#define glCompressedTexImage1D USE_GL_FUNCTION(p_glCompressedTexImage1D)
DECLARE_GL_FUNCTION(glCompressedTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTexSubImage3D USE_GL_FUNCTION(p_glCompressedTexSubImage3D)
DECLARE_GL_FUNCTION(glCompressedTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTexSubImage2D USE_GL_FUNCTION(p_glCompressedTexSubImage2D)
DECLARE_GL_FUNCTION(glCompressedTexSubImage1D, void, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTexSubImage1D USE_GL_FUNCTION(p_glCompressedTexSubImage1D)
DECLARE_GL_FUNCTION(glGetCompressedTexImage, void, (GLenum target, GLint level, void *img))
#define glGetCompressedTexImage USE_GL_FUNCTION(p_glGetCompressedTexImage)
#endif /* GL_VERSION_1_3 */

#ifndef GL_VERSION_1_4
#define GL_VERSION_1_4 1
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_MIRRORED_REPEAT                0x8370
#define GL_MAX_TEXTURE_LOD_BIAS           0x84FD
#define GL_TEXTURE_LOD_BIAS               0x8501
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508
#define GL_TEXTURE_DEPTH_SIZE             0x884A
#define GL_TEXTURE_COMPARE_MODE           0x884C
#define GL_TEXTURE_COMPARE_FUNC           0x884D
#define GL_BLEND_COLOR                    0x8005
#define GL_BLEND_EQUATION                 0x8009
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_FUNC_ADD                       0x8006
#define GL_FUNC_REVERSE_SUBTRACT          0x800B
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_MIN                            0x8007
#define GL_MAX                            0x8008
DECLARE_GL_FUNCTION(glBlendFuncSeparate, void, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))
#define glBlendFuncSeparate USE_GL_FUNCTION(p_glBlendFuncSeparate)
DECLARE_GL_FUNCTION(glMultiDrawArrays, void, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount))
#define glMultiDrawArrays USE_GL_FUNCTION(p_glMultiDrawArrays)
DECLARE_GL_FUNCTION(glMultiDrawElements, void, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount))
#define glMultiDrawElements USE_GL_FUNCTION(p_glMultiDrawElements)
DECLARE_GL_FUNCTION(glPointParameterf, void, (GLenum pname, GLfloat param))
#define glPointParameterf USE_GL_FUNCTION(p_glPointParameterf)
DECLARE_GL_FUNCTION(glPointParameterfv, void, (GLenum pname, const GLfloat *params))
#define glPointParameterfv USE_GL_FUNCTION(p_glPointParameterfv)
DECLARE_GL_FUNCTION(glPointParameteri, void, (GLenum pname, GLint param))
#define glPointParameteri USE_GL_FUNCTION(p_glPointParameteri)
DECLARE_GL_FUNCTION(glPointParameteriv, void, (GLenum pname, const GLint *params))
#define glPointParameteriv USE_GL_FUNCTION(p_glPointParameteriv)
DECLARE_GL_FUNCTION(glBlendColor, void, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))
#define glBlendColor USE_GL_FUNCTION(p_glBlendColor)
DECLARE_GL_FUNCTION(glBlendEquation, void, (GLenum mode))
#define glBlendEquation USE_GL_FUNCTION(p_glBlendEquation)
#endif /* GL_VERSION_1_4 */

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1
typedef intptr_t GLsizeiptr;
typedef intptr_t GLintptr;
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#define GL_SRC1_ALPHA                     0x8589
DECLARE_GL_FUNCTION(glGenQueries, void, (GLsizei n, GLuint *ids))
#define glGenQueries USE_GL_FUNCTION(p_glGenQueries)
DECLARE_GL_FUNCTION(glDeleteQueries, void, (GLsizei n, const GLuint *ids))
#define glDeleteQueries USE_GL_FUNCTION(p_glDeleteQueries)
DECLARE_GL_FUNCTION(glIsQuery, GLboolean, (GLuint id))
#define glIsQuery USE_GL_FUNCTION(p_glIsQuery)
DECLARE_GL_FUNCTION(glBeginQuery, void, (GLenum target, GLuint id))
#define glBeginQuery USE_GL_FUNCTION(p_glBeginQuery)
DECLARE_GL_FUNCTION(glEndQuery, void, (GLenum target))
#define glEndQuery USE_GL_FUNCTION(p_glEndQuery)
DECLARE_GL_FUNCTION(glGetQueryiv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetQueryiv USE_GL_FUNCTION(p_glGetQueryiv)
DECLARE_GL_FUNCTION(glGetQueryObjectiv, void, (GLuint id, GLenum pname, GLint *params))
#define glGetQueryObjectiv USE_GL_FUNCTION(p_glGetQueryObjectiv)
DECLARE_GL_FUNCTION(glGetQueryObjectuiv, void, (GLuint id, GLenum pname, GLuint *params))
#define glGetQueryObjectuiv USE_GL_FUNCTION(p_glGetQueryObjectuiv)
DECLARE_GL_FUNCTION(glBindBuffer, void, (GLenum target, GLuint buffer))
#define glBindBuffer USE_GL_FUNCTION(p_glBindBuffer)
DECLARE_GL_FUNCTION(glDeleteBuffers, void, (GLsizei n, const GLuint *buffers))
#define glDeleteBuffers USE_GL_FUNCTION(p_glDeleteBuffers)
DECLARE_GL_FUNCTION(glGenBuffers, void, (GLsizei n, GLuint *buffers))
#define glGenBuffers USE_GL_FUNCTION(p_glGenBuffers)
DECLARE_GL_FUNCTION(glIsBuffer, GLboolean, (GLuint buffer))
#define glIsBuffer USE_GL_FUNCTION(p_glIsBuffer)
DECLARE_GL_FUNCTION(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
#define glBufferData USE_GL_FUNCTION(p_glBufferData)
DECLARE_GL_FUNCTION(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data))
#define glBufferSubData USE_GL_FUNCTION(p_glBufferSubData)
DECLARE_GL_FUNCTION(glGetBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, void *data))
#define glGetBufferSubData USE_GL_FUNCTION(p_glGetBufferSubData)
DECLARE_GL_FUNCTION(glMapBuffer, void *, (GLenum target, GLenum access))
#define glMapBuffer USE_GL_FUNCTION(p_glMapBuffer)
DECLARE_GL_FUNCTION(glUnmapBuffer, GLboolean, (GLenum target))
#define glUnmapBuffer USE_GL_FUNCTION(p_glUnmapBuffer)
DECLARE_GL_FUNCTION(glGetBufferParameteriv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetBufferParameteriv USE_GL_FUNCTION(p_glGetBufferParameteriv)
DECLARE_GL_FUNCTION(glGetBufferPointerv, void, (GLenum target, GLenum pname, void **params))
#define glGetBufferPointerv USE_GL_FUNCTION(p_glGetBufferPointerv)
#endif /* GL_VERSION_1_5 */

#ifndef GL_VERSION_2_0
#define GL_VERSION_2_0 1
typedef char GLchar;
typedef int16_t GLshort;
typedef int8_t GLbyte;
typedef uint16_t GLushort;
#define GL_BLEND_EQUATION_RGB             0x8009
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED    0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE       0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE     0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE       0x8625
#define GL_CURRENT_VERTEX_ATTRIB          0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE      0x8642
#define GL_VERTEX_ATTRIB_ARRAY_POINTER    0x8645
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_MAX_DRAW_BUFFERS               0x8824
#define GL_DRAW_BUFFER0                   0x8825
#define GL_DRAW_BUFFER1                   0x8826
#define GL_DRAW_BUFFER2                   0x8827
#define GL_DRAW_BUFFER3                   0x8828
#define GL_DRAW_BUFFER4                   0x8829
#define GL_DRAW_BUFFER5                   0x882A
#define GL_DRAW_BUFFER6                   0x882B
#define GL_DRAW_BUFFER7                   0x882C
#define GL_DRAW_BUFFER8                   0x882D
#define GL_DRAW_BUFFER9                   0x882E
#define GL_DRAW_BUFFER10                  0x882F
#define GL_DRAW_BUFFER11                  0x8830
#define GL_DRAW_BUFFER12                  0x8831
#define GL_DRAW_BUFFER13                  0x8832
#define GL_DRAW_BUFFER14                  0x8833
#define GL_DRAW_BUFFER15                  0x8834
#define GL_BLEND_EQUATION_ALPHA           0x883D
#define GL_MAX_VERTEX_ATTRIBS             0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_IMAGE_UNITS        0x8872
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS  0x8B4A
#define GL_MAX_VARYING_FLOATS             0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE                    0x8B4F
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_1D                     0x8B5D
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_3D                     0x8B5F
#define GL_SAMPLER_CUBE                   0x8B60
#define GL_SAMPLER_1D_SHADOW              0x8B61
#define GL_SAMPLER_2D_SHADOW              0x8B62
#define GL_DELETE_STATUS                  0x8B80
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_ATTACHED_SHADERS               0x8B85
#define GL_ACTIVE_UNIFORMS                0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH      0x8B87
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_ACTIVE_ATTRIBUTES              0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH    0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_CURRENT_PROGRAM                0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN      0x8CA0
#define GL_LOWER_LEFT                     0x8CA1
#define GL_UPPER_LEFT                     0x8CA2
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
DECLARE_GL_FUNCTION(glBlendEquationSeparate, void, (GLenum modeRGB, GLenum modeAlpha))
#define glBlendEquationSeparate USE_GL_FUNCTION(p_glBlendEquationSeparate)
DECLARE_GL_FUNCTION(glDrawBuffers, void, (GLsizei n, const GLenum *bufs))
#define glDrawBuffers USE_GL_FUNCTION(p_glDrawBuffers)
DECLARE_GL_FUNCTION(glStencilOpSeparate, void, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass))
#define glStencilOpSeparate USE_GL_FUNCTION(p_glStencilOpSeparate)
DECLARE_GL_FUNCTION(glStencilFuncSeparate, void, (GLenum face, GLenum func, GLint ref, GLuint mask))
#define glStencilFuncSeparate USE_GL_FUNCTION(p_glStencilFuncSeparate)
DECLARE_GL_FUNCTION(glStencilMaskSeparate, void, (GLenum face, GLuint mask))
#define glStencilMaskSeparate USE_GL_FUNCTION(p_glStencilMaskSeparate)
DECLARE_GL_FUNCTION(glAttachShader, void, (GLuint program, GLuint shader))
#define glAttachShader USE_GL_FUNCTION(p_glAttachShader)
DECLARE_GL_FUNCTION(glBindAttribLocation, void, (GLuint program, GLuint index, const GLchar *name))
#define glBindAttribLocation USE_GL_FUNCTION(p_glBindAttribLocation)
DECLARE_GL_FUNCTION(glCompileShader, void, (GLuint shader))
#define glCompileShader USE_GL_FUNCTION(p_glCompileShader)
DECLARE_GL_FUNCTION(glCreateProgram, GLuint, (void))
#define glCreateProgram USE_GL_FUNCTION(p_glCreateProgram)
DECLARE_GL_FUNCTION(glCreateShader, GLuint, (GLenum type))
#define glCreateShader USE_GL_FUNCTION(p_glCreateShader)
DECLARE_GL_FUNCTION(glDeleteProgram, void, (GLuint program))
#define glDeleteProgram USE_GL_FUNCTION(p_glDeleteProgram)
DECLARE_GL_FUNCTION(glDeleteShader, void, (GLuint shader))
#define glDeleteShader USE_GL_FUNCTION(p_glDeleteShader)
DECLARE_GL_FUNCTION(glDetachShader, void, (GLuint program, GLuint shader))
#define glDetachShader USE_GL_FUNCTION(p_glDetachShader)
DECLARE_GL_FUNCTION(glDisableVertexAttribArray, void, (GLuint index))
#define glDisableVertexAttribArray USE_GL_FUNCTION(p_glDisableVertexAttribArray)
DECLARE_GL_FUNCTION(glEnableVertexAttribArray, void, (GLuint index))
#define glEnableVertexAttribArray USE_GL_FUNCTION(p_glEnableVertexAttribArray)
DECLARE_GL_FUNCTION(glGetActiveAttrib, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name))
#define glGetActiveAttrib USE_GL_FUNCTION(p_glGetActiveAttrib)
DECLARE_GL_FUNCTION(glGetActiveUniform, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name))
#define glGetActiveUniform USE_GL_FUNCTION(p_glGetActiveUniform)
DECLARE_GL_FUNCTION(glGetAttachedShaders, void, (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders))
#define glGetAttachedShaders USE_GL_FUNCTION(p_glGetAttachedShaders)
DECLARE_GL_FUNCTION(glGetAttribLocation, GLint, (GLuint program, const GLchar *name))
#define glGetAttribLocation USE_GL_FUNCTION(p_glGetAttribLocation)
DECLARE_GL_FUNCTION(glGetProgramiv, void, (GLuint program, GLenum pname, GLint *params))
#define glGetProgramiv USE_GL_FUNCTION(p_glGetProgramiv)
DECLARE_GL_FUNCTION(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
#define glGetProgramInfoLog USE_GL_FUNCTION(p_glGetProgramInfoLog)
DECLARE_GL_FUNCTION(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params))
#define glGetShaderiv USE_GL_FUNCTION(p_glGetShaderiv)
DECLARE_GL_FUNCTION(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
#define glGetShaderInfoLog USE_GL_FUNCTION(p_glGetShaderInfoLog)
DECLARE_GL_FUNCTION(glGetShaderSource, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source))
#define glGetShaderSource USE_GL_FUNCTION(p_glGetShaderSource)
DECLARE_GL_FUNCTION(glGetUniformLocation, GLint, (GLuint program, const GLchar *name))
#define glGetUniformLocation USE_GL_FUNCTION(p_glGetUniformLocation)
DECLARE_GL_FUNCTION(glGetUniformfv, void, (GLuint program, GLint location, GLfloat *params))
#define glGetUniformfv USE_GL_FUNCTION(p_glGetUniformfv)
DECLARE_GL_FUNCTION(glGetUniformiv, void, (GLuint program, GLint location, GLint *params))
#define glGetUniformiv USE_GL_FUNCTION(p_glGetUniformiv)
DECLARE_GL_FUNCTION(glGetVertexAttribdv, void, (GLuint index, GLenum pname, GLdouble *params))
#define glGetVertexAttribdv USE_GL_FUNCTION(p_glGetVertexAttribdv)
DECLARE_GL_FUNCTION(glGetVertexAttribfv, void, (GLuint index, GLenum pname, GLfloat *params))
#define glGetVertexAttribfv USE_GL_FUNCTION(p_glGetVertexAttribfv)
DECLARE_GL_FUNCTION(glGetVertexAttribiv, void, (GLuint index, GLenum pname, GLint *params))
#define glGetVertexAttribiv USE_GL_FUNCTION(p_glGetVertexAttribiv)
DECLARE_GL_FUNCTION(glGetVertexAttribPointerv, void, (GLuint index, GLenum pname, void **pointer))
#define glGetVertexAttribPointerv USE_GL_FUNCTION(p_glGetVertexAttribPointerv)
DECLARE_GL_FUNCTION(glIsProgram, GLboolean, (GLuint program))
#define glIsProgram USE_GL_FUNCTION(p_glIsProgram)
DECLARE_GL_FUNCTION(glIsShader, GLboolean, (GLuint shader))
#define glIsShader USE_GL_FUNCTION(p_glIsShader)
DECLARE_GL_FUNCTION(glLinkProgram, void, (GLuint program))
#define glLinkProgram USE_GL_FUNCTION(p_glLinkProgram)
DECLARE_GL_FUNCTION(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length))
#define glShaderSource USE_GL_FUNCTION(p_glShaderSource)
DECLARE_GL_FUNCTION(glUseProgram, void, (GLuint program))
#define glUseProgram USE_GL_FUNCTION(p_glUseProgram)
DECLARE_GL_FUNCTION(glUniform1f, void, (GLint location, GLfloat v0))
#define glUniform1f USE_GL_FUNCTION(p_glUniform1f)
DECLARE_GL_FUNCTION(glUniform2f, void, (GLint location, GLfloat v0, GLfloat v1))
#define glUniform2f USE_GL_FUNCTION(p_glUniform2f)
DECLARE_GL_FUNCTION(glUniform3f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2))
#define glUniform3f USE_GL_FUNCTION(p_glUniform3f)
DECLARE_GL_FUNCTION(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
#define glUniform4f USE_GL_FUNCTION(p_glUniform4f)
DECLARE_GL_FUNCTION(glUniform1i, void, (GLint location, GLint v0))
#define glUniform1i USE_GL_FUNCTION(p_glUniform1i)
DECLARE_GL_FUNCTION(glUniform2i, void, (GLint location, GLint v0, GLint v1))
#define glUniform2i USE_GL_FUNCTION(p_glUniform2i)
DECLARE_GL_FUNCTION(glUniform3i, void, (GLint location, GLint v0, GLint v1, GLint v2))
#define glUniform3i USE_GL_FUNCTION(p_glUniform3i)
DECLARE_GL_FUNCTION(glUniform4i, void, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3))
#define glUniform4i USE_GL_FUNCTION(p_glUniform4i)
DECLARE_GL_FUNCTION(glUniform1fv, void, (GLint location, GLsizei count, const GLfloat *value))
#define glUniform1fv USE_GL_FUNCTION(p_glUniform1fv)
DECLARE_GL_FUNCTION(glUniform2fv, void, (GLint location, GLsizei count, const GLfloat *value))
#define glUniform2fv USE_GL_FUNCTION(p_glUniform2fv)
DECLARE_GL_FUNCTION(glUniform3fv, void, (GLint location, GLsizei count, const GLfloat *value))
#define glUniform3fv USE_GL_FUNCTION(p_glUniform3fv)
DECLARE_GL_FUNCTION(glUniform4fv, void, (GLint location, GLsizei count, const GLfloat *value))
#define glUniform4fv USE_GL_FUNCTION(p_glUniform4fv)
DECLARE_GL_FUNCTION(glUniform1iv, void, (GLint location, GLsizei count, const GLint *value))
#define glUniform1iv USE_GL_FUNCTION(p_glUniform1iv)
DECLARE_GL_FUNCTION(glUniform2iv, void, (GLint location, GLsizei count, const GLint *value))
#define glUniform2iv USE_GL_FUNCTION(p_glUniform2iv)
DECLARE_GL_FUNCTION(glUniform3iv, void, (GLint location, GLsizei count, const GLint *value))
#define glUniform3iv USE_GL_FUNCTION(p_glUniform3iv)
DECLARE_GL_FUNCTION(glUniform4iv, void, (GLint location, GLsizei count, const GLint *value))
#define glUniform4iv USE_GL_FUNCTION(p_glUniform4iv)
DECLARE_GL_FUNCTION(glUniformMatrix2fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix2fv USE_GL_FUNCTION(p_glUniformMatrix2fv)
DECLARE_GL_FUNCTION(glUniformMatrix3fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix3fv USE_GL_FUNCTION(p_glUniformMatrix3fv)
DECLARE_GL_FUNCTION(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix4fv USE_GL_FUNCTION(p_glUniformMatrix4fv)
DECLARE_GL_FUNCTION(glValidateProgram, void, (GLuint program))
#define glValidateProgram USE_GL_FUNCTION(p_glValidateProgram)
DECLARE_GL_FUNCTION(glVertexAttrib1d, void, (GLuint index, GLdouble x))
#define glVertexAttrib1d USE_GL_FUNCTION(p_glVertexAttrib1d)
DECLARE_GL_FUNCTION(glVertexAttrib1dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttrib1dv USE_GL_FUNCTION(p_glVertexAttrib1dv)
DECLARE_GL_FUNCTION(glVertexAttrib1f, void, (GLuint index, GLfloat x))
#define glVertexAttrib1f USE_GL_FUNCTION(p_glVertexAttrib1f)
DECLARE_GL_FUNCTION(glVertexAttrib1fv, void, (GLuint index, const GLfloat *v))
#define glVertexAttrib1fv USE_GL_FUNCTION(p_glVertexAttrib1fv)
DECLARE_GL_FUNCTION(glVertexAttrib1s, void, (GLuint index, GLshort x))
#define glVertexAttrib1s USE_GL_FUNCTION(p_glVertexAttrib1s)
DECLARE_GL_FUNCTION(glVertexAttrib1sv, void, (GLuint index, const GLshort *v))
#define glVertexAttrib1sv USE_GL_FUNCTION(p_glVertexAttrib1sv)
DECLARE_GL_FUNCTION(glVertexAttrib2d, void, (GLuint index, GLdouble x, GLdouble y))
#define glVertexAttrib2d USE_GL_FUNCTION(p_glVertexAttrib2d)
DECLARE_GL_FUNCTION(glVertexAttrib2dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttrib2dv USE_GL_FUNCTION(p_glVertexAttrib2dv)
DECLARE_GL_FUNCTION(glVertexAttrib2f, void, (GLuint index, GLfloat x, GLfloat y))
#define glVertexAttrib2f USE_GL_FUNCTION(p_glVertexAttrib2f)
DECLARE_GL_FUNCTION(glVertexAttrib2fv, void, (GLuint index, const GLfloat *v))
#define glVertexAttrib2fv USE_GL_FUNCTION(p_glVertexAttrib2fv)
DECLARE_GL_FUNCTION(glVertexAttrib2s, void, (GLuint index, GLshort x, GLshort y))
#define glVertexAttrib2s USE_GL_FUNCTION(p_glVertexAttrib2s)
DECLARE_GL_FUNCTION(glVertexAttrib2sv, void, (GLuint index, const GLshort *v))
#define glVertexAttrib2sv USE_GL_FUNCTION(p_glVertexAttrib2sv)
DECLARE_GL_FUNCTION(glVertexAttrib3d, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z))
#define glVertexAttrib3d USE_GL_FUNCTION(p_glVertexAttrib3d)
DECLARE_GL_FUNCTION(glVertexAttrib3dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttrib3dv USE_GL_FUNCTION(p_glVertexAttrib3dv)
DECLARE_GL_FUNCTION(glVertexAttrib3f, void, (GLuint index, GLfloat x, GLfloat y, GLfloat z))
#define glVertexAttrib3f USE_GL_FUNCTION(p_glVertexAttrib3f)
DECLARE_GL_FUNCTION(glVertexAttrib3fv, void, (GLuint index, const GLfloat *v))
#define glVertexAttrib3fv USE_GL_FUNCTION(p_glVertexAttrib3fv)
DECLARE_GL_FUNCTION(glVertexAttrib3s, void, (GLuint index, GLshort x, GLshort y, GLshort z))
#define glVertexAttrib3s USE_GL_FUNCTION(p_glVertexAttrib3s)
DECLARE_GL_FUNCTION(glVertexAttrib3sv, void, (GLuint index, const GLshort *v))
#define glVertexAttrib3sv USE_GL_FUNCTION(p_glVertexAttrib3sv)
DECLARE_GL_FUNCTION(glVertexAttrib4Nbv, void, (GLuint index, const GLbyte *v))
#define glVertexAttrib4Nbv USE_GL_FUNCTION(p_glVertexAttrib4Nbv)
DECLARE_GL_FUNCTION(glVertexAttrib4Niv, void, (GLuint index, const GLint *v))
#define glVertexAttrib4Niv USE_GL_FUNCTION(p_glVertexAttrib4Niv)
DECLARE_GL_FUNCTION(glVertexAttrib4Nsv, void, (GLuint index, const GLshort *v))
#define glVertexAttrib4Nsv USE_GL_FUNCTION(p_glVertexAttrib4Nsv)
DECLARE_GL_FUNCTION(glVertexAttrib4Nub, void, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w))
#define glVertexAttrib4Nub USE_GL_FUNCTION(p_glVertexAttrib4Nub)
DECLARE_GL_FUNCTION(glVertexAttrib4Nubv, void, (GLuint index, const GLubyte *v))
#define glVertexAttrib4Nubv USE_GL_FUNCTION(p_glVertexAttrib4Nubv)
DECLARE_GL_FUNCTION(glVertexAttrib4Nuiv, void, (GLuint index, const GLuint *v))
#define glVertexAttrib4Nuiv USE_GL_FUNCTION(p_glVertexAttrib4Nuiv)
DECLARE_GL_FUNCTION(glVertexAttrib4Nusv, void, (GLuint index, const GLushort *v))
#define glVertexAttrib4Nusv USE_GL_FUNCTION(p_glVertexAttrib4Nusv)
DECLARE_GL_FUNCTION(glVertexAttrib4bv, void, (GLuint index, const GLbyte *v))
#define glVertexAttrib4bv USE_GL_FUNCTION(p_glVertexAttrib4bv)
DECLARE_GL_FUNCTION(glVertexAttrib4d, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
#define glVertexAttrib4d USE_GL_FUNCTION(p_glVertexAttrib4d)
DECLARE_GL_FUNCTION(glVertexAttrib4dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttrib4dv USE_GL_FUNCTION(p_glVertexAttrib4dv)
DECLARE_GL_FUNCTION(glVertexAttrib4f, void, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w))
#define glVertexAttrib4f USE_GL_FUNCTION(p_glVertexAttrib4f)
DECLARE_GL_FUNCTION(glVertexAttrib4fv, void, (GLuint index, const GLfloat *v))
#define glVertexAttrib4fv USE_GL_FUNCTION(p_glVertexAttrib4fv)
DECLARE_GL_FUNCTION(glVertexAttrib4iv, void, (GLuint index, const GLint *v))
#define glVertexAttrib4iv USE_GL_FUNCTION(p_glVertexAttrib4iv)
DECLARE_GL_FUNCTION(glVertexAttrib4s, void, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w))
#define glVertexAttrib4s USE_GL_FUNCTION(p_glVertexAttrib4s)
DECLARE_GL_FUNCTION(glVertexAttrib4sv, void, (GLuint index, const GLshort *v))
#define glVertexAttrib4sv USE_GL_FUNCTION(p_glVertexAttrib4sv)
DECLARE_GL_FUNCTION(glVertexAttrib4ubv, void, (GLuint index, const GLubyte *v))
#define glVertexAttrib4ubv USE_GL_FUNCTION(p_glVertexAttrib4ubv)
DECLARE_GL_FUNCTION(glVertexAttrib4uiv, void, (GLuint index, const GLuint *v))
#define glVertexAttrib4uiv USE_GL_FUNCTION(p_glVertexAttrib4uiv)
DECLARE_GL_FUNCTION(glVertexAttrib4usv, void, (GLuint index, const GLushort *v))
#define glVertexAttrib4usv USE_GL_FUNCTION(p_glVertexAttrib4usv)
DECLARE_GL_FUNCTION(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))
#define glVertexAttribPointer USE_GL_FUNCTION(p_glVertexAttribPointer)
#endif /* GL_VERSION_2_0 */

#ifndef GL_VERSION_2_1
#define GL_VERSION_2_1 1
#define GL_PIXEL_PACK_BUFFER              0x88EB
#define GL_PIXEL_UNPACK_BUFFER            0x88EC
#define GL_PIXEL_PACK_BUFFER_BINDING      0x88ED
#define GL_PIXEL_UNPACK_BUFFER_BINDING    0x88EF
#define GL_FLOAT_MAT2x3                   0x8B65
#define GL_FLOAT_MAT2x4                   0x8B66
#define GL_FLOAT_MAT3x2                   0x8B67
#define GL_FLOAT_MAT3x4                   0x8B68
#define GL_FLOAT_MAT4x2                   0x8B69
#define GL_FLOAT_MAT4x3                   0x8B6A
#define GL_SRGB                           0x8C40
#define GL_SRGB8                          0x8C41
#define GL_SRGB_ALPHA                     0x8C42
#define GL_SRGB8_ALPHA8                   0x8C43
#define GL_COMPRESSED_SRGB                0x8C48
#define GL_COMPRESSED_SRGB_ALPHA          0x8C49
DECLARE_GL_FUNCTION(glUniformMatrix2x3fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix2x3fv USE_GL_FUNCTION(p_glUniformMatrix2x3fv)
DECLARE_GL_FUNCTION(glUniformMatrix3x2fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix3x2fv USE_GL_FUNCTION(p_glUniformMatrix3x2fv)
DECLARE_GL_FUNCTION(glUniformMatrix2x4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix2x4fv USE_GL_FUNCTION(p_glUniformMatrix2x4fv)
DECLARE_GL_FUNCTION(glUniformMatrix4x2fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix4x2fv USE_GL_FUNCTION(p_glUniformMatrix4x2fv)
DECLARE_GL_FUNCTION(glUniformMatrix3x4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix3x4fv USE_GL_FUNCTION(p_glUniformMatrix3x4fv)
DECLARE_GL_FUNCTION(glUniformMatrix4x3fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glUniformMatrix4x3fv USE_GL_FUNCTION(p_glUniformMatrix4x3fv)
#endif /* GL_VERSION_2_1 */

#ifndef GL_VERSION_3_0
#define GL_VERSION_3_0 1
typedef uint16_t GLhalf;
#define GL_COMPARE_REF_TO_TEXTURE         0x884E
#define GL_CLIP_DISTANCE0                 0x3000
#define GL_CLIP_DISTANCE1                 0x3001
#define GL_CLIP_DISTANCE2                 0x3002
#define GL_CLIP_DISTANCE3                 0x3003
#define GL_CLIP_DISTANCE4                 0x3004
#define GL_CLIP_DISTANCE5                 0x3005
#define GL_CLIP_DISTANCE6                 0x3006
#define GL_CLIP_DISTANCE7                 0x3007
#define GL_MAX_CLIP_DISTANCES             0x0D32
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_NUM_EXTENSIONS                 0x821D
#define GL_CONTEXT_FLAGS                  0x821E
#define GL_COMPRESSED_RED                 0x8225
#define GL_COMPRESSED_RG                  0x8226
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x00000001
#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_VERTEX_ATTRIB_ARRAY_INTEGER    0x88FD
#define GL_MAX_ARRAY_TEXTURE_LAYERS       0x88FF
#define GL_MIN_PROGRAM_TEXEL_OFFSET       0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET       0x8905
#define GL_CLAMP_READ_COLOR               0x891C
#define GL_FIXED_ONLY                     0x891D
#define GL_MAX_VARYING_COMPONENTS         0x8B4B
#define GL_TEXTURE_1D_ARRAY               0x8C18
#define GL_PROXY_TEXTURE_1D_ARRAY         0x8C19
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_PROXY_TEXTURE_2D_ARRAY         0x8C1B
#define GL_TEXTURE_BINDING_1D_ARRAY       0x8C1C
#define GL_TEXTURE_BINDING_2D_ARRAY       0x8C1D
#define GL_R11F_G11F_B10F                 0x8C3A
#define GL_UNSIGNED_INT_10F_11F_11F_REV   0x8C3B
#define GL_RGB9_E5                        0x8C3D
#define GL_UNSIGNED_INT_5_9_9_9_REV       0x8C3E
#define GL_TEXTURE_SHARED_SIZE            0x8C3F
#define GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH 0x8C76
#define GL_TRANSFORM_FEEDBACK_BUFFER_MODE 0x8C7F
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS 0x8C80
#define GL_TRANSFORM_FEEDBACK_VARYINGS    0x8C83
#define GL_TRANSFORM_FEEDBACK_BUFFER_START 0x8C84
#define GL_TRANSFORM_FEEDBACK_BUFFER_SIZE 0x8C85
#define GL_PRIMITIVES_GENERATED           0x8C87
#define GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN 0x8C88
#define GL_RASTERIZER_DISCARD             0x8C89
#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS 0x8C8B
#define GL_INTERLEAVED_ATTRIBS            0x8C8C
#define GL_SEPARATE_ATTRIBS               0x8C8D
#define GL_TRANSFORM_FEEDBACK_BUFFER      0x8C8E
#define GL_TRANSFORM_FEEDBACK_BUFFER_BINDING 0x8C8F
#define GL_RGBA32UI                       0x8D70
#define GL_RGB32UI                        0x8D71
#define GL_RGBA16UI                       0x8D76
#define GL_RGB16UI                        0x8D77
#define GL_RGBA8UI                        0x8D7C
#define GL_RGB8UI                         0x8D7D
#define GL_RGBA32I                        0x8D82
#define GL_RGB32I                         0x8D83
#define GL_RGBA16I                        0x8D88
#define GL_RGB16I                         0x8D89
#define GL_RGBA8I                         0x8D8E
#define GL_RGB8I                          0x8D8F
#define GL_RED_INTEGER                    0x8D94
#define GL_GREEN_INTEGER                  0x8D95
#define GL_BLUE_INTEGER                   0x8D96
#define GL_RGB_INTEGER                    0x8D98
#define GL_RGBA_INTEGER                   0x8D99
#define GL_BGR_INTEGER                    0x8D9A
#define GL_BGRA_INTEGER                   0x8D9B
#define GL_SAMPLER_1D_ARRAY               0x8DC0
#define GL_SAMPLER_2D_ARRAY               0x8DC1
#define GL_SAMPLER_1D_ARRAY_SHADOW        0x8DC3
#define GL_SAMPLER_2D_ARRAY_SHADOW        0x8DC4
#define GL_SAMPLER_CUBE_SHADOW            0x8DC5
#define GL_UNSIGNED_INT_VEC2              0x8DC6
#define GL_UNSIGNED_INT_VEC3              0x8DC7
#define GL_UNSIGNED_INT_VEC4              0x8DC8
#define GL_INT_SAMPLER_1D                 0x8DC9
#define GL_INT_SAMPLER_2D                 0x8DCA
#define GL_INT_SAMPLER_3D                 0x8DCB
#define GL_INT_SAMPLER_CUBE               0x8DCC
#define GL_INT_SAMPLER_1D_ARRAY           0x8DCE
#define GL_INT_SAMPLER_2D_ARRAY           0x8DCF
#define GL_UNSIGNED_INT_SAMPLER_1D        0x8DD1
#define GL_UNSIGNED_INT_SAMPLER_2D        0x8DD2
#define GL_UNSIGNED_INT_SAMPLER_3D        0x8DD3
#define GL_UNSIGNED_INT_SAMPLER_CUBE      0x8DD4
#define GL_UNSIGNED_INT_SAMPLER_1D_ARRAY  0x8DD6
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY  0x8DD7
#define GL_QUERY_WAIT                     0x8E13
#define GL_QUERY_NO_WAIT                  0x8E14
#define GL_QUERY_BY_REGION_WAIT           0x8E15
#define GL_QUERY_BY_REGION_NO_WAIT        0x8E16
#define GL_BUFFER_ACCESS_FLAGS            0x911F
#define GL_BUFFER_MAP_LENGTH              0x9120
#define GL_BUFFER_MAP_OFFSET              0x9121
#define GL_DEPTH_COMPONENT32F             0x8CAC
#define GL_DEPTH32F_STENCIL8              0x8CAD
#define GL_FLOAT_32_UNSIGNED_INT_24_8_REV 0x8DAD
#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING 0x8210
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE 0x8211
#define GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE 0x8212
#define GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE 0x8213
#define GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE 0x8214
#define GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE 0x8215
#define GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE 0x8216
#define GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE 0x8217
#define GL_FRAMEBUFFER_DEFAULT            0x8218
#define GL_FRAMEBUFFER_UNDEFINED          0x8219
#define GL_DEPTH_STENCIL_ATTACHMENT       0x821A
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8
#define GL_DEPTH_STENCIL                  0x84F9
#define GL_UNSIGNED_INT_24_8              0x84FA
#define GL_DEPTH24_STENCIL8               0x88F0
#define GL_TEXTURE_STENCIL_SIZE           0x88F1
#define GL_TEXTURE_RED_TYPE               0x8C10
#define GL_TEXTURE_GREEN_TYPE             0x8C11
#define GL_TEXTURE_BLUE_TYPE              0x8C12
#define GL_TEXTURE_ALPHA_TYPE             0x8C13
#define GL_TEXTURE_DEPTH_TYPE             0x8C16
#define GL_UNSIGNED_NORMALIZED            0x8C17
#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_DRAW_FRAMEBUFFER_BINDING       0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_READ_FRAMEBUFFER_BINDING       0x8CAA
#define GL_RENDERBUFFER_SAMPLES           0x8CAB
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE 0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME 0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL 0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER 0x8CD4
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED        0x8CDD
#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41
#define GL_RENDERBUFFER_WIDTH             0x8D42
#define GL_RENDERBUFFER_HEIGHT            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GL_STENCIL_INDEX1                 0x8D46
#define GL_STENCIL_INDEX4                 0x8D47
#define GL_STENCIL_INDEX8                 0x8D48
#define GL_STENCIL_INDEX16                0x8D49
#define GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE 0x8D56
#define GL_MAX_SAMPLES                    0x8D57
#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_HALF_FLOAT                     0x140B
#define GL_MAP_READ_BIT                   0x0001
#define GL_MAP_WRITE_BIT                  0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT       0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT      0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT         0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT         0x0020
#define GL_COMPRESSED_RED_RGTC1           0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1    0x8DBC
#define GL_COMPRESSED_RG_RGTC2            0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2     0x8DBE
#define GL_RG                             0x8227
#define GL_RG_INTEGER                     0x8228
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_VERTEX_ARRAY_BINDING           0x85B5
DECLARE_GL_FUNCTION(glColorMaski, void, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a))
#define glColorMaski USE_GL_FUNCTION(p_glColorMaski)
DECLARE_GL_FUNCTION(glGetBooleani_v, void, (GLenum target, GLuint index, GLboolean *data))
#define glGetBooleani_v USE_GL_FUNCTION(p_glGetBooleani_v)
DECLARE_GL_FUNCTION(glGetIntegeri_v, void, (GLenum target, GLuint index, GLint *data))
#define glGetIntegeri_v USE_GL_FUNCTION(p_glGetIntegeri_v)
DECLARE_GL_FUNCTION(glEnablei, void, (GLenum target, GLuint index))
#define glEnablei USE_GL_FUNCTION(p_glEnablei)
DECLARE_GL_FUNCTION(glDisablei, void, (GLenum target, GLuint index))
#define glDisablei USE_GL_FUNCTION(p_glDisablei)
DECLARE_GL_FUNCTION(glIsEnabledi, GLboolean, (GLenum target, GLuint index))
#define glIsEnabledi USE_GL_FUNCTION(p_glIsEnabledi)
DECLARE_GL_FUNCTION(glBeginTransformFeedback, void, (GLenum primitiveMode))
#define glBeginTransformFeedback USE_GL_FUNCTION(p_glBeginTransformFeedback)
DECLARE_GL_FUNCTION(glEndTransformFeedback, void, (void))
#define glEndTransformFeedback USE_GL_FUNCTION(p_glEndTransformFeedback)
DECLARE_GL_FUNCTION(glBindBufferRange, void, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size))
#define glBindBufferRange USE_GL_FUNCTION(p_glBindBufferRange)
DECLARE_GL_FUNCTION(glBindBufferBase, void, (GLenum target, GLuint index, GLuint buffer))
#define glBindBufferBase USE_GL_FUNCTION(p_glBindBufferBase)
DECLARE_GL_FUNCTION(glTransformFeedbackVaryings, void, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode))
#define glTransformFeedbackVaryings USE_GL_FUNCTION(p_glTransformFeedbackVaryings)
DECLARE_GL_FUNCTION(glGetTransformFeedbackVarying, void, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name))
#define glGetTransformFeedbackVarying USE_GL_FUNCTION(p_glGetTransformFeedbackVarying)
DECLARE_GL_FUNCTION(glClampColor, void, (GLenum target, GLenum clamp))
#define glClampColor USE_GL_FUNCTION(p_glClampColor)
DECLARE_GL_FUNCTION(glBeginConditionalRender, void, (GLuint id, GLenum mode))
#define glBeginConditionalRender USE_GL_FUNCTION(p_glBeginConditionalRender)
DECLARE_GL_FUNCTION(glEndConditionalRender, void, (void))
#define glEndConditionalRender USE_GL_FUNCTION(p_glEndConditionalRender)
DECLARE_GL_FUNCTION(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))
#define glVertexAttribIPointer USE_GL_FUNCTION(p_glVertexAttribIPointer)
DECLARE_GL_FUNCTION(glGetVertexAttribIiv, void, (GLuint index, GLenum pname, GLint *params))
#define glGetVertexAttribIiv USE_GL_FUNCTION(p_glGetVertexAttribIiv)
DECLARE_GL_FUNCTION(glGetVertexAttribIuiv, void, (GLuint index, GLenum pname, GLuint *params))
#define glGetVertexAttribIuiv USE_GL_FUNCTION(p_glGetVertexAttribIuiv)
DECLARE_GL_FUNCTION(glVertexAttribI1i, void, (GLuint index, GLint x))
#define glVertexAttribI1i USE_GL_FUNCTION(p_glVertexAttribI1i)
DECLARE_GL_FUNCTION(glVertexAttribI2i, void, (GLuint index, GLint x, GLint y))
#define glVertexAttribI2i USE_GL_FUNCTION(p_glVertexAttribI2i)
DECLARE_GL_FUNCTION(glVertexAttribI3i, void, (GLuint index, GLint x, GLint y, GLint z))
#define glVertexAttribI3i USE_GL_FUNCTION(p_glVertexAttribI3i)
DECLARE_GL_FUNCTION(glVertexAttribI4i, void, (GLuint index, GLint x, GLint y, GLint z, GLint w))
#define glVertexAttribI4i USE_GL_FUNCTION(p_glVertexAttribI4i)
DECLARE_GL_FUNCTION(glVertexAttribI1ui, void, (GLuint index, GLuint x))
#define glVertexAttribI1ui USE_GL_FUNCTION(p_glVertexAttribI1ui)
DECLARE_GL_FUNCTION(glVertexAttribI2ui, void, (GLuint index, GLuint x, GLuint y))
#define glVertexAttribI2ui USE_GL_FUNCTION(p_glVertexAttribI2ui)
DECLARE_GL_FUNCTION(glVertexAttribI3ui, void, (GLuint index, GLuint x, GLuint y, GLuint z))
#define glVertexAttribI3ui USE_GL_FUNCTION(p_glVertexAttribI3ui)
DECLARE_GL_FUNCTION(glVertexAttribI4ui, void, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w))
#define glVertexAttribI4ui USE_GL_FUNCTION(p_glVertexAttribI4ui)
DECLARE_GL_FUNCTION(glVertexAttribI1iv, void, (GLuint index, const GLint *v))
#define glVertexAttribI1iv USE_GL_FUNCTION(p_glVertexAttribI1iv)
DECLARE_GL_FUNCTION(glVertexAttribI2iv, void, (GLuint index, const GLint *v))
#define glVertexAttribI2iv USE_GL_FUNCTION(p_glVertexAttribI2iv)
DECLARE_GL_FUNCTION(glVertexAttribI3iv, void, (GLuint index, const GLint *v))
#define glVertexAttribI3iv USE_GL_FUNCTION(p_glVertexAttribI3iv)
DECLARE_GL_FUNCTION(glVertexAttribI4iv, void, (GLuint index, const GLint *v))
#define glVertexAttribI4iv USE_GL_FUNCTION(p_glVertexAttribI4iv)
DECLARE_GL_FUNCTION(glVertexAttribI1uiv, void, (GLuint index, const GLuint *v))
#define glVertexAttribI1uiv USE_GL_FUNCTION(p_glVertexAttribI1uiv)
DECLARE_GL_FUNCTION(glVertexAttribI2uiv, void, (GLuint index, const GLuint *v))
#define glVertexAttribI2uiv USE_GL_FUNCTION(p_glVertexAttribI2uiv)
DECLARE_GL_FUNCTION(glVertexAttribI3uiv, void, (GLuint index, const GLuint *v))
#define glVertexAttribI3uiv USE_GL_FUNCTION(p_glVertexAttribI3uiv)
DECLARE_GL_FUNCTION(glVertexAttribI4uiv, void, (GLuint index, const GLuint *v))
#define glVertexAttribI4uiv USE_GL_FUNCTION(p_glVertexAttribI4uiv)
DECLARE_GL_FUNCTION(glVertexAttribI4bv, void, (GLuint index, const GLbyte *v))
#define glVertexAttribI4bv USE_GL_FUNCTION(p_glVertexAttribI4bv)
DECLARE_GL_FUNCTION(glVertexAttribI4sv, void, (GLuint index, const GLshort *v))
#define glVertexAttribI4sv USE_GL_FUNCTION(p_glVertexAttribI4sv)
DECLARE_GL_FUNCTION(glVertexAttribI4ubv, void, (GLuint index, const GLubyte *v))
#define glVertexAttribI4ubv USE_GL_FUNCTION(p_glVertexAttribI4ubv)
DECLARE_GL_FUNCTION(glVertexAttribI4usv, void, (GLuint index, const GLushort *v))
#define glVertexAttribI4usv USE_GL_FUNCTION(p_glVertexAttribI4usv)
DECLARE_GL_FUNCTION(glGetUniformuiv, void, (GLuint program, GLint location, GLuint *params))
#define glGetUniformuiv USE_GL_FUNCTION(p_glGetUniformuiv)
DECLARE_GL_FUNCTION(glBindFragDataLocation, void, (GLuint program, GLuint color, const GLchar *name))
#define glBindFragDataLocation USE_GL_FUNCTION(p_glBindFragDataLocation)
DECLARE_GL_FUNCTION(glGetFragDataLocation, GLint, (GLuint program, const GLchar *name))
#define glGetFragDataLocation USE_GL_FUNCTION(p_glGetFragDataLocation)
DECLARE_GL_FUNCTION(glUniform1ui, void, (GLint location, GLuint v0))
#define glUniform1ui USE_GL_FUNCTION(p_glUniform1ui)
DECLARE_GL_FUNCTION(glUniform2ui, void, (GLint location, GLuint v0, GLuint v1))
#define glUniform2ui USE_GL_FUNCTION(p_glUniform2ui)
DECLARE_GL_FUNCTION(glUniform3ui, void, (GLint location, GLuint v0, GLuint v1, GLuint v2))
#define glUniform3ui USE_GL_FUNCTION(p_glUniform3ui)
DECLARE_GL_FUNCTION(glUniform4ui, void, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3))
#define glUniform4ui USE_GL_FUNCTION(p_glUniform4ui)
DECLARE_GL_FUNCTION(glUniform1uiv, void, (GLint location, GLsizei count, const GLuint *value))
#define glUniform1uiv USE_GL_FUNCTION(p_glUniform1uiv)
DECLARE_GL_FUNCTION(glUniform2uiv, void, (GLint location, GLsizei count, const GLuint *value))
#define glUniform2uiv USE_GL_FUNCTION(p_glUniform2uiv)
DECLARE_GL_FUNCTION(glUniform3uiv, void, (GLint location, GLsizei count, const GLuint *value))
#define glUniform3uiv USE_GL_FUNCTION(p_glUniform3uiv)
DECLARE_GL_FUNCTION(glUniform4uiv, void, (GLint location, GLsizei count, const GLuint *value))
#define glUniform4uiv USE_GL_FUNCTION(p_glUniform4uiv)
DECLARE_GL_FUNCTION(glTexParameterIiv, void, (GLenum target, GLenum pname, const GLint *params))
#define glTexParameterIiv USE_GL_FUNCTION(p_glTexParameterIiv)
DECLARE_GL_FUNCTION(glTexParameterIuiv, void, (GLenum target, GLenum pname, const GLuint *params))
#define glTexParameterIuiv USE_GL_FUNCTION(p_glTexParameterIuiv)
DECLARE_GL_FUNCTION(glGetTexParameterIiv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetTexParameterIiv USE_GL_FUNCTION(p_glGetTexParameterIiv)
DECLARE_GL_FUNCTION(glGetTexParameterIuiv, void, (GLenum target, GLenum pname, GLuint *params))
#define glGetTexParameterIuiv USE_GL_FUNCTION(p_glGetTexParameterIuiv)
DECLARE_GL_FUNCTION(glClearBufferiv, void, (GLenum buffer, GLint drawbuffer, const GLint *value))
#define glClearBufferiv USE_GL_FUNCTION(p_glClearBufferiv)
DECLARE_GL_FUNCTION(glClearBufferuiv, void, (GLenum buffer, GLint drawbuffer, const GLuint *value))
#define glClearBufferuiv USE_GL_FUNCTION(p_glClearBufferuiv)
DECLARE_GL_FUNCTION(glClearBufferfv, void, (GLenum buffer, GLint drawbuffer, const GLfloat *value))
#define glClearBufferfv USE_GL_FUNCTION(p_glClearBufferfv)
DECLARE_GL_FUNCTION(glClearBufferfi, void, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil))
#define glClearBufferfi USE_GL_FUNCTION(p_glClearBufferfi)
DECLARE_GL_FUNCTION(glGetStringi, const GLubyte *, (GLenum name, GLuint index))
#define glGetStringi USE_GL_FUNCTION(p_glGetStringi)
DECLARE_GL_FUNCTION(glIsRenderbuffer, GLboolean, (GLuint renderbuffer))
#define glIsRenderbuffer USE_GL_FUNCTION(p_glIsRenderbuffer)
DECLARE_GL_FUNCTION(glBindRenderbuffer, void, (GLenum target, GLuint renderbuffer))
#define glBindRenderbuffer USE_GL_FUNCTION(p_glBindRenderbuffer)
DECLARE_GL_FUNCTION(glDeleteRenderbuffers, void, (GLsizei n, const GLuint *renderbuffers))
#define glDeleteRenderbuffers USE_GL_FUNCTION(p_glDeleteRenderbuffers)
DECLARE_GL_FUNCTION(glGenRenderbuffers, void, (GLsizei n, GLuint *renderbuffers))
#define glGenRenderbuffers USE_GL_FUNCTION(p_glGenRenderbuffers)
DECLARE_GL_FUNCTION(glRenderbufferStorage, void, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height))
#define glRenderbufferStorage USE_GL_FUNCTION(p_glRenderbufferStorage)
DECLARE_GL_FUNCTION(glGetRenderbufferParameteriv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetRenderbufferParameteriv USE_GL_FUNCTION(p_glGetRenderbufferParameteriv)
DECLARE_GL_FUNCTION(glIsFramebuffer, GLboolean, (GLuint framebuffer))
#define glIsFramebuffer USE_GL_FUNCTION(p_glIsFramebuffer)
DECLARE_GL_FUNCTION(glBindFramebuffer, void, (GLenum target, GLuint framebuffer))
#define glBindFramebuffer USE_GL_FUNCTION(p_glBindFramebuffer)
DECLARE_GL_FUNCTION(glDeleteFramebuffers, void, (GLsizei n, const GLuint *framebuffers))
#define glDeleteFramebuffers USE_GL_FUNCTION(p_glDeleteFramebuffers)
DECLARE_GL_FUNCTION(glGenFramebuffers, void, (GLsizei n, GLuint *framebuffers))
#define glGenFramebuffers USE_GL_FUNCTION(p_glGenFramebuffers)
DECLARE_GL_FUNCTION(glCheckFramebufferStatus, GLenum, (GLenum target))
#define glCheckFramebufferStatus USE_GL_FUNCTION(p_glCheckFramebufferStatus)
DECLARE_GL_FUNCTION(glFramebufferTexture1D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
#define glFramebufferTexture1D USE_GL_FUNCTION(p_glFramebufferTexture1D)
DECLARE_GL_FUNCTION(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
#define glFramebufferTexture2D USE_GL_FUNCTION(p_glFramebufferTexture2D)
DECLARE_GL_FUNCTION(glFramebufferTexture3D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))
#define glFramebufferTexture3D USE_GL_FUNCTION(p_glFramebufferTexture3D)
DECLARE_GL_FUNCTION(glFramebufferRenderbuffer, void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))
#define glFramebufferRenderbuffer USE_GL_FUNCTION(p_glFramebufferRenderbuffer)
DECLARE_GL_FUNCTION(glGetFramebufferAttachmentParameteriv, void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))
#define glGetFramebufferAttachmentParameteriv USE_GL_FUNCTION(p_glGetFramebufferAttachmentParameteriv)
DECLARE_GL_FUNCTION(glGenerateMipmap, void, (GLenum target))
#define glGenerateMipmap USE_GL_FUNCTION(p_glGenerateMipmap)
DECLARE_GL_FUNCTION(glBlitFramebuffer, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))
#define glBlitFramebuffer USE_GL_FUNCTION(p_glBlitFramebuffer)
DECLARE_GL_FUNCTION(glRenderbufferStorageMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))
#define glRenderbufferStorageMultisample USE_GL_FUNCTION(p_glRenderbufferStorageMultisample)
DECLARE_GL_FUNCTION(glFramebufferTextureLayer, void, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer))
#define glFramebufferTextureLayer USE_GL_FUNCTION(p_glFramebufferTextureLayer)
DECLARE_GL_FUNCTION(glMapBufferRange, void *, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access))
#define glMapBufferRange USE_GL_FUNCTION(p_glMapBufferRange)
DECLARE_GL_FUNCTION(glFlushMappedBufferRange, void, (GLenum target, GLintptr offset, GLsizeiptr length))
#define glFlushMappedBufferRange USE_GL_FUNCTION(p_glFlushMappedBufferRange)
DECLARE_GL_FUNCTION(glBindVertexArray, void, (GLuint array))
#define glBindVertexArray USE_GL_FUNCTION(p_glBindVertexArray)
DECLARE_GL_FUNCTION(glDeleteVertexArrays, void, (GLsizei n, const GLuint *arrays))
#define glDeleteVertexArrays USE_GL_FUNCTION(p_glDeleteVertexArrays)
DECLARE_GL_FUNCTION(glGenVertexArrays, void, (GLsizei n, GLuint *arrays))
#define glGenVertexArrays USE_GL_FUNCTION(p_glGenVertexArrays)
DECLARE_GL_FUNCTION(glIsVertexArray, GLboolean, (GLuint array))
#define glIsVertexArray USE_GL_FUNCTION(p_glIsVertexArray)
#endif /* GL_VERSION_3_0 */

#ifndef GL_VERSION_3_1
#define GL_VERSION_3_1 1
#define GL_SAMPLER_2D_RECT                0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW         0x8B64
#define GL_SAMPLER_BUFFER                 0x8DC2
#define GL_INT_SAMPLER_2D_RECT            0x8DCD
#define GL_INT_SAMPLER_BUFFER             0x8DD0
#define GL_UNSIGNED_INT_SAMPLER_2D_RECT   0x8DD5
#define GL_UNSIGNED_INT_SAMPLER_BUFFER    0x8DD8
#define GL_TEXTURE_BUFFER                 0x8C2A
#define GL_MAX_TEXTURE_BUFFER_SIZE        0x8C2B
#define GL_TEXTURE_BINDING_BUFFER         0x8C2C
#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING 0x8C2D
#define GL_TEXTURE_RECTANGLE              0x84F5
#define GL_TEXTURE_BINDING_RECTANGLE      0x84F6
#define GL_PROXY_TEXTURE_RECTANGLE        0x84F7
#define GL_MAX_RECTANGLE_TEXTURE_SIZE     0x84F8
#define GL_R8_SNORM                       0x8F94
#define GL_RG8_SNORM                      0x8F95
#define GL_RGB8_SNORM                     0x8F96
#define GL_RGBA8_SNORM                    0x8F97
#define GL_R16_SNORM                      0x8F98
#define GL_RG16_SNORM                     0x8F99
#define GL_RGB16_SNORM                    0x8F9A
#define GL_RGBA16_SNORM                   0x8F9B
#define GL_SIGNED_NORMALIZED              0x8F9C
#define GL_PRIMITIVE_RESTART              0x8F9D
#define GL_PRIMITIVE_RESTART_INDEX        0x8F9E
#define GL_COPY_READ_BUFFER               0x8F36
#define GL_COPY_WRITE_BUFFER              0x8F37
#define GL_UNIFORM_BUFFER                 0x8A11
#define GL_UNIFORM_BUFFER_BINDING         0x8A28
#define GL_UNIFORM_BUFFER_START           0x8A29
#define GL_UNIFORM_BUFFER_SIZE            0x8A2A
#define GL_MAX_VERTEX_UNIFORM_BLOCKS      0x8A2B
#define GL_MAX_GEOMETRY_UNIFORM_BLOCKS    0x8A2C
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS    0x8A2D
#define GL_MAX_COMBINED_UNIFORM_BLOCKS    0x8A2E
#define GL_MAX_UNIFORM_BUFFER_BINDINGS    0x8A2F
#define GL_MAX_UNIFORM_BLOCK_SIZE         0x8A30
#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS 0x8A31
#define GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS 0x8A32
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS 0x8A33
#define GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT 0x8A34
#define GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH 0x8A35
#define GL_ACTIVE_UNIFORM_BLOCKS          0x8A36
#define GL_UNIFORM_TYPE                   0x8A37
#define GL_UNIFORM_SIZE                   0x8A38
#define GL_UNIFORM_NAME_LENGTH            0x8A39
#define GL_UNIFORM_BLOCK_INDEX            0x8A3A
#define GL_UNIFORM_OFFSET                 0x8A3B
#define GL_UNIFORM_ARRAY_STRIDE           0x8A3C
#define GL_UNIFORM_MATRIX_STRIDE          0x8A3D
#define GL_UNIFORM_IS_ROW_MAJOR           0x8A3E
#define GL_UNIFORM_BLOCK_BINDING          0x8A3F
#define GL_UNIFORM_BLOCK_DATA_SIZE        0x8A40
#define GL_UNIFORM_BLOCK_NAME_LENGTH      0x8A41
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS  0x8A42
#define GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES 0x8A43
#define GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER 0x8A44
#define GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER 0x8A45
#define GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER 0x8A46
#define GL_INVALID_INDEX                  0xFFFFFFFFu
DECLARE_GL_FUNCTION(glDrawArraysInstanced, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount))
#define glDrawArraysInstanced USE_GL_FUNCTION(p_glDrawArraysInstanced)
DECLARE_GL_FUNCTION(glDrawElementsInstanced, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount))
#define glDrawElementsInstanced USE_GL_FUNCTION(p_glDrawElementsInstanced)
DECLARE_GL_FUNCTION(glTexBuffer, void, (GLenum target, GLenum internalformat, GLuint buffer))
#define glTexBuffer USE_GL_FUNCTION(p_glTexBuffer)
DECLARE_GL_FUNCTION(glPrimitiveRestartIndex, void, (GLuint index))
#define glPrimitiveRestartIndex USE_GL_FUNCTION(p_glPrimitiveRestartIndex)
DECLARE_GL_FUNCTION(glCopyBufferSubData, void, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size))
#define glCopyBufferSubData USE_GL_FUNCTION(p_glCopyBufferSubData)
DECLARE_GL_FUNCTION(glGetUniformIndices, void, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices))
#define glGetUniformIndices USE_GL_FUNCTION(p_glGetUniformIndices)
DECLARE_GL_FUNCTION(glGetActiveUniformsiv, void, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params))
#define glGetActiveUniformsiv USE_GL_FUNCTION(p_glGetActiveUniformsiv)
DECLARE_GL_FUNCTION(glGetActiveUniformName, void, (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName))
#define glGetActiveUniformName USE_GL_FUNCTION(p_glGetActiveUniformName)
DECLARE_GL_FUNCTION(glGetUniformBlockIndex, GLuint, (GLuint program, const GLchar *uniformBlockName))
#define glGetUniformBlockIndex USE_GL_FUNCTION(p_glGetUniformBlockIndex)
DECLARE_GL_FUNCTION(glGetActiveUniformBlockiv, void, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params))
#define glGetActiveUniformBlockiv USE_GL_FUNCTION(p_glGetActiveUniformBlockiv)
DECLARE_GL_FUNCTION(glGetActiveUniformBlockName, void, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName))
#define glGetActiveUniformBlockName USE_GL_FUNCTION(p_glGetActiveUniformBlockName)
DECLARE_GL_FUNCTION(glUniformBlockBinding, void, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding))
#define glUniformBlockBinding USE_GL_FUNCTION(p_glUniformBlockBinding)
#endif /* GL_VERSION_3_1 */

#ifndef GL_VERSION_3_2
#define GL_VERSION_3_2 1
typedef struct __GLsync *GLsync;
typedef uint64_t GLuint64;
typedef int64_t GLint64;
#define GL_CONTEXT_CORE_PROFILE_BIT       0x00000001
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#define GL_LINES_ADJACENCY                0x000A
#define GL_LINE_STRIP_ADJACENCY           0x000B
#define GL_TRIANGLES_ADJACENCY            0x000C
#define GL_TRIANGLE_STRIP_ADJACENCY       0x000D
#define GL_PROGRAM_POINT_SIZE             0x8642
#define GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS 0x8C29
#define GL_FRAMEBUFFER_ATTACHMENT_LAYERED 0x8DA7
#define GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS 0x8DA8
#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_GEOMETRY_VERTICES_OUT          0x8916
#define GL_GEOMETRY_INPUT_TYPE            0x8917
#define GL_GEOMETRY_OUTPUT_TYPE           0x8918
#define GL_MAX_GEOMETRY_UNIFORM_COMPONENTS 0x8DDF
#define GL_MAX_GEOMETRY_OUTPUT_VERTICES   0x8DE0
#define GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS 0x8DE1
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS   0x9122
#define GL_MAX_GEOMETRY_INPUT_COMPONENTS  0x9123
#define GL_MAX_GEOMETRY_OUTPUT_COMPONENTS 0x9124
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS  0x9125
#define GL_CONTEXT_PROFILE_MASK           0x9126
#define GL_DEPTH_CLAMP                    0x864F
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION        0x8E4D
#define GL_LAST_VERTEX_CONVENTION         0x8E4E
#define GL_PROVOKING_VERTEX               0x8E4F
#define GL_TEXTURE_CUBE_MAP_SEAMLESS      0x884F
#define GL_MAX_SERVER_WAIT_TIMEOUT        0x9111
#define GL_OBJECT_TYPE                    0x9112
#define GL_SYNC_CONDITION                 0x9113
#define GL_SYNC_STATUS                    0x9114
#define GL_SYNC_FLAGS                     0x9115
#define GL_SYNC_FENCE                     0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE     0x9117
#define GL_UNSIGNALED                     0x9118
#define GL_SIGNALED                       0x9119
#define GL_ALREADY_SIGNALED               0x911A
#define GL_TIMEOUT_EXPIRED                0x911B
#define GL_CONDITION_SATISFIED            0x911C
#define GL_WAIT_FAILED                    0x911D
#define GL_TIMEOUT_IGNORED                0xFFFFFFFFFFFFFFFFull
#define GL_SYNC_FLUSH_COMMANDS_BIT        0x00000001
#define GL_SAMPLE_POSITION                0x8E50
#define GL_SAMPLE_MASK                    0x8E51
#define GL_SAMPLE_MASK_VALUE              0x8E52
#define GL_MAX_SAMPLE_MASK_WORDS          0x8E59
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE   0x9101
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY   0x9102
#define GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY 0x9103
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE 0x9104
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY 0x9105
#define GL_TEXTURE_SAMPLES                0x9106
#define GL_TEXTURE_FIXED_SAMPLE_LOCATIONS 0x9107
#define GL_SAMPLER_2D_MULTISAMPLE         0x9108
#define GL_INT_SAMPLER_2D_MULTISAMPLE     0x9109
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY   0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 0x910D
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F
#define GL_MAX_INTEGER_SAMPLES            0x9110
DECLARE_GL_FUNCTION(glDrawElementsBaseVertex, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex))
#define glDrawElementsBaseVertex USE_GL_FUNCTION(p_glDrawElementsBaseVertex)
DECLARE_GL_FUNCTION(glDrawRangeElementsBaseVertex, void, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex))
#define glDrawRangeElementsBaseVertex USE_GL_FUNCTION(p_glDrawRangeElementsBaseVertex)
DECLARE_GL_FUNCTION(glDrawElementsInstancedBaseVertex, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex))
#define glDrawElementsInstancedBaseVertex USE_GL_FUNCTION(p_glDrawElementsInstancedBaseVertex)
DECLARE_GL_FUNCTION(glMultiDrawElementsBaseVertex, void, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex))
#define glMultiDrawElementsBaseVertex USE_GL_FUNCTION(p_glMultiDrawElementsBaseVertex)
DECLARE_GL_FUNCTION(glProvokingVertex, void, (GLenum mode))
#define glProvokingVertex USE_GL_FUNCTION(p_glProvokingVertex)
DECLARE_GL_FUNCTION(glFenceSync, GLsync, (GLenum condition, GLbitfield flags))
#define glFenceSync USE_GL_FUNCTION(p_glFenceSync)
DECLARE_GL_FUNCTION(glIsSync, GLboolean, (GLsync sync))
#define glIsSync USE_GL_FUNCTION(p_glIsSync)
DECLARE_GL_FUNCTION(glDeleteSync, void, (GLsync sync))
#define glDeleteSync USE_GL_FUNCTION(p_glDeleteSync)
DECLARE_GL_FUNCTION(glClientWaitSync, GLenum, (GLsync sync, GLbitfield flags, GLuint64 timeout))
#define glClientWaitSync USE_GL_FUNCTION(p_glClientWaitSync)
DECLARE_GL_FUNCTION(glWaitSync, void, (GLsync sync, GLbitfield flags, GLuint64 timeout))
#define glWaitSync USE_GL_FUNCTION(p_glWaitSync)
DECLARE_GL_FUNCTION(glGetInteger64v, void, (GLenum pname, GLint64 *data))
#define glGetInteger64v USE_GL_FUNCTION(p_glGetInteger64v)
DECLARE_GL_FUNCTION(glGetSynciv, void, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values))
#define glGetSynciv USE_GL_FUNCTION(p_glGetSynciv)
DECLARE_GL_FUNCTION(glGetInteger64i_v, void, (GLenum target, GLuint index, GLint64 *data))
#define glGetInteger64i_v USE_GL_FUNCTION(p_glGetInteger64i_v)
DECLARE_GL_FUNCTION(glGetBufferParameteri64v, void, (GLenum target, GLenum pname, GLint64 *params))
#define glGetBufferParameteri64v USE_GL_FUNCTION(p_glGetBufferParameteri64v)
DECLARE_GL_FUNCTION(glFramebufferTexture, void, (GLenum target, GLenum attachment, GLuint texture, GLint level))
#define glFramebufferTexture USE_GL_FUNCTION(p_glFramebufferTexture)
DECLARE_GL_FUNCTION(glTexImage2DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations))
#define glTexImage2DMultisample USE_GL_FUNCTION(p_glTexImage2DMultisample)
DECLARE_GL_FUNCTION(glTexImage3DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))
#define glTexImage3DMultisample USE_GL_FUNCTION(p_glTexImage3DMultisample)
DECLARE_GL_FUNCTION(glGetMultisamplefv, void, (GLenum pname, GLuint index, GLfloat *val))
#define glGetMultisamplefv USE_GL_FUNCTION(p_glGetMultisamplefv)
DECLARE_GL_FUNCTION(glSampleMaski, void, (GLuint maskNumber, GLbitfield mask))
#define glSampleMaski USE_GL_FUNCTION(p_glSampleMaski)
#endif /* GL_VERSION_3_2 */

#ifndef GL_VERSION_3_3
#define GL_VERSION_3_3 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR    0x88FE
#define GL_SRC1_COLOR                     0x88F9
#define GL_ONE_MINUS_SRC1_COLOR           0x88FA
#define GL_ONE_MINUS_SRC1_ALPHA           0x88FB
#define GL_MAX_DUAL_SOURCE_DRAW_BUFFERS   0x88FC
#define GL_ANY_SAMPLES_PASSED             0x8C2F
#define GL_SAMPLER_BINDING                0x8919
#define GL_RGB10_A2UI                     0x906F
#define GL_TEXTURE_SWIZZLE_R              0x8E42
#define GL_TEXTURE_SWIZZLE_G              0x8E43
#define GL_TEXTURE_SWIZZLE_B              0x8E44
#define GL_TEXTURE_SWIZZLE_A              0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46
#define GL_TIME_ELAPSED                   0x88BF
#define GL_TIMESTAMP                      0x8E28
#define GL_INT_2_10_10_10_REV             0x8D9F
DECLARE_GL_FUNCTION(glBindFragDataLocationIndexed, void, (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name))
#define glBindFragDataLocationIndexed USE_GL_FUNCTION(p_glBindFragDataLocationIndexed)
DECLARE_GL_FUNCTION(glGetFragDataIndex, GLint, (GLuint program, const GLchar *name))
#define glGetFragDataIndex USE_GL_FUNCTION(p_glGetFragDataIndex)
DECLARE_GL_FUNCTION(glGenSamplers, void, (GLsizei count, GLuint *samplers))
#define glGenSamplers USE_GL_FUNCTION(p_glGenSamplers)
DECLARE_GL_FUNCTION(glDeleteSamplers, void, (GLsizei count, const GLuint *samplers))
#define glDeleteSamplers USE_GL_FUNCTION(p_glDeleteSamplers)
DECLARE_GL_FUNCTION(glIsSampler, GLboolean, (GLuint sampler))
#define glIsSampler USE_GL_FUNCTION(p_glIsSampler)
DECLARE_GL_FUNCTION(glBindSampler, void, (GLuint unit, GLuint sampler))
#define glBindSampler USE_GL_FUNCTION(p_glBindSampler)
DECLARE_GL_FUNCTION(glSamplerParameteri, void, (GLuint sampler, GLenum pname, GLint param))
#define glSamplerParameteri USE_GL_FUNCTION(p_glSamplerParameteri)
DECLARE_GL_FUNCTION(glSamplerParameteriv, void, (GLuint sampler, GLenum pname, const GLint *param))
#define glSamplerParameteriv USE_GL_FUNCTION(p_glSamplerParameteriv)
DECLARE_GL_FUNCTION(glSamplerParameterf, void, (GLuint sampler, GLenum pname, GLfloat param))
#define glSamplerParameterf USE_GL_FUNCTION(p_glSamplerParameterf)
DECLARE_GL_FUNCTION(glSamplerParameterfv, void, (GLuint sampler, GLenum pname, const GLfloat *param))
#define glSamplerParameterfv USE_GL_FUNCTION(p_glSamplerParameterfv)
DECLARE_GL_FUNCTION(glSamplerParameterIiv, void, (GLuint sampler, GLenum pname, const GLint *param))
#define glSamplerParameterIiv USE_GL_FUNCTION(p_glSamplerParameterIiv)
DECLARE_GL_FUNCTION(glSamplerParameterIuiv, void, (GLuint sampler, GLenum pname, const GLuint *param))
#define glSamplerParameterIuiv USE_GL_FUNCTION(p_glSamplerParameterIuiv)
DECLARE_GL_FUNCTION(glGetSamplerParameteriv, void, (GLuint sampler, GLenum pname, GLint *params))
#define glGetSamplerParameteriv USE_GL_FUNCTION(p_glGetSamplerParameteriv)
DECLARE_GL_FUNCTION(glGetSamplerParameterIiv, void, (GLuint sampler, GLenum pname, GLint *params))
#define glGetSamplerParameterIiv USE_GL_FUNCTION(p_glGetSamplerParameterIiv)
DECLARE_GL_FUNCTION(glGetSamplerParameterfv, void, (GLuint sampler, GLenum pname, GLfloat *params))
#define glGetSamplerParameterfv USE_GL_FUNCTION(p_glGetSamplerParameterfv)
DECLARE_GL_FUNCTION(glGetSamplerParameterIuiv, void, (GLuint sampler, GLenum pname, GLuint *params))
#define glGetSamplerParameterIuiv USE_GL_FUNCTION(p_glGetSamplerParameterIuiv)
DECLARE_GL_FUNCTION(glQueryCounter, void, (GLuint id, GLenum target))
#define glQueryCounter USE_GL_FUNCTION(p_glQueryCounter)
DECLARE_GL_FUNCTION(glGetQueryObjecti64v, void, (GLuint id, GLenum pname, GLint64 *params))
#define glGetQueryObjecti64v USE_GL_FUNCTION(p_glGetQueryObjecti64v)
DECLARE_GL_FUNCTION(glGetQueryObjectui64v, void, (GLuint id, GLenum pname, GLuint64 *params))
#define glGetQueryObjectui64v USE_GL_FUNCTION(p_glGetQueryObjectui64v)
DECLARE_GL_FUNCTION(glVertexAttribDivisor, void, (GLuint index, GLuint divisor))
#define glVertexAttribDivisor USE_GL_FUNCTION(p_glVertexAttribDivisor)
DECLARE_GL_FUNCTION(glVertexAttribP1ui, void, (GLuint index, GLenum type, GLboolean normalized, GLuint value))
#define glVertexAttribP1ui USE_GL_FUNCTION(p_glVertexAttribP1ui)
DECLARE_GL_FUNCTION(glVertexAttribP1uiv, void, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value))
#define glVertexAttribP1uiv USE_GL_FUNCTION(p_glVertexAttribP1uiv)
DECLARE_GL_FUNCTION(glVertexAttribP2ui, void, (GLuint index, GLenum type, GLboolean normalized, GLuint value))
#define glVertexAttribP2ui USE_GL_FUNCTION(p_glVertexAttribP2ui)
DECLARE_GL_FUNCTION(glVertexAttribP2uiv, void, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value))
#define glVertexAttribP2uiv USE_GL_FUNCTION(p_glVertexAttribP2uiv)
DECLARE_GL_FUNCTION(glVertexAttribP3ui, void, (GLuint index, GLenum type, GLboolean normalized, GLuint value))
#define glVertexAttribP3ui USE_GL_FUNCTION(p_glVertexAttribP3ui)
DECLARE_GL_FUNCTION(glVertexAttribP3uiv, void, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value))
#define glVertexAttribP3uiv USE_GL_FUNCTION(p_glVertexAttribP3uiv)
DECLARE_GL_FUNCTION(glVertexAttribP4ui, void, (GLuint index, GLenum type, GLboolean normalized, GLuint value))
#define glVertexAttribP4ui USE_GL_FUNCTION(p_glVertexAttribP4ui)
DECLARE_GL_FUNCTION(glVertexAttribP4uiv, void, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value))
#define glVertexAttribP4uiv USE_GL_FUNCTION(p_glVertexAttribP4uiv)
#endif /* GL_VERSION_3_3 */

#ifndef GL_VERSION_4_0
#define GL_VERSION_4_0 1
#define GL_SAMPLE_SHADING                 0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE       0x8C37
#define GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5E
#define GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET 0x8E5F
#define GL_TEXTURE_CUBE_MAP_ARRAY         0x9009
#define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY 0x900A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARRAY   0x900B
#define GL_SAMPLER_CUBE_MAP_ARRAY         0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW  0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY     0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F
#define GL_DRAW_INDIRECT_BUFFER           0x8F3F
#define GL_DRAW_INDIRECT_BUFFER_BINDING   0x8F43
#define GL_GEOMETRY_SHADER_INVOCATIONS    0x887F
#define GL_MAX_GEOMETRY_SHADER_INVOCATIONS 0x8E5A
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS 0x8E5D
#define GL_MAX_VERTEX_STREAMS             0x8E71
#define GL_DOUBLE_VEC2                    0x8FFC
#define GL_DOUBLE_VEC3                    0x8FFD
#define GL_DOUBLE_VEC4                    0x8FFE
#define GL_DOUBLE_MAT2                    0x8F46
#define GL_DOUBLE_MAT3                    0x8F47
#define GL_DOUBLE_MAT4                    0x8F48
#define GL_DOUBLE_MAT2x3                  0x8F49
#define GL_DOUBLE_MAT2x4                  0x8F4A
#define GL_DOUBLE_MAT3x2                  0x8F4B
#define GL_DOUBLE_MAT3x4                  0x8F4C
#define GL_DOUBLE_MAT4x2                  0x8F4D
#define GL_DOUBLE_MAT4x3                  0x8F4E
#define GL_ACTIVE_SUBROUTINES             0x8DE5
#define GL_ACTIVE_SUBROUTINE_UNIFORMS     0x8DE6
#define GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS 0x8E47
#define GL_ACTIVE_SUBROUTINE_MAX_LENGTH   0x8E48
#define GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH 0x8E49
#define GL_MAX_SUBROUTINES                0x8DE7
#define GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS 0x8DE8
#define GL_NUM_COMPATIBLE_SUBROUTINES     0x8E4A
#define GL_COMPATIBLE_SUBROUTINES         0x8E4B
#define GL_PATCHES                        0x000E
#define GL_PATCH_VERTICES                 0x8E72
#define GL_PATCH_DEFAULT_INNER_LEVEL      0x8E73
#define GL_PATCH_DEFAULT_OUTER_LEVEL      0x8E74
#define GL_TESS_CONTROL_OUTPUT_VERTICES   0x8E75
#define GL_TESS_GEN_MODE                  0x8E76
#define GL_TESS_GEN_SPACING               0x8E77
#define GL_TESS_GEN_VERTEX_ORDER          0x8E78
#define GL_TESS_GEN_POINT_MODE            0x8E79
#define GL_ISOLINES                       0x8E7A
#define GL_FRACTIONAL_ODD                 0x8E7B
#define GL_FRACTIONAL_EVEN                0x8E7C
#define GL_MAX_PATCH_VERTICES             0x8E7D
#define GL_MAX_TESS_GEN_LEVEL             0x8E7E
#define GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E7F
#define GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E80
#define GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS 0x8E81
#define GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS 0x8E82
#define GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS 0x8E83
#define GL_MAX_TESS_PATCH_COMPONENTS      0x8E84
#define GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS 0x8E85
#define GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS 0x8E86
#define GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS 0x8E89
#define GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS 0x8E8A
#define GL_MAX_TESS_CONTROL_INPUT_COMPONENTS 0x886C
#define GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS 0x886D
#define GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS 0x8E1E
#define GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS 0x8E1F
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER 0x84F0
#define GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER 0x84F1
#define GL_TESS_EVALUATION_SHADER         0x8E87
#define GL_TESS_CONTROL_SHADER            0x8E88
#define GL_TRANSFORM_FEEDBACK             0x8E22
#define GL_TRANSFORM_FEEDBACK_BUFFER_PAUSED 0x8E23
#define GL_TRANSFORM_FEEDBACK_BUFFER_ACTIVE 0x8E24
#define GL_TRANSFORM_FEEDBACK_BINDING     0x8E25
#define GL_MAX_TRANSFORM_FEEDBACK_BUFFERS 0x8E70
DECLARE_GL_FUNCTION(glMinSampleShading, void, (GLfloat value))
#define glMinSampleShading USE_GL_FUNCTION(p_glMinSampleShading)
DECLARE_GL_FUNCTION(glBlendEquationi, void, (GLuint buf, GLenum mode))
#define glBlendEquationi USE_GL_FUNCTION(p_glBlendEquationi)
DECLARE_GL_FUNCTION(glBlendEquationSeparatei, void, (GLuint buf, GLenum modeRGB, GLenum modeAlpha))
#define glBlendEquationSeparatei USE_GL_FUNCTION(p_glBlendEquationSeparatei)
DECLARE_GL_FUNCTION(glBlendFunci, void, (GLuint buf, GLenum src, GLenum dst))
#define glBlendFunci USE_GL_FUNCTION(p_glBlendFunci)
DECLARE_GL_FUNCTION(glBlendFuncSeparatei, void, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))
#define glBlendFuncSeparatei USE_GL_FUNCTION(p_glBlendFuncSeparatei)
DECLARE_GL_FUNCTION(glDrawArraysIndirect, void, (GLenum mode, const void *indirect))
#define glDrawArraysIndirect USE_GL_FUNCTION(p_glDrawArraysIndirect)
DECLARE_GL_FUNCTION(glDrawElementsIndirect, void, (GLenum mode, GLenum type, const void *indirect))
#define glDrawElementsIndirect USE_GL_FUNCTION(p_glDrawElementsIndirect)
DECLARE_GL_FUNCTION(glUniform1d, void, (GLint location, GLdouble x))
#define glUniform1d USE_GL_FUNCTION(p_glUniform1d)
DECLARE_GL_FUNCTION(glUniform2d, void, (GLint location, GLdouble x, GLdouble y))
#define glUniform2d USE_GL_FUNCTION(p_glUniform2d)
DECLARE_GL_FUNCTION(glUniform3d, void, (GLint location, GLdouble x, GLdouble y, GLdouble z))
#define glUniform3d USE_GL_FUNCTION(p_glUniform3d)
DECLARE_GL_FUNCTION(glUniform4d, void, (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
#define glUniform4d USE_GL_FUNCTION(p_glUniform4d)
DECLARE_GL_FUNCTION(glUniform1dv, void, (GLint location, GLsizei count, const GLdouble *value))
#define glUniform1dv USE_GL_FUNCTION(p_glUniform1dv)
DECLARE_GL_FUNCTION(glUniform2dv, void, (GLint location, GLsizei count, const GLdouble *value))
#define glUniform2dv USE_GL_FUNCTION(p_glUniform2dv)
DECLARE_GL_FUNCTION(glUniform3dv, void, (GLint location, GLsizei count, const GLdouble *value))
#define glUniform3dv USE_GL_FUNCTION(p_glUniform3dv)
DECLARE_GL_FUNCTION(glUniform4dv, void, (GLint location, GLsizei count, const GLdouble *value))
#define glUniform4dv USE_GL_FUNCTION(p_glUniform4dv)
DECLARE_GL_FUNCTION(glUniformMatrix2dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix2dv USE_GL_FUNCTION(p_glUniformMatrix2dv)
DECLARE_GL_FUNCTION(glUniformMatrix3dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix3dv USE_GL_FUNCTION(p_glUniformMatrix3dv)
DECLARE_GL_FUNCTION(glUniformMatrix4dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix4dv USE_GL_FUNCTION(p_glUniformMatrix4dv)
DECLARE_GL_FUNCTION(glUniformMatrix2x3dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix2x3dv USE_GL_FUNCTION(p_glUniformMatrix2x3dv)
DECLARE_GL_FUNCTION(glUniformMatrix2x4dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix2x4dv USE_GL_FUNCTION(p_glUniformMatrix2x4dv)
DECLARE_GL_FUNCTION(glUniformMatrix3x2dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix3x2dv USE_GL_FUNCTION(p_glUniformMatrix3x2dv)
DECLARE_GL_FUNCTION(glUniformMatrix3x4dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix3x4dv USE_GL_FUNCTION(p_glUniformMatrix3x4dv)
DECLARE_GL_FUNCTION(glUniformMatrix4x2dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix4x2dv USE_GL_FUNCTION(p_glUniformMatrix4x2dv)
DECLARE_GL_FUNCTION(glUniformMatrix4x3dv, void, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glUniformMatrix4x3dv USE_GL_FUNCTION(p_glUniformMatrix4x3dv)
DECLARE_GL_FUNCTION(glGetUniformdv, void, (GLuint program, GLint location, GLdouble *params))
#define glGetUniformdv USE_GL_FUNCTION(p_glGetUniformdv)
DECLARE_GL_FUNCTION(glGetSubroutineUniformLocation, GLint, (GLuint program, GLenum shadertype, const GLchar *name))
#define glGetSubroutineUniformLocation USE_GL_FUNCTION(p_glGetSubroutineUniformLocation)
DECLARE_GL_FUNCTION(glGetSubroutineIndex, GLuint, (GLuint program, GLenum shadertype, const GLchar *name))
#define glGetSubroutineIndex USE_GL_FUNCTION(p_glGetSubroutineIndex)
DECLARE_GL_FUNCTION(glGetActiveSubroutineUniformiv, void, (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values))
#define glGetActiveSubroutineUniformiv USE_GL_FUNCTION(p_glGetActiveSubroutineUniformiv)
DECLARE_GL_FUNCTION(glGetActiveSubroutineUniformName, void, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name))
#define glGetActiveSubroutineUniformName USE_GL_FUNCTION(p_glGetActiveSubroutineUniformName)
DECLARE_GL_FUNCTION(glGetActiveSubroutineName, void, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name))
#define glGetActiveSubroutineName USE_GL_FUNCTION(p_glGetActiveSubroutineName)
DECLARE_GL_FUNCTION(glUniformSubroutinesuiv, void, (GLenum shadertype, GLsizei count, const GLuint *indices))
#define glUniformSubroutinesuiv USE_GL_FUNCTION(p_glUniformSubroutinesuiv)
DECLARE_GL_FUNCTION(glGetUniformSubroutineuiv, void, (GLenum shadertype, GLint location, GLuint *params))
#define glGetUniformSubroutineuiv USE_GL_FUNCTION(p_glGetUniformSubroutineuiv)
DECLARE_GL_FUNCTION(glGetProgramStageiv, void, (GLuint program, GLenum shadertype, GLenum pname, GLint *values))
#define glGetProgramStageiv USE_GL_FUNCTION(p_glGetProgramStageiv)
DECLARE_GL_FUNCTION(glPatchParameteri, void, (GLenum pname, GLint value))
#define glPatchParameteri USE_GL_FUNCTION(p_glPatchParameteri)
DECLARE_GL_FUNCTION(glPatchParameterfv, void, (GLenum pname, const GLfloat *values))
#define glPatchParameterfv USE_GL_FUNCTION(p_glPatchParameterfv)
DECLARE_GL_FUNCTION(glBindTransformFeedback, void, (GLenum target, GLuint id))
#define glBindTransformFeedback USE_GL_FUNCTION(p_glBindTransformFeedback)
DECLARE_GL_FUNCTION(glDeleteTransformFeedbacks, void, (GLsizei n, const GLuint *ids))
#define glDeleteTransformFeedbacks USE_GL_FUNCTION(p_glDeleteTransformFeedbacks)
DECLARE_GL_FUNCTION(glGenTransformFeedbacks, void, (GLsizei n, GLuint *ids))
#define glGenTransformFeedbacks USE_GL_FUNCTION(p_glGenTransformFeedbacks)
DECLARE_GL_FUNCTION(glIsTransformFeedback, GLboolean, (GLuint id))
#define glIsTransformFeedback USE_GL_FUNCTION(p_glIsTransformFeedback)
DECLARE_GL_FUNCTION(glPauseTransformFeedback, void, (void))
#define glPauseTransformFeedback USE_GL_FUNCTION(p_glPauseTransformFeedback)
DECLARE_GL_FUNCTION(glResumeTransformFeedback, void, (void))
#define glResumeTransformFeedback USE_GL_FUNCTION(p_glResumeTransformFeedback)
DECLARE_GL_FUNCTION(glDrawTransformFeedback, void, (GLenum mode, GLuint id))
#define glDrawTransformFeedback USE_GL_FUNCTION(p_glDrawTransformFeedback)
DECLARE_GL_FUNCTION(glDrawTransformFeedbackStream, void, (GLenum mode, GLuint id, GLuint stream))
#define glDrawTransformFeedbackStream USE_GL_FUNCTION(p_glDrawTransformFeedbackStream)
DECLARE_GL_FUNCTION(glBeginQueryIndexed, void, (GLenum target, GLuint index, GLuint id))
#define glBeginQueryIndexed USE_GL_FUNCTION(p_glBeginQueryIndexed)
DECLARE_GL_FUNCTION(glEndQueryIndexed, void, (GLenum target, GLuint index))
#define glEndQueryIndexed USE_GL_FUNCTION(p_glEndQueryIndexed)
DECLARE_GL_FUNCTION(glGetQueryIndexediv, void, (GLenum target, GLuint index, GLenum pname, GLint *params))
#define glGetQueryIndexediv USE_GL_FUNCTION(p_glGetQueryIndexediv)
#endif /* GL_VERSION_4_0 */

#ifndef GL_VERSION_4_1
#define GL_VERSION_4_1 1
#define GL_FIXED                          0x140C
#define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5
#define GL_SHADER_COMPILER                0x8DFA
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9
#define GL_MAX_VERTEX_UNIFORM_VECTORS     0x8DFB
#define GL_MAX_VARYING_VECTORS            0x8DFC
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS   0x8DFD
#define GL_RGB565                         0x8D62
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#define GL_PROGRAM_BINARY_LENGTH          0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS     0x87FE
#define GL_PROGRAM_BINARY_FORMATS         0x87FF
#define GL_VERTEX_SHADER_BIT              0x00000001
#define GL_FRAGMENT_SHADER_BIT            0x00000002
#define GL_GEOMETRY_SHADER_BIT            0x00000004
#define GL_TESS_CONTROL_SHADER_BIT        0x00000008
#define GL_TESS_EVALUATION_SHADER_BIT     0x00000010
#define GL_ALL_SHADER_BITS                0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE              0x8258
#define GL_ACTIVE_PROGRAM                 0x8259
#define GL_PROGRAM_PIPELINE_BINDING       0x825A
#define GL_MAX_VIEWPORTS                  0x825B
#define GL_VIEWPORT_SUBPIXEL_BITS         0x825C
#define GL_VIEWPORT_BOUNDS_RANGE          0x825D
#define GL_LAYER_PROVOKING_VERTEX         0x825E
#define GL_VIEWPORT_INDEX_PROVOKING_VERTEX 0x825F
#define GL_UNDEFINED_VERTEX               0x8260
DECLARE_GL_FUNCTION(glReleaseShaderCompiler, void, (void))
#define glReleaseShaderCompiler USE_GL_FUNCTION(p_glReleaseShaderCompiler)
DECLARE_GL_FUNCTION(glShaderBinary, void, (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length))
#define glShaderBinary USE_GL_FUNCTION(p_glShaderBinary)
DECLARE_GL_FUNCTION(glGetShaderPrecisionFormat, void, (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision))
#define glGetShaderPrecisionFormat USE_GL_FUNCTION(p_glGetShaderPrecisionFormat)
DECLARE_GL_FUNCTION(glDepthRangef, void, (GLfloat n, GLfloat f))
#define glDepthRangef USE_GL_FUNCTION(p_glDepthRangef)
DECLARE_GL_FUNCTION(glClearDepthf, void, (GLfloat d))
#define glClearDepthf USE_GL_FUNCTION(p_glClearDepthf)
DECLARE_GL_FUNCTION(glGetProgramBinary, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary))
#define glGetProgramBinary USE_GL_FUNCTION(p_glGetProgramBinary)
DECLARE_GL_FUNCTION(glProgramBinary, void, (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length))
#define glProgramBinary USE_GL_FUNCTION(p_glProgramBinary)
DECLARE_GL_FUNCTION(glProgramParameteri, void, (GLuint program, GLenum pname, GLint value))
#define glProgramParameteri USE_GL_FUNCTION(p_glProgramParameteri)
DECLARE_GL_FUNCTION(glUseProgramStages, void, (GLuint pipeline, GLbitfield stages, GLuint program))
#define glUseProgramStages USE_GL_FUNCTION(p_glUseProgramStages)
DECLARE_GL_FUNCTION(glActiveShaderProgram, void, (GLuint pipeline, GLuint program))
#define glActiveShaderProgram USE_GL_FUNCTION(p_glActiveShaderProgram)
DECLARE_GL_FUNCTION(glCreateShaderProgramv, GLuint, (GLenum type, GLsizei count, const GLchar *const*strings))
#define glCreateShaderProgramv USE_GL_FUNCTION(p_glCreateShaderProgramv)
DECLARE_GL_FUNCTION(glBindProgramPipeline, void, (GLuint pipeline))
#define glBindProgramPipeline USE_GL_FUNCTION(p_glBindProgramPipeline)
DECLARE_GL_FUNCTION(glDeleteProgramPipelines, void, (GLsizei n, const GLuint *pipelines))
#define glDeleteProgramPipelines USE_GL_FUNCTION(p_glDeleteProgramPipelines)
DECLARE_GL_FUNCTION(glGenProgramPipelines, void, (GLsizei n, GLuint *pipelines))
#define glGenProgramPipelines USE_GL_FUNCTION(p_glGenProgramPipelines)
DECLARE_GL_FUNCTION(glIsProgramPipeline, GLboolean, (GLuint pipeline))
#define glIsProgramPipeline USE_GL_FUNCTION(p_glIsProgramPipeline)
DECLARE_GL_FUNCTION(glGetProgramPipelineiv, void, (GLuint pipeline, GLenum pname, GLint *params))
#define glGetProgramPipelineiv USE_GL_FUNCTION(p_glGetProgramPipelineiv)
DECLARE_GL_FUNCTION(glProgramUniform1i, void, (GLuint program, GLint location, GLint v0))
#define glProgramUniform1i USE_GL_FUNCTION(p_glProgramUniform1i)
DECLARE_GL_FUNCTION(glProgramUniform1iv, void, (GLuint program, GLint location, GLsizei count, const GLint *value))
#define glProgramUniform1iv USE_GL_FUNCTION(p_glProgramUniform1iv)
DECLARE_GL_FUNCTION(glProgramUniform1f, void, (GLuint program, GLint location, GLfloat v0))
#define glProgramUniform1f USE_GL_FUNCTION(p_glProgramUniform1f)
DECLARE_GL_FUNCTION(glProgramUniform1fv, void, (GLuint program, GLint location, GLsizei count, const GLfloat *value))
#define glProgramUniform1fv USE_GL_FUNCTION(p_glProgramUniform1fv)
DECLARE_GL_FUNCTION(glProgramUniform1d, void, (GLuint program, GLint location, GLdouble v0))
#define glProgramUniform1d USE_GL_FUNCTION(p_glProgramUniform1d)
DECLARE_GL_FUNCTION(glProgramUniform1dv, void, (GLuint program, GLint location, GLsizei count, const GLdouble *value))
#define glProgramUniform1dv USE_GL_FUNCTION(p_glProgramUniform1dv)
DECLARE_GL_FUNCTION(glProgramUniform1ui, void, (GLuint program, GLint location, GLuint v0))
#define glProgramUniform1ui USE_GL_FUNCTION(p_glProgramUniform1ui)
DECLARE_GL_FUNCTION(glProgramUniform1uiv, void, (GLuint program, GLint location, GLsizei count, const GLuint *value))
#define glProgramUniform1uiv USE_GL_FUNCTION(p_glProgramUniform1uiv)
DECLARE_GL_FUNCTION(glProgramUniform2i, void, (GLuint program, GLint location, GLint v0, GLint v1))
#define glProgramUniform2i USE_GL_FUNCTION(p_glProgramUniform2i)
DECLARE_GL_FUNCTION(glProgramUniform2iv, void, (GLuint program, GLint location, GLsizei count, const GLint *value))
#define glProgramUniform2iv USE_GL_FUNCTION(p_glProgramUniform2iv)
DECLARE_GL_FUNCTION(glProgramUniform2f, void, (GLuint program, GLint location, GLfloat v0, GLfloat v1))
#define glProgramUniform2f USE_GL_FUNCTION(p_glProgramUniform2f)
DECLARE_GL_FUNCTION(glProgramUniform2fv, void, (GLuint program, GLint location, GLsizei count, const GLfloat *value))
#define glProgramUniform2fv USE_GL_FUNCTION(p_glProgramUniform2fv)
DECLARE_GL_FUNCTION(glProgramUniform2d, void, (GLuint program, GLint location, GLdouble v0, GLdouble v1))
#define glProgramUniform2d USE_GL_FUNCTION(p_glProgramUniform2d)
DECLARE_GL_FUNCTION(glProgramUniform2dv, void, (GLuint program, GLint location, GLsizei count, const GLdouble *value))
#define glProgramUniform2dv USE_GL_FUNCTION(p_glProgramUniform2dv)
DECLARE_GL_FUNCTION(glProgramUniform2ui, void, (GLuint program, GLint location, GLuint v0, GLuint v1))
#define glProgramUniform2ui USE_GL_FUNCTION(p_glProgramUniform2ui)
DECLARE_GL_FUNCTION(glProgramUniform2uiv, void, (GLuint program, GLint location, GLsizei count, const GLuint *value))
#define glProgramUniform2uiv USE_GL_FUNCTION(p_glProgramUniform2uiv)
DECLARE_GL_FUNCTION(glProgramUniform3i, void, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2))
#define glProgramUniform3i USE_GL_FUNCTION(p_glProgramUniform3i)
DECLARE_GL_FUNCTION(glProgramUniform3iv, void, (GLuint program, GLint location, GLsizei count, const GLint *value))
#define glProgramUniform3iv USE_GL_FUNCTION(p_glProgramUniform3iv)
DECLARE_GL_FUNCTION(glProgramUniform3f, void, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2))
#define glProgramUniform3f USE_GL_FUNCTION(p_glProgramUniform3f)
DECLARE_GL_FUNCTION(glProgramUniform3fv, void, (GLuint program, GLint location, GLsizei count, const GLfloat *value))
#define glProgramUniform3fv USE_GL_FUNCTION(p_glProgramUniform3fv)
DECLARE_GL_FUNCTION(glProgramUniform3d, void, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2))
#define glProgramUniform3d USE_GL_FUNCTION(p_glProgramUniform3d)
DECLARE_GL_FUNCTION(glProgramUniform3dv, void, (GLuint program, GLint location, GLsizei count, const GLdouble *value))
#define glProgramUniform3dv USE_GL_FUNCTION(p_glProgramUniform3dv)
DECLARE_GL_FUNCTION(glProgramUniform3ui, void, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2))
#define glProgramUniform3ui USE_GL_FUNCTION(p_glProgramUniform3ui)
DECLARE_GL_FUNCTION(glProgramUniform3uiv, void, (GLuint program, GLint location, GLsizei count, const GLuint *value))
#define glProgramUniform3uiv USE_GL_FUNCTION(p_glProgramUniform3uiv)
DECLARE_GL_FUNCTION(glProgramUniform4i, void, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3))
#define glProgramUniform4i USE_GL_FUNCTION(p_glProgramUniform4i)
DECLARE_GL_FUNCTION(glProgramUniform4iv, void, (GLuint program, GLint location, GLsizei count, const GLint *value))
#define glProgramUniform4iv USE_GL_FUNCTION(p_glProgramUniform4iv)
DECLARE_GL_FUNCTION(glProgramUniform4f, void, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
#define glProgramUniform4f USE_GL_FUNCTION(p_glProgramUniform4f)
DECLARE_GL_FUNCTION(glProgramUniform4fv, void, (GLuint program, GLint location, GLsizei count, const GLfloat *value))
#define glProgramUniform4fv USE_GL_FUNCTION(p_glProgramUniform4fv)
DECLARE_GL_FUNCTION(glProgramUniform4d, void, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3))
#define glProgramUniform4d USE_GL_FUNCTION(p_glProgramUniform4d)
DECLARE_GL_FUNCTION(glProgramUniform4dv, void, (GLuint program, GLint location, GLsizei count, const GLdouble *value))
#define glProgramUniform4dv USE_GL_FUNCTION(p_glProgramUniform4dv)
DECLARE_GL_FUNCTION(glProgramUniform4ui, void, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3))
#define glProgramUniform4ui USE_GL_FUNCTION(p_glProgramUniform4ui)
DECLARE_GL_FUNCTION(glProgramUniform4uiv, void, (GLuint program, GLint location, GLsizei count, const GLuint *value))
#define glProgramUniform4uiv USE_GL_FUNCTION(p_glProgramUniform4uiv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix2fv USE_GL_FUNCTION(p_glProgramUniformMatrix2fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix3fv USE_GL_FUNCTION(p_glProgramUniformMatrix3fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix4fv USE_GL_FUNCTION(p_glProgramUniformMatrix4fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix2dv USE_GL_FUNCTION(p_glProgramUniformMatrix2dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix3dv USE_GL_FUNCTION(p_glProgramUniformMatrix3dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix4dv USE_GL_FUNCTION(p_glProgramUniformMatrix4dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2x3fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix2x3fv USE_GL_FUNCTION(p_glProgramUniformMatrix2x3fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3x2fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix3x2fv USE_GL_FUNCTION(p_glProgramUniformMatrix3x2fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2x4fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix2x4fv USE_GL_FUNCTION(p_glProgramUniformMatrix2x4fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4x2fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix4x2fv USE_GL_FUNCTION(p_glProgramUniformMatrix4x2fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3x4fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix3x4fv USE_GL_FUNCTION(p_glProgramUniformMatrix3x4fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4x3fv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
#define glProgramUniformMatrix4x3fv USE_GL_FUNCTION(p_glProgramUniformMatrix4x3fv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2x3dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix2x3dv USE_GL_FUNCTION(p_glProgramUniformMatrix2x3dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3x2dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix3x2dv USE_GL_FUNCTION(p_glProgramUniformMatrix3x2dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix2x4dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix2x4dv USE_GL_FUNCTION(p_glProgramUniformMatrix2x4dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4x2dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix4x2dv USE_GL_FUNCTION(p_glProgramUniformMatrix4x2dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix3x4dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix3x4dv USE_GL_FUNCTION(p_glProgramUniformMatrix3x4dv)
DECLARE_GL_FUNCTION(glProgramUniformMatrix4x3dv, void, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value))
#define glProgramUniformMatrix4x3dv USE_GL_FUNCTION(p_glProgramUniformMatrix4x3dv)
DECLARE_GL_FUNCTION(glValidateProgramPipeline, void, (GLuint pipeline))
#define glValidateProgramPipeline USE_GL_FUNCTION(p_glValidateProgramPipeline)
DECLARE_GL_FUNCTION(glGetProgramPipelineInfoLog, void, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
#define glGetProgramPipelineInfoLog USE_GL_FUNCTION(p_glGetProgramPipelineInfoLog)
DECLARE_GL_FUNCTION(glVertexAttribL1d, void, (GLuint index, GLdouble x))
#define glVertexAttribL1d USE_GL_FUNCTION(p_glVertexAttribL1d)
DECLARE_GL_FUNCTION(glVertexAttribL2d, void, (GLuint index, GLdouble x, GLdouble y))
#define glVertexAttribL2d USE_GL_FUNCTION(p_glVertexAttribL2d)
DECLARE_GL_FUNCTION(glVertexAttribL3d, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z))
#define glVertexAttribL3d USE_GL_FUNCTION(p_glVertexAttribL3d)
DECLARE_GL_FUNCTION(glVertexAttribL4d, void, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w))
#define glVertexAttribL4d USE_GL_FUNCTION(p_glVertexAttribL4d)
DECLARE_GL_FUNCTION(glVertexAttribL1dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttribL1dv USE_GL_FUNCTION(p_glVertexAttribL1dv)
DECLARE_GL_FUNCTION(glVertexAttribL2dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttribL2dv USE_GL_FUNCTION(p_glVertexAttribL2dv)
DECLARE_GL_FUNCTION(glVertexAttribL3dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttribL3dv USE_GL_FUNCTION(p_glVertexAttribL3dv)
DECLARE_GL_FUNCTION(glVertexAttribL4dv, void, (GLuint index, const GLdouble *v))
#define glVertexAttribL4dv USE_GL_FUNCTION(p_glVertexAttribL4dv)
DECLARE_GL_FUNCTION(glVertexAttribLPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))
#define glVertexAttribLPointer USE_GL_FUNCTION(p_glVertexAttribLPointer)
DECLARE_GL_FUNCTION(glGetVertexAttribLdv, void, (GLuint index, GLenum pname, GLdouble *params))
#define glGetVertexAttribLdv USE_GL_FUNCTION(p_glGetVertexAttribLdv)
DECLARE_GL_FUNCTION(glViewportArrayv, void, (GLuint first, GLsizei count, const GLfloat *v))
#define glViewportArrayv USE_GL_FUNCTION(p_glViewportArrayv)
DECLARE_GL_FUNCTION(glViewportIndexedf, void, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h))
#define glViewportIndexedf USE_GL_FUNCTION(p_glViewportIndexedf)
DECLARE_GL_FUNCTION(glViewportIndexedfv, void, (GLuint index, const GLfloat *v))
#define glViewportIndexedfv USE_GL_FUNCTION(p_glViewportIndexedfv)
DECLARE_GL_FUNCTION(glScissorArrayv, void, (GLuint first, GLsizei count, const GLint *v))
#define glScissorArrayv USE_GL_FUNCTION(p_glScissorArrayv)
DECLARE_GL_FUNCTION(glScissorIndexed, void, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height))
#define glScissorIndexed USE_GL_FUNCTION(p_glScissorIndexed)
DECLARE_GL_FUNCTION(glScissorIndexedv, void, (GLuint index, const GLint *v))
#define glScissorIndexedv USE_GL_FUNCTION(p_glScissorIndexedv)
DECLARE_GL_FUNCTION(glDepthRangeArrayv, void, (GLuint first, GLsizei count, const GLdouble *v))
#define glDepthRangeArrayv USE_GL_FUNCTION(p_glDepthRangeArrayv)
DECLARE_GL_FUNCTION(glDepthRangeIndexed, void, (GLuint index, GLdouble n, GLdouble f))
#define glDepthRangeIndexed USE_GL_FUNCTION(p_glDepthRangeIndexed)
DECLARE_GL_FUNCTION(glGetFloati_v, void, (GLenum target, GLuint index, GLfloat *data))
#define glGetFloati_v USE_GL_FUNCTION(p_glGetFloati_v)
DECLARE_GL_FUNCTION(glGetDoublei_v, void, (GLenum target, GLuint index, GLdouble *data))
#define glGetDoublei_v USE_GL_FUNCTION(p_glGetDoublei_v)
#endif /* GL_VERSION_4_1 */

#ifndef GL_VERSION_4_2
#define GL_VERSION_4_2 1
#define GL_COPY_READ_BUFFER_BINDING       0x8F36
#define GL_COPY_WRITE_BUFFER_BINDING      0x8F37
#define GL_TRANSFORM_FEEDBACK_ACTIVE      0x8E24
#define GL_TRANSFORM_FEEDBACK_PAUSED      0x8E23
#define GL_UNPACK_COMPRESSED_BLOCK_WIDTH  0x9127
#define GL_UNPACK_COMPRESSED_BLOCK_HEIGHT 0x9128
#define GL_UNPACK_COMPRESSED_BLOCK_DEPTH  0x9129
#define GL_UNPACK_COMPRESSED_BLOCK_SIZE   0x912A
#define GL_PACK_COMPRESSED_BLOCK_WIDTH    0x912B
#define GL_PACK_COMPRESSED_BLOCK_HEIGHT   0x912C
#define GL_PACK_COMPRESSED_BLOCK_DEPTH    0x912D
#define GL_PACK_COMPRESSED_BLOCK_SIZE     0x912E
#define GL_NUM_SAMPLE_COUNTS              0x9380
#define GL_MIN_MAP_BUFFER_ALIGNMENT       0x90BC
#define GL_ATOMIC_COUNTER_BUFFER          0x92C0
#define GL_ATOMIC_COUNTER_BUFFER_BINDING  0x92C1
#define GL_ATOMIC_COUNTER_BUFFER_START    0x92C2
#define GL_ATOMIC_COUNTER_BUFFER_SIZE     0x92C3
#define GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE 0x92C4
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS 0x92C5
#define GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES 0x92C6
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER 0x92C7
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER 0x92C8
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER 0x92C9
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER 0x92CA
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER 0x92CB
#define GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS 0x92CC
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS 0x92CD
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS 0x92CE
#define GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS 0x92CF
#define GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS 0x92D0
#define GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS 0x92D1
#define GL_MAX_VERTEX_ATOMIC_COUNTERS     0x92D2
#define GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS 0x92D3
#define GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS 0x92D4
#define GL_MAX_GEOMETRY_ATOMIC_COUNTERS   0x92D5
#define GL_MAX_FRAGMENT_ATOMIC_COUNTERS   0x92D6
#define GL_MAX_COMBINED_ATOMIC_COUNTERS   0x92D7
#define GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE 0x92D8
#define GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS 0x92DC
#define GL_ACTIVE_ATOMIC_COUNTER_BUFFERS  0x92D9
#define GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX 0x92DA
#define GL_UNSIGNED_INT_ATOMIC_COUNTER    0x92DB
#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT 0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT      0x00000002
#define GL_UNIFORM_BARRIER_BIT            0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT      0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_COMMAND_BARRIER_BIT            0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT       0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT     0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT      0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT        0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT 0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT     0x00001000
#define GL_ALL_BARRIER_BITS               0xFFFFFFFF
#define GL_MAX_IMAGE_UNITS                0x8F38
#define GL_MAX_COMBINED_IMAGE_UNITS_AND_FRAGMENT_OUTPUTS 0x8F39
#define GL_IMAGE_BINDING_NAME             0x8F3A
#define GL_IMAGE_BINDING_LEVEL            0x8F3B
#define GL_IMAGE_BINDING_LAYERED          0x8F3C
#define GL_IMAGE_BINDING_LAYER            0x8F3D
#define GL_IMAGE_BINDING_ACCESS           0x8F3E
#define GL_IMAGE_1D                       0x904C
#define GL_IMAGE_2D                       0x904D
#define GL_IMAGE_3D                       0x904E
#define GL_IMAGE_2D_RECT                  0x904F
#define GL_IMAGE_CUBE                     0x9050
#define GL_IMAGE_BUFFER                   0x9051
#define GL_IMAGE_1D_ARRAY                 0x9052
#define GL_IMAGE_2D_ARRAY                 0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY           0x9054
#define GL_IMAGE_2D_MULTISAMPLE           0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY     0x9056
#define GL_INT_IMAGE_1D                   0x9057
#define GL_INT_IMAGE_2D                   0x9058
#define GL_INT_IMAGE_3D                   0x9059
#define GL_INT_IMAGE_2D_RECT              0x905A
#define GL_INT_IMAGE_CUBE                 0x905B
#define GL_INT_IMAGE_BUFFER               0x905C
#define GL_INT_IMAGE_1D_ARRAY             0x905D
#define GL_INT_IMAGE_2D_ARRAY             0x905E
#define GL_INT_IMAGE_CUBE_MAP_ARRAY       0x905F
#define GL_INT_IMAGE_2D_MULTISAMPLE       0x9060
#define GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x9061
#define GL_UNSIGNED_INT_IMAGE_1D          0x9062
#define GL_UNSIGNED_INT_IMAGE_2D          0x9063
#define GL_UNSIGNED_INT_IMAGE_3D          0x9064
#define GL_UNSIGNED_INT_IMAGE_2D_RECT     0x9065
#define GL_UNSIGNED_INT_IMAGE_CUBE        0x9066
#define GL_UNSIGNED_INT_IMAGE_BUFFER      0x9067
#define GL_UNSIGNED_INT_IMAGE_1D_ARRAY    0x9068
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY    0x9069
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE 0x906B
#define GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY 0x906C
#define GL_MAX_IMAGE_SAMPLES              0x906D
#define GL_IMAGE_BINDING_FORMAT           0x906E
#define GL_IMAGE_FORMAT_COMPATIBILITY_TYPE 0x90C7
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE 0x90C8
#define GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS 0x90C9
#define GL_MAX_VERTEX_IMAGE_UNIFORMS      0x90CA
#define GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS 0x90CB
#define GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS 0x90CC
#define GL_MAX_GEOMETRY_IMAGE_UNIFORMS    0x90CD
#define GL_MAX_FRAGMENT_IMAGE_UNIFORMS    0x90CE
#define GL_MAX_COMBINED_IMAGE_UNIFORMS    0x90CF
#define GL_COMPRESSED_RGBA_BPTC_UNORM     0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F
#define GL_TEXTURE_IMMUTABLE_FORMAT       0x912F
DECLARE_GL_FUNCTION(glDrawArraysInstancedBaseInstance, void, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance))
#define glDrawArraysInstancedBaseInstance USE_GL_FUNCTION(p_glDrawArraysInstancedBaseInstance)
DECLARE_GL_FUNCTION(glDrawElementsInstancedBaseInstance, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance))
#define glDrawElementsInstancedBaseInstance USE_GL_FUNCTION(p_glDrawElementsInstancedBaseInstance)
DECLARE_GL_FUNCTION(glDrawElementsInstancedBaseVertexBaseInstance, void, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance))
#define glDrawElementsInstancedBaseVertexBaseInstance USE_GL_FUNCTION(p_glDrawElementsInstancedBaseVertexBaseInstance)
DECLARE_GL_FUNCTION(glGetInternalformativ, void, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params))
#define glGetInternalformativ USE_GL_FUNCTION(p_glGetInternalformativ)
DECLARE_GL_FUNCTION(glGetActiveAtomicCounterBufferiv, void, (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params))
#define glGetActiveAtomicCounterBufferiv USE_GL_FUNCTION(p_glGetActiveAtomicCounterBufferiv)
DECLARE_GL_FUNCTION(glBindImageTexture, void, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format))
#define glBindImageTexture USE_GL_FUNCTION(p_glBindImageTexture)
DECLARE_GL_FUNCTION(glMemoryBarrier, void, (GLbitfield barriers))
#define glMemoryBarrier USE_GL_FUNCTION(p_glMemoryBarrier)
DECLARE_GL_FUNCTION(glTexStorage1D, void, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width))
#define glTexStorage1D USE_GL_FUNCTION(p_glTexStorage1D)
DECLARE_GL_FUNCTION(glTexStorage2D, void, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))
#define glTexStorage2D USE_GL_FUNCTION(p_glTexStorage2D)
DECLARE_GL_FUNCTION(glTexStorage3D, void, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))
#define glTexStorage3D USE_GL_FUNCTION(p_glTexStorage3D)
DECLARE_GL_FUNCTION(glDrawTransformFeedbackInstanced, void, (GLenum mode, GLuint id, GLsizei instancecount))
#define glDrawTransformFeedbackInstanced USE_GL_FUNCTION(p_glDrawTransformFeedbackInstanced)
DECLARE_GL_FUNCTION(glDrawTransformFeedbackStreamInstanced, void, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount))
#define glDrawTransformFeedbackStreamInstanced USE_GL_FUNCTION(p_glDrawTransformFeedbackStreamInstanced)
#endif /* GL_VERSION_4_2 */

#ifndef GL_VERSION_4_3
#define GL_VERSION_4_3 1
typedef void (APIENTRY  *GLDEBUGPROC)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam);
#define GL_NUM_SHADING_LANGUAGE_VERSIONS  0x82E9
#define GL_VERTEX_ATTRIB_ARRAY_LONG       0x874E
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_SRGB8_ETC2          0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC 0x9279
#define GL_COMPRESSED_R11_EAC             0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC      0x9271
#define GL_COMPRESSED_RG11_EAC            0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC     0x9273
#define GL_PRIMITIVE_RESTART_FIXED_INDEX  0x8D69
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE 0x8D6A
#define GL_MAX_ELEMENT_INDEX              0x8D6B
#define GL_COMPUTE_SHADER                 0x91B9
#define GL_MAX_COMPUTE_UNIFORM_BLOCKS     0x91BB
#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS 0x91BC
#define GL_MAX_COMPUTE_IMAGE_UNIFORMS     0x91BD
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE 0x8262
#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS 0x8263
#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS 0x8264
#define GL_MAX_COMPUTE_ATOMIC_COUNTERS    0x8265
#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS 0x8266
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS 0x90EB
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT   0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE    0x91BF
#define GL_COMPUTE_WORK_GROUP_SIZE        0x8267
#define GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER 0x90EC
#define GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_COMPUTE_SHADER 0x90ED
#define GL_DISPATCH_INDIRECT_BUFFER       0x90EE
#define GL_DISPATCH_INDIRECT_BUFFER_BINDING 0x90EF
#define GL_COMPUTE_SHADER_BIT             0x00000020
#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION        0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM      0x8245
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM     0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER   0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY       0x8249
#define GL_DEBUG_SOURCE_APPLICATION       0x824A
#define GL_DEBUG_SOURCE_OTHER             0x824B
#define GL_DEBUG_TYPE_ERROR               0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
#define GL_DEBUG_TYPE_PORTABILITY         0x824F
#define GL_DEBUG_TYPE_PERFORMANCE         0x8250
#define GL_DEBUG_TYPE_OTHER               0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH       0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES      0x9144
#define GL_DEBUG_LOGGED_MESSAGES          0x9145
#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH    0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH        0x826D
#define GL_BUFFER                         0x82E0
#define GL_SHADER                         0x82E1
#define GL_PROGRAM                        0x82E2
#define GL_QUERY                          0x82E3
#define GL_PROGRAM_PIPELINE               0x82E4
#define GL_SAMPLER                        0x82E6
#define GL_MAX_LABEL_LENGTH               0x82E8
#define GL_DEBUG_OUTPUT                   0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT         0x00000002
#define GL_MAX_UNIFORM_LOCATIONS          0x826E
#define GL_FRAMEBUFFER_DEFAULT_WIDTH      0x9310
#define GL_FRAMEBUFFER_DEFAULT_HEIGHT     0x9311
#define GL_FRAMEBUFFER_DEFAULT_LAYERS     0x9312
#define GL_FRAMEBUFFER_DEFAULT_SAMPLES    0x9313
#define GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS 0x9314
#define GL_MAX_FRAMEBUFFER_WIDTH          0x9315
#define GL_MAX_FRAMEBUFFER_HEIGHT         0x9316
#define GL_MAX_FRAMEBUFFER_LAYERS         0x9317
#define GL_MAX_FRAMEBUFFER_SAMPLES        0x9318
#define GL_INTERNALFORMAT_SUPPORTED       0x826F
#define GL_INTERNALFORMAT_PREFERRED       0x8270
#define GL_INTERNALFORMAT_RED_SIZE        0x8271
#define GL_INTERNALFORMAT_GREEN_SIZE      0x8272
#define GL_INTERNALFORMAT_BLUE_SIZE       0x8273
#define GL_INTERNALFORMAT_ALPHA_SIZE      0x8274
#define GL_INTERNALFORMAT_DEPTH_SIZE      0x8275
#define GL_INTERNALFORMAT_STENCIL_SIZE    0x8276
#define GL_INTERNALFORMAT_SHARED_SIZE     0x8277
#define GL_INTERNALFORMAT_RED_TYPE        0x8278
#define GL_INTERNALFORMAT_GREEN_TYPE      0x8279
#define GL_INTERNALFORMAT_BLUE_TYPE       0x827A
#define GL_INTERNALFORMAT_ALPHA_TYPE      0x827B
#define GL_INTERNALFORMAT_DEPTH_TYPE      0x827C
#define GL_INTERNALFORMAT_STENCIL_TYPE    0x827D
#define GL_MAX_WIDTH                      0x827E
#define GL_MAX_HEIGHT                     0x827F
#define GL_MAX_DEPTH                      0x8280
#define GL_MAX_LAYERS                     0x8281
#define GL_MAX_COMBINED_DIMENSIONS        0x8282
#define GL_COLOR_COMPONENTS               0x8283
#define GL_DEPTH_COMPONENTS               0x8284
#define GL_STENCIL_COMPONENTS             0x8285
#define GL_COLOR_RENDERABLE               0x8286
#define GL_DEPTH_RENDERABLE               0x8287
#define GL_STENCIL_RENDERABLE             0x8288
#define GL_FRAMEBUFFER_RENDERABLE         0x8289
#define GL_FRAMEBUFFER_RENDERABLE_LAYERED 0x828A
#define GL_FRAMEBUFFER_BLEND              0x828B
#define GL_READ_PIXELS                    0x828C
#define GL_READ_PIXELS_FORMAT             0x828D
#define GL_READ_PIXELS_TYPE               0x828E
#define GL_TEXTURE_IMAGE_FORMAT           0x828F
#define GL_TEXTURE_IMAGE_TYPE             0x8290
#define GL_GET_TEXTURE_IMAGE_FORMAT       0x8291
#define GL_GET_TEXTURE_IMAGE_TYPE         0x8292
#define GL_MIPMAP                         0x8293
#define GL_MANUAL_GENERATE_MIPMAP         0x8294
#define GL_AUTO_GENERATE_MIPMAP           0x8295
#define GL_COLOR_ENCODING                 0x8296
#define GL_SRGB_READ                      0x8297
#define GL_SRGB_WRITE                     0x8298
#define GL_FILTER                         0x829A
#define GL_VERTEX_TEXTURE                 0x829B
#define GL_TESS_CONTROL_TEXTURE           0x829C
#define GL_TESS_EVALUATION_TEXTURE        0x829D
#define GL_GEOMETRY_TEXTURE               0x829E
#define GL_FRAGMENT_TEXTURE               0x829F
#define GL_COMPUTE_TEXTURE                0x82A0
#define GL_TEXTURE_SHADOW                 0x82A1
#define GL_TEXTURE_GATHER                 0x82A2
#define GL_TEXTURE_GATHER_SHADOW          0x82A3
#define GL_SHADER_IMAGE_LOAD              0x82A4
#define GL_SHADER_IMAGE_STORE             0x82A5
#define GL_SHADER_IMAGE_ATOMIC            0x82A6
#define GL_IMAGE_TEXEL_SIZE               0x82A7
#define GL_IMAGE_COMPATIBILITY_CLASS      0x82A8
#define GL_IMAGE_PIXEL_FORMAT             0x82A9
#define GL_IMAGE_PIXEL_TYPE               0x82AA
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST 0x82AC
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST 0x82AD
#define GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE 0x82AE
#define GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE 0x82AF
#define GL_TEXTURE_COMPRESSED_BLOCK_WIDTH 0x82B1
#define GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT 0x82B2
#define GL_TEXTURE_COMPRESSED_BLOCK_SIZE  0x82B3
#define GL_CLEAR_BUFFER                   0x82B4
#define GL_TEXTURE_VIEW                   0x82B5
#define GL_VIEW_COMPATIBILITY_CLASS       0x82B6
#define GL_FULL_SUPPORT                   0x82B7
#define GL_CAVEAT_SUPPORT                 0x82B8
#define GL_IMAGE_CLASS_4_X_32             0x82B9
#define GL_IMAGE_CLASS_2_X_32             0x82BA
#define GL_IMAGE_CLASS_1_X_32             0x82BB
#define GL_IMAGE_CLASS_4_X_16             0x82BC
#define GL_IMAGE_CLASS_2_X_16             0x82BD
#define GL_IMAGE_CLASS_1_X_16             0x82BE
#define GL_IMAGE_CLASS_4_X_8              0x82BF
#define GL_IMAGE_CLASS_2_X_8              0x82C0
#define GL_IMAGE_CLASS_1_X_8              0x82C1
#define GL_IMAGE_CLASS_11_11_10           0x82C2
#define GL_IMAGE_CLASS_10_10_10_2         0x82C3
#define GL_VIEW_CLASS_128_BITS            0x82C4
#define GL_VIEW_CLASS_96_BITS             0x82C5
#define GL_VIEW_CLASS_64_BITS             0x82C6
#define GL_VIEW_CLASS_48_BITS             0x82C7
#define GL_VIEW_CLASS_32_BITS             0x82C8
#define GL_VIEW_CLASS_24_BITS             0x82C9
#define GL_VIEW_CLASS_16_BITS             0x82CA
#define GL_VIEW_CLASS_8_BITS              0x82CB
#define GL_VIEW_CLASS_S3TC_DXT1_RGB       0x82CC
#define GL_VIEW_CLASS_S3TC_DXT1_RGBA      0x82CD
#define GL_VIEW_CLASS_S3TC_DXT3_RGBA      0x82CE
#define GL_VIEW_CLASS_S3TC_DXT5_RGBA      0x82CF
#define GL_VIEW_CLASS_RGTC1_RED           0x82D0
#define GL_VIEW_CLASS_RGTC2_RG            0x82D1
#define GL_VIEW_CLASS_BPTC_UNORM          0x82D2
#define GL_VIEW_CLASS_BPTC_FLOAT          0x82D3
#define GL_UNIFORM                        0x92E1
#define GL_UNIFORM_BLOCK                  0x92E2
#define GL_PROGRAM_INPUT                  0x92E3
#define GL_PROGRAM_OUTPUT                 0x92E4
#define GL_BUFFER_VARIABLE                0x92E5
#define GL_SHADER_STORAGE_BLOCK           0x92E6
#define GL_VERTEX_SUBROUTINE              0x92E8
#define GL_TESS_CONTROL_SUBROUTINE        0x92E9
#define GL_TESS_EVALUATION_SUBROUTINE     0x92EA
#define GL_GEOMETRY_SUBROUTINE            0x92EB
#define GL_FRAGMENT_SUBROUTINE            0x92EC
#define GL_COMPUTE_SUBROUTINE             0x92ED
#define GL_VERTEX_SUBROUTINE_UNIFORM      0x92EE
#define GL_TESS_CONTROL_SUBROUTINE_UNIFORM 0x92EF
#define GL_TESS_EVALUATION_SUBROUTINE_UNIFORM 0x92F0
#define GL_GEOMETRY_SUBROUTINE_UNIFORM    0x92F1
#define GL_FRAGMENT_SUBROUTINE_UNIFORM    0x92F2
#define GL_COMPUTE_SUBROUTINE_UNIFORM     0x92F3
#define GL_TRANSFORM_FEEDBACK_VARYING     0x92F4
#define GL_ACTIVE_RESOURCES               0x92F5
#define GL_MAX_NAME_LENGTH                0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES       0x92F7
#define GL_MAX_NUM_COMPATIBLE_SUBROUTINES 0x92F8
#define GL_NAME_LENGTH                    0x92F9
#define GL_TYPE                           0x92FA
#define GL_ARRAY_SIZE                     0x92FB
#define GL_OFFSET                         0x92FC
#define GL_BLOCK_INDEX                    0x92FD
#define GL_ARRAY_STRIDE                   0x92FE
#define GL_MATRIX_STRIDE                  0x92FF
#define GL_IS_ROW_MAJOR                   0x9300
#define GL_ATOMIC_COUNTER_BUFFER_INDEX    0x9301
#define GL_BUFFER_BINDING                 0x9302
#define GL_BUFFER_DATA_SIZE               0x9303
#define GL_NUM_ACTIVE_VARIABLES           0x9304
#define GL_ACTIVE_VARIABLES               0x9305
#define GL_REFERENCED_BY_VERTEX_SHADER    0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER 0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER  0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER  0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER   0x930B
#define GL_TOP_LEVEL_ARRAY_SIZE           0x930C
#define GL_TOP_LEVEL_ARRAY_STRIDE         0x930D
#define GL_LOCATION                       0x930E
#define GL_LOCATION_INDEX                 0x930F
#define GL_IS_PER_PATCH                   0x92E7
#define GL_SHADER_STORAGE_BUFFER          0x90D2
#define GL_SHADER_STORAGE_BUFFER_BINDING  0x90D3
#define GL_SHADER_STORAGE_BUFFER_START    0x90D4
#define GL_SHADER_STORAGE_BUFFER_SIZE     0x90D5
#define GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS 0x90D6
#define GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS 0x90D7
#define GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS 0x90D8
#define GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS 0x90D9
#define GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS 0x90DA
#define GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS 0x90DB
#define GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS 0x90DC
#define GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS 0x90DD
#define GL_MAX_SHADER_STORAGE_BLOCK_SIZE  0x90DE
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT 0x90DF
#define GL_SHADER_STORAGE_BARRIER_BIT     0x00002000
#define GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES 0x8F39
#define GL_DEPTH_STENCIL_TEXTURE_MODE     0x90EA
#define GL_TEXTURE_BUFFER_OFFSET          0x919D
#define GL_TEXTURE_BUFFER_SIZE            0x919E
#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT 0x919F
#define GL_TEXTURE_VIEW_MIN_LEVEL         0x82DB
#define GL_TEXTURE_VIEW_NUM_LEVELS        0x82DC
#define GL_TEXTURE_VIEW_MIN_LAYER         0x82DD
#define GL_TEXTURE_VIEW_NUM_LAYERS        0x82DE
#define GL_TEXTURE_IMMUTABLE_LEVELS       0x82DF
#define GL_VERTEX_ATTRIB_BINDING          0x82D4
#define GL_VERTEX_ATTRIB_RELATIVE_OFFSET  0x82D5
#define GL_VERTEX_BINDING_DIVISOR         0x82D6
#define GL_VERTEX_BINDING_OFFSET          0x82D7
#define GL_VERTEX_BINDING_STRIDE          0x82D8
#define GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET 0x82D9
#define GL_MAX_VERTEX_ATTRIB_BINDINGS     0x82DA
#define GL_VERTEX_BINDING_BUFFER          0x8F4F
DECLARE_GL_FUNCTION(glClearBufferData, void, (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data))
#define glClearBufferData USE_GL_FUNCTION(p_glClearBufferData)
DECLARE_GL_FUNCTION(glClearBufferSubData, void, (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data))
#define glClearBufferSubData USE_GL_FUNCTION(p_glClearBufferSubData)
DECLARE_GL_FUNCTION(glDispatchCompute, void, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z))
#define glDispatchCompute USE_GL_FUNCTION(p_glDispatchCompute)
DECLARE_GL_FUNCTION(glDispatchComputeIndirect, void, (GLintptr indirect))
#define glDispatchComputeIndirect USE_GL_FUNCTION(p_glDispatchComputeIndirect)
DECLARE_GL_FUNCTION(glCopyImageSubData, void, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth))
#define glCopyImageSubData USE_GL_FUNCTION(p_glCopyImageSubData)
DECLARE_GL_FUNCTION(glFramebufferParameteri, void, (GLenum target, GLenum pname, GLint param))
#define glFramebufferParameteri USE_GL_FUNCTION(p_glFramebufferParameteri)
DECLARE_GL_FUNCTION(glGetFramebufferParameteriv, void, (GLenum target, GLenum pname, GLint *params))
#define glGetFramebufferParameteriv USE_GL_FUNCTION(p_glGetFramebufferParameteriv)
DECLARE_GL_FUNCTION(glGetInternalformati64v, void, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params))
#define glGetInternalformati64v USE_GL_FUNCTION(p_glGetInternalformati64v)
DECLARE_GL_FUNCTION(glInvalidateTexSubImage, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth))
#define glInvalidateTexSubImage USE_GL_FUNCTION(p_glInvalidateTexSubImage)
DECLARE_GL_FUNCTION(glInvalidateTexImage, void, (GLuint texture, GLint level))
#define glInvalidateTexImage USE_GL_FUNCTION(p_glInvalidateTexImage)
DECLARE_GL_FUNCTION(glInvalidateBufferSubData, void, (GLuint buffer, GLintptr offset, GLsizeiptr length))
#define glInvalidateBufferSubData USE_GL_FUNCTION(p_glInvalidateBufferSubData)
DECLARE_GL_FUNCTION(glInvalidateBufferData, void, (GLuint buffer))
#define glInvalidateBufferData USE_GL_FUNCTION(p_glInvalidateBufferData)
DECLARE_GL_FUNCTION(glInvalidateFramebuffer, void, (GLenum target, GLsizei numAttachments, const GLenum *attachments))
#define glInvalidateFramebuffer USE_GL_FUNCTION(p_glInvalidateFramebuffer)
DECLARE_GL_FUNCTION(glInvalidateSubFramebuffer, void, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height))
#define glInvalidateSubFramebuffer USE_GL_FUNCTION(p_glInvalidateSubFramebuffer)
DECLARE_GL_FUNCTION(glMultiDrawArraysIndirect, void, (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride))
#define glMultiDrawArraysIndirect USE_GL_FUNCTION(p_glMultiDrawArraysIndirect)
DECLARE_GL_FUNCTION(glMultiDrawElementsIndirect, void, (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride))
#define glMultiDrawElementsIndirect USE_GL_FUNCTION(p_glMultiDrawElementsIndirect)
DECLARE_GL_FUNCTION(glGetProgramInterfaceiv, void, (GLuint program, GLenum programInterface, GLenum pname, GLint *params))
#define glGetProgramInterfaceiv USE_GL_FUNCTION(p_glGetProgramInterfaceiv)
DECLARE_GL_FUNCTION(glGetProgramResourceIndex, GLuint, (GLuint program, GLenum programInterface, const GLchar *name))
#define glGetProgramResourceIndex USE_GL_FUNCTION(p_glGetProgramResourceIndex)
DECLARE_GL_FUNCTION(glGetProgramResourceName, void, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name))
#define glGetProgramResourceName USE_GL_FUNCTION(p_glGetProgramResourceName)
DECLARE_GL_FUNCTION(glGetProgramResourceiv, void, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params))
#define glGetProgramResourceiv USE_GL_FUNCTION(p_glGetProgramResourceiv)
DECLARE_GL_FUNCTION(glGetProgramResourceLocation, GLint, (GLuint program, GLenum programInterface, const GLchar *name))
#define glGetProgramResourceLocation USE_GL_FUNCTION(p_glGetProgramResourceLocation)
DECLARE_GL_FUNCTION(glGetProgramResourceLocationIndex, GLint, (GLuint program, GLenum programInterface, const GLchar *name))
#define glGetProgramResourceLocationIndex USE_GL_FUNCTION(p_glGetProgramResourceLocationIndex)
DECLARE_GL_FUNCTION(glShaderStorageBlockBinding, void, (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding))
#define glShaderStorageBlockBinding USE_GL_FUNCTION(p_glShaderStorageBlockBinding)
DECLARE_GL_FUNCTION(glTexBufferRange, void, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))
#define glTexBufferRange USE_GL_FUNCTION(p_glTexBufferRange)
DECLARE_GL_FUNCTION(glTexStorage2DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations))
#define glTexStorage2DMultisample USE_GL_FUNCTION(p_glTexStorage2DMultisample)
DECLARE_GL_FUNCTION(glTexStorage3DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))
#define glTexStorage3DMultisample USE_GL_FUNCTION(p_glTexStorage3DMultisample)
DECLARE_GL_FUNCTION(glTextureView, void, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers))
#define glTextureView USE_GL_FUNCTION(p_glTextureView)
DECLARE_GL_FUNCTION(glBindVertexBuffer, void, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride))
#define glBindVertexBuffer USE_GL_FUNCTION(p_glBindVertexBuffer)
DECLARE_GL_FUNCTION(glVertexAttribFormat, void, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset))
#define glVertexAttribFormat USE_GL_FUNCTION(p_glVertexAttribFormat)
DECLARE_GL_FUNCTION(glVertexAttribIFormat, void, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset))
#define glVertexAttribIFormat USE_GL_FUNCTION(p_glVertexAttribIFormat)
DECLARE_GL_FUNCTION(glVertexAttribLFormat, void, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset))
#define glVertexAttribLFormat USE_GL_FUNCTION(p_glVertexAttribLFormat)
DECLARE_GL_FUNCTION(glVertexAttribBinding, void, (GLuint attribindex, GLuint bindingindex))
#define glVertexAttribBinding USE_GL_FUNCTION(p_glVertexAttribBinding)
DECLARE_GL_FUNCTION(glVertexBindingDivisor, void, (GLuint bindingindex, GLuint divisor))
#define glVertexBindingDivisor USE_GL_FUNCTION(p_glVertexBindingDivisor)
DECLARE_GL_FUNCTION(glDebugMessageControl, void, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled))
#define glDebugMessageControl USE_GL_FUNCTION(p_glDebugMessageControl)
DECLARE_GL_FUNCTION(glDebugMessageInsert, void, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf))
#define glDebugMessageInsert USE_GL_FUNCTION(p_glDebugMessageInsert)
DECLARE_GL_FUNCTION(glDebugMessageCallbackRef, void, (GLDEBUGPROC callback, const void *userParam))
#define glDebugMessageCallbackRef USE_GL_FUNCTION(p_glDebugMessageCallbackRef)
DECLARE_GL_FUNCTION(glGetDebugMessageLog, GLuint, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog))
#define glGetDebugMessageLog USE_GL_FUNCTION(p_glGetDebugMessageLog)
DECLARE_GL_FUNCTION(glPushDebugGroup, void, (GLenum source, GLuint id, GLsizei length, const GLchar *message))
#define glPushDebugGroup USE_GL_FUNCTION(p_glPushDebugGroup)
DECLARE_GL_FUNCTION(glPopDebugGroup, void, (void))
#define glPopDebugGroup USE_GL_FUNCTION(p_glPopDebugGroup)
DECLARE_GL_FUNCTION(glObjectLabel, void, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label))
#define glObjectLabel USE_GL_FUNCTION(p_glObjectLabel)
DECLARE_GL_FUNCTION(glGetObjectLabel, void, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label))
#define glGetObjectLabel USE_GL_FUNCTION(p_glGetObjectLabel)
DECLARE_GL_FUNCTION(glObjectPtrLabel, void, (const void *ptr, GLsizei length, const GLchar *label))
#define glObjectPtrLabel USE_GL_FUNCTION(p_glObjectPtrLabel)
DECLARE_GL_FUNCTION(glGetObjectPtrLabel, void, (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label))
#define glGetObjectPtrLabel USE_GL_FUNCTION(p_glGetObjectPtrLabel)
#endif /* GL_VERSION_4_3 */

#ifndef GL_VERSION_4_4
#define GL_VERSION_4_4 1
#define GL_MAX_VERTEX_ATTRIB_STRIDE       0x82E5
#define GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED 0x8221
#define GL_TEXTURE_BUFFER_BINDING         0x8C2A
#define GL_MAP_PERSISTENT_BIT             0x0040
#define GL_MAP_COHERENT_BIT               0x0080
#define GL_DYNAMIC_STORAGE_BIT            0x0100
#define GL_CLIENT_STORAGE_BIT             0x0200
#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT 0x00004000
#define GL_BUFFER_IMMUTABLE_STORAGE       0x821F
#define GL_BUFFER_STORAGE_FLAGS           0x8220
#define GL_CLEAR_TEXTURE                  0x9365
#define GL_LOCATION_COMPONENT             0x934A
#define GL_TRANSFORM_FEEDBACK_BUFFER_INDEX 0x934B
#define GL_TRANSFORM_FEEDBACK_BUFFER_STRIDE 0x934C
#define GL_QUERY_BUFFER                   0x9192
#define GL_QUERY_BUFFER_BARRIER_BIT       0x00008000
#define GL_QUERY_BUFFER_BINDING           0x9193
#define GL_QUERY_RESULT_NO_WAIT           0x9194
#define GL_MIRROR_CLAMP_TO_EDGE           0x8743
DECLARE_GL_FUNCTION(glBufferStorage, void, (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags))
#define glBufferStorage USE_GL_FUNCTION(p_glBufferStorage)
DECLARE_GL_FUNCTION(glClearTexImage, void, (GLuint texture, GLint level, GLenum format, GLenum type, const void *data))
#define glClearTexImage USE_GL_FUNCTION(p_glClearTexImage)
DECLARE_GL_FUNCTION(glClearTexSubImage, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data))
#define glClearTexSubImage USE_GL_FUNCTION(p_glClearTexSubImage)
DECLARE_GL_FUNCTION(glBindBuffersBase, void, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers))
#define glBindBuffersBase USE_GL_FUNCTION(p_glBindBuffersBase)
DECLARE_GL_FUNCTION(glBindBuffersRange, void, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes))
#define glBindBuffersRange USE_GL_FUNCTION(p_glBindBuffersRange)
DECLARE_GL_FUNCTION(glBindTextures, void, (GLuint first, GLsizei count, const GLuint *textures))
#define glBindTextures USE_GL_FUNCTION(p_glBindTextures)
DECLARE_GL_FUNCTION(glBindSamplers, void, (GLuint first, GLsizei count, const GLuint *samplers))
#define glBindSamplers USE_GL_FUNCTION(p_glBindSamplers)
DECLARE_GL_FUNCTION(glBindImageTextures, void, (GLuint first, GLsizei count, const GLuint *textures))
#define glBindImageTextures USE_GL_FUNCTION(p_glBindImageTextures)
DECLARE_GL_FUNCTION(glBindVertexBuffers, void, (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides))
#define glBindVertexBuffers USE_GL_FUNCTION(p_glBindVertexBuffers)
#endif /* GL_VERSION_4_4 */

#ifndef GL_VERSION_4_5
#define GL_VERSION_4_5 1
#define GL_CONTEXT_LOST                   0x0507
#define GL_NEGATIVE_ONE_TO_ONE            0x935E
#define GL_ZERO_TO_ONE                    0x935F
#define GL_CLIP_ORIGIN                    0x935C
#define GL_CLIP_DEPTH_MODE                0x935D
#define GL_QUERY_WAIT_INVERTED            0x8E17
#define GL_QUERY_NO_WAIT_INVERTED         0x8E18
#define GL_QUERY_BY_REGION_WAIT_INVERTED  0x8E19
#define GL_QUERY_BY_REGION_NO_WAIT_INVERTED 0x8E1A
#define GL_MAX_CULL_DISTANCES             0x82F9
#define GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES 0x82FA
#define GL_TEXTURE_TARGET                 0x1006
#define GL_QUERY_TARGET                   0x82EA
#define GL_GUILTY_CONTEXT_RESET           0x8253
#define GL_INNOCENT_CONTEXT_RESET         0x8254
#define GL_UNKNOWN_CONTEXT_RESET          0x8255
#define GL_RESET_NOTIFICATION_STRATEGY    0x8256
#define GL_LOSE_CONTEXT_ON_RESET          0x8252
#define GL_NO_RESET_NOTIFICATION          0x8261
#define GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT 0x00000004
#define GL_CONTEXT_RELEASE_BEHAVIOR       0x82FB
#define GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH 0x82FC
DECLARE_GL_FUNCTION(glClipControl, void, (GLenum origin, GLenum depth))
#define glClipControl USE_GL_FUNCTION(p_glClipControl)
DECLARE_GL_FUNCTION(glCreateTransformFeedbacks, void, (GLsizei n, GLuint *ids))
#define glCreateTransformFeedbacks USE_GL_FUNCTION(p_glCreateTransformFeedbacks)
DECLARE_GL_FUNCTION(glTransformFeedbackBufferBase, void, (GLuint xfb, GLuint index, GLuint buffer))
#define glTransformFeedbackBufferBase USE_GL_FUNCTION(p_glTransformFeedbackBufferBase)
DECLARE_GL_FUNCTION(glTransformFeedbackBufferRange, void, (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size))
#define glTransformFeedbackBufferRange USE_GL_FUNCTION(p_glTransformFeedbackBufferRange)
DECLARE_GL_FUNCTION(glGetTransformFeedbackiv, void, (GLuint xfb, GLenum pname, GLint *param))
#define glGetTransformFeedbackiv USE_GL_FUNCTION(p_glGetTransformFeedbackiv)
DECLARE_GL_FUNCTION(glGetTransformFeedbacki_v, void, (GLuint xfb, GLenum pname, GLuint index, GLint *param))
#define glGetTransformFeedbacki_v USE_GL_FUNCTION(p_glGetTransformFeedbacki_v)
DECLARE_GL_FUNCTION(glGetTransformFeedbacki64_v, void, (GLuint xfb, GLenum pname, GLuint index, GLint64 *param))
#define glGetTransformFeedbacki64_v USE_GL_FUNCTION(p_glGetTransformFeedbacki64_v)
DECLARE_GL_FUNCTION(glCreateBuffers, void, (GLsizei n, GLuint *buffers))
#define glCreateBuffers USE_GL_FUNCTION(p_glCreateBuffers)
DECLARE_GL_FUNCTION(glNamedBufferStorage, void, (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags))
#define glNamedBufferStorage USE_GL_FUNCTION(p_glNamedBufferStorage)
DECLARE_GL_FUNCTION(glNamedBufferData, void, (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage))
#define glNamedBufferData USE_GL_FUNCTION(p_glNamedBufferData)
DECLARE_GL_FUNCTION(glNamedBufferSubData, void, (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data))
#define glNamedBufferSubData USE_GL_FUNCTION(p_glNamedBufferSubData)
DECLARE_GL_FUNCTION(glCopyNamedBufferSubData, void, (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size))
#define glCopyNamedBufferSubData USE_GL_FUNCTION(p_glCopyNamedBufferSubData)
DECLARE_GL_FUNCTION(glClearNamedBufferData, void, (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data))
#define glClearNamedBufferData USE_GL_FUNCTION(p_glClearNamedBufferData)
DECLARE_GL_FUNCTION(glClearNamedBufferSubData, void, (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data))
#define glClearNamedBufferSubData USE_GL_FUNCTION(p_glClearNamedBufferSubData)
DECLARE_GL_FUNCTION(glMapNamedBuffer, void *, (GLuint buffer, GLenum access))
#define glMapNamedBuffer USE_GL_FUNCTION(p_glMapNamedBuffer)
DECLARE_GL_FUNCTION(glMapNamedBufferRange, void *, (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access))
#define glMapNamedBufferRange USE_GL_FUNCTION(p_glMapNamedBufferRange)
DECLARE_GL_FUNCTION(glUnmapNamedBuffer, GLboolean, (GLuint buffer))
#define glUnmapNamedBuffer USE_GL_FUNCTION(p_glUnmapNamedBuffer)
DECLARE_GL_FUNCTION(glFlushMappedNamedBufferRange, void, (GLuint buffer, GLintptr offset, GLsizeiptr length))
#define glFlushMappedNamedBufferRange USE_GL_FUNCTION(p_glFlushMappedNamedBufferRange)
DECLARE_GL_FUNCTION(glGetNamedBufferParameteriv, void, (GLuint buffer, GLenum pname, GLint *params))
#define glGetNamedBufferParameteriv USE_GL_FUNCTION(p_glGetNamedBufferParameteriv)
DECLARE_GL_FUNCTION(glGetNamedBufferParameteri64v, void, (GLuint buffer, GLenum pname, GLint64 *params))
#define glGetNamedBufferParameteri64v USE_GL_FUNCTION(p_glGetNamedBufferParameteri64v)
DECLARE_GL_FUNCTION(glGetNamedBufferPointerv, void, (GLuint buffer, GLenum pname, void **params))
#define glGetNamedBufferPointerv USE_GL_FUNCTION(p_glGetNamedBufferPointerv)
DECLARE_GL_FUNCTION(glGetNamedBufferSubData, void, (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data))
#define glGetNamedBufferSubData USE_GL_FUNCTION(p_glGetNamedBufferSubData)
DECLARE_GL_FUNCTION(glCreateFramebuffers, void, (GLsizei n, GLuint *framebuffers))
#define glCreateFramebuffers USE_GL_FUNCTION(p_glCreateFramebuffers)
DECLARE_GL_FUNCTION(glNamedFramebufferRenderbuffer, void, (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))
#define glNamedFramebufferRenderbuffer USE_GL_FUNCTION(p_glNamedFramebufferRenderbuffer)
DECLARE_GL_FUNCTION(glNamedFramebufferParameteri, void, (GLuint framebuffer, GLenum pname, GLint param))
#define glNamedFramebufferParameteri USE_GL_FUNCTION(p_glNamedFramebufferParameteri)
DECLARE_GL_FUNCTION(glNamedFramebufferTexture, void, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level))
#define glNamedFramebufferTexture USE_GL_FUNCTION(p_glNamedFramebufferTexture)
DECLARE_GL_FUNCTION(glNamedFramebufferTextureLayer, void, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer))
#define glNamedFramebufferTextureLayer USE_GL_FUNCTION(p_glNamedFramebufferTextureLayer)
DECLARE_GL_FUNCTION(glNamedFramebufferDrawBuffer, void, (GLuint framebuffer, GLenum buf))
#define glNamedFramebufferDrawBuffer USE_GL_FUNCTION(p_glNamedFramebufferDrawBuffer)
DECLARE_GL_FUNCTION(glNamedFramebufferDrawBuffers, void, (GLuint framebuffer, GLsizei n, const GLenum *bufs))
#define glNamedFramebufferDrawBuffers USE_GL_FUNCTION(p_glNamedFramebufferDrawBuffers)
DECLARE_GL_FUNCTION(glNamedFramebufferReadBuffer, void, (GLuint framebuffer, GLenum src))
#define glNamedFramebufferReadBuffer USE_GL_FUNCTION(p_glNamedFramebufferReadBuffer)
DECLARE_GL_FUNCTION(glInvalidateNamedFramebufferData, void, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments))
#define glInvalidateNamedFramebufferData USE_GL_FUNCTION(p_glInvalidateNamedFramebufferData)
DECLARE_GL_FUNCTION(glInvalidateNamedFramebufferSubData, void, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height))
#define glInvalidateNamedFramebufferSubData USE_GL_FUNCTION(p_glInvalidateNamedFramebufferSubData)
DECLARE_GL_FUNCTION(glClearNamedFramebufferiv, void, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value))
#define glClearNamedFramebufferiv USE_GL_FUNCTION(p_glClearNamedFramebufferiv)
DECLARE_GL_FUNCTION(glClearNamedFramebufferuiv, void, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value))
#define glClearNamedFramebufferuiv USE_GL_FUNCTION(p_glClearNamedFramebufferuiv)
DECLARE_GL_FUNCTION(glClearNamedFramebufferfv, void, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value))
#define glClearNamedFramebufferfv USE_GL_FUNCTION(p_glClearNamedFramebufferfv)
DECLARE_GL_FUNCTION(glClearNamedFramebufferfi, void, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil))
#define glClearNamedFramebufferfi USE_GL_FUNCTION(p_glClearNamedFramebufferfi)
DECLARE_GL_FUNCTION(glBlitNamedFramebuffer, void, (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))
#define glBlitNamedFramebuffer USE_GL_FUNCTION(p_glBlitNamedFramebuffer)
DECLARE_GL_FUNCTION(glCheckNamedFramebufferStatus, GLenum, (GLuint framebuffer, GLenum target))
#define glCheckNamedFramebufferStatus USE_GL_FUNCTION(p_glCheckNamedFramebufferStatus)
DECLARE_GL_FUNCTION(glGetNamedFramebufferParameteriv, void, (GLuint framebuffer, GLenum pname, GLint *param))
#define glGetNamedFramebufferParameteriv USE_GL_FUNCTION(p_glGetNamedFramebufferParameteriv)
DECLARE_GL_FUNCTION(glGetNamedFramebufferAttachmentParameteriv, void, (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params))
#define glGetNamedFramebufferAttachmentParameteriv USE_GL_FUNCTION(p_glGetNamedFramebufferAttachmentParameteriv)
DECLARE_GL_FUNCTION(glCreateRenderbuffers, void, (GLsizei n, GLuint *renderbuffers))
#define glCreateRenderbuffers USE_GL_FUNCTION(p_glCreateRenderbuffers)
DECLARE_GL_FUNCTION(glNamedRenderbufferStorage, void, (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height))
#define glNamedRenderbufferStorage USE_GL_FUNCTION(p_glNamedRenderbufferStorage)
DECLARE_GL_FUNCTION(glNamedRenderbufferStorageMultisample, void, (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))
#define glNamedRenderbufferStorageMultisample USE_GL_FUNCTION(p_glNamedRenderbufferStorageMultisample)
DECLARE_GL_FUNCTION(glGetNamedRenderbufferParameteriv, void, (GLuint renderbuffer, GLenum pname, GLint *params))
#define glGetNamedRenderbufferParameteriv USE_GL_FUNCTION(p_glGetNamedRenderbufferParameteriv)
DECLARE_GL_FUNCTION(glCreateTextures, void, (GLenum target, GLsizei n, GLuint *textures))
#define glCreateTextures USE_GL_FUNCTION(p_glCreateTextures)
DECLARE_GL_FUNCTION(glTextureBuffer, void, (GLuint texture, GLenum internalformat, GLuint buffer))
#define glTextureBuffer USE_GL_FUNCTION(p_glTextureBuffer)
DECLARE_GL_FUNCTION(glTextureBufferRange, void, (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))
#define glTextureBufferRange USE_GL_FUNCTION(p_glTextureBufferRange)
DECLARE_GL_FUNCTION(glTextureStorage1D, void, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width))
#define glTextureStorage1D USE_GL_FUNCTION(p_glTextureStorage1D)
DECLARE_GL_FUNCTION(glTextureStorage2D, void, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))
#define glTextureStorage2D USE_GL_FUNCTION(p_glTextureStorage2D)
DECLARE_GL_FUNCTION(glTextureStorage3D, void, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))
#define glTextureStorage3D USE_GL_FUNCTION(p_glTextureStorage3D)
DECLARE_GL_FUNCTION(glTextureStorage2DMultisample, void, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations))
#define glTextureStorage2DMultisample USE_GL_FUNCTION(p_glTextureStorage2DMultisample)
DECLARE_GL_FUNCTION(glTextureStorage3DMultisample, void, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))
#define glTextureStorage3DMultisample USE_GL_FUNCTION(p_glTextureStorage3DMultisample)
DECLARE_GL_FUNCTION(glTextureSubImage1D, void, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels))
#define glTextureSubImage1D USE_GL_FUNCTION(p_glTextureSubImage1D)
DECLARE_GL_FUNCTION(glTextureSubImage2D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))
#define glTextureSubImage2D USE_GL_FUNCTION(p_glTextureSubImage2D)
DECLARE_GL_FUNCTION(glTextureSubImage3D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels))
#define glTextureSubImage3D USE_GL_FUNCTION(p_glTextureSubImage3D)
DECLARE_GL_FUNCTION(glCompressedTextureSubImage1D, void, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTextureSubImage1D USE_GL_FUNCTION(p_glCompressedTextureSubImage1D)
DECLARE_GL_FUNCTION(glCompressedTextureSubImage2D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTextureSubImage2D USE_GL_FUNCTION(p_glCompressedTextureSubImage2D)
DECLARE_GL_FUNCTION(glCompressedTextureSubImage3D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data))
#define glCompressedTextureSubImage3D USE_GL_FUNCTION(p_glCompressedTextureSubImage3D)
DECLARE_GL_FUNCTION(glCopyTextureSubImage1D, void, (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width))
#define glCopyTextureSubImage1D USE_GL_FUNCTION(p_glCopyTextureSubImage1D)
DECLARE_GL_FUNCTION(glCopyTextureSubImage2D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height))
#define glCopyTextureSubImage2D USE_GL_FUNCTION(p_glCopyTextureSubImage2D)
DECLARE_GL_FUNCTION(glCopyTextureSubImage3D, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))
#define glCopyTextureSubImage3D USE_GL_FUNCTION(p_glCopyTextureSubImage3D)
DECLARE_GL_FUNCTION(glTextureParameterf, void, (GLuint texture, GLenum pname, GLfloat param))
#define glTextureParameterf USE_GL_FUNCTION(p_glTextureParameterf)
DECLARE_GL_FUNCTION(glTextureParameterfv, void, (GLuint texture, GLenum pname, const GLfloat *param))
#define glTextureParameterfv USE_GL_FUNCTION(p_glTextureParameterfv)
DECLARE_GL_FUNCTION(glTextureParameteri, void, (GLuint texture, GLenum pname, GLint param))
#define glTextureParameteri USE_GL_FUNCTION(p_glTextureParameteri)
DECLARE_GL_FUNCTION(glTextureParameterIiv, void, (GLuint texture, GLenum pname, const GLint *params))
#define glTextureParameterIiv USE_GL_FUNCTION(p_glTextureParameterIiv)
DECLARE_GL_FUNCTION(glTextureParameterIuiv, void, (GLuint texture, GLenum pname, const GLuint *params))
#define glTextureParameterIuiv USE_GL_FUNCTION(p_glTextureParameterIuiv)
DECLARE_GL_FUNCTION(glTextureParameteriv, void, (GLuint texture, GLenum pname, const GLint *param))
#define glTextureParameteriv USE_GL_FUNCTION(p_glTextureParameteriv)
DECLARE_GL_FUNCTION(glGenerateTextureMipmap, void, (GLuint texture))
#define glGenerateTextureMipmap USE_GL_FUNCTION(p_glGenerateTextureMipmap)
DECLARE_GL_FUNCTION(glBindTextureUnit, void, (GLuint unit, GLuint texture))
#define glBindTextureUnit USE_GL_FUNCTION(p_glBindTextureUnit)
DECLARE_GL_FUNCTION(glGetTextureImage, void, (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels))
#define glGetTextureImage USE_GL_FUNCTION(p_glGetTextureImage)
DECLARE_GL_FUNCTION(glGetCompressedTextureImage, void, (GLuint texture, GLint level, GLsizei bufSize, void *pixels))
#define glGetCompressedTextureImage USE_GL_FUNCTION(p_glGetCompressedTextureImage)
DECLARE_GL_FUNCTION(glGetTextureLevelParameterfv, void, (GLuint texture, GLint level, GLenum pname, GLfloat *params))
#define glGetTextureLevelParameterfv USE_GL_FUNCTION(p_glGetTextureLevelParameterfv)
DECLARE_GL_FUNCTION(glGetTextureLevelParameteriv, void, (GLuint texture, GLint level, GLenum pname, GLint *params))
#define glGetTextureLevelParameteriv USE_GL_FUNCTION(p_glGetTextureLevelParameteriv)
DECLARE_GL_FUNCTION(glGetTextureParameterfv, void, (GLuint texture, GLenum pname, GLfloat *params))
#define glGetTextureParameterfv USE_GL_FUNCTION(p_glGetTextureParameterfv)
DECLARE_GL_FUNCTION(glGetTextureParameterIiv, void, (GLuint texture, GLenum pname, GLint *params))
#define glGetTextureParameterIiv USE_GL_FUNCTION(p_glGetTextureParameterIiv)
DECLARE_GL_FUNCTION(glGetTextureParameterIuiv, void, (GLuint texture, GLenum pname, GLuint *params))
#define glGetTextureParameterIuiv USE_GL_FUNCTION(p_glGetTextureParameterIuiv)
DECLARE_GL_FUNCTION(glGetTextureParameteriv, void, (GLuint texture, GLenum pname, GLint *params))
#define glGetTextureParameteriv USE_GL_FUNCTION(p_glGetTextureParameteriv)
DECLARE_GL_FUNCTION(glCreateVertexArrays, void, (GLsizei n, GLuint *arrays))
#define glCreateVertexArrays USE_GL_FUNCTION(p_glCreateVertexArrays)
DECLARE_GL_FUNCTION(glDisableVertexArrayAttrib, void, (GLuint vaobj, GLuint index))
#define glDisableVertexArrayAttrib USE_GL_FUNCTION(p_glDisableVertexArrayAttrib)
DECLARE_GL_FUNCTION(glEnableVertexArrayAttrib, void, (GLuint vaobj, GLuint index))
#define glEnableVertexArrayAttrib USE_GL_FUNCTION(p_glEnableVertexArrayAttrib)
DECLARE_GL_FUNCTION(glVertexArrayElementBuffer, void, (GLuint vaobj, GLuint buffer))
#define glVertexArrayElementBuffer USE_GL_FUNCTION(p_glVertexArrayElementBuffer)
DECLARE_GL_FUNCTION(glVertexArrayVertexBuffer, void, (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride))
#define glVertexArrayVertexBuffer USE_GL_FUNCTION(p_glVertexArrayVertexBuffer)
DECLARE_GL_FUNCTION(glVertexArrayVertexBuffers, void, (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides))
#define glVertexArrayVertexBuffers USE_GL_FUNCTION(p_glVertexArrayVertexBuffers)
DECLARE_GL_FUNCTION(glVertexArrayAttribBinding, void, (GLuint vaobj, GLuint attribindex, GLuint bindingindex))
#define glVertexArrayAttribBinding USE_GL_FUNCTION(p_glVertexArrayAttribBinding)
DECLARE_GL_FUNCTION(glVertexArrayAttribFormat, void, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset))
#define glVertexArrayAttribFormat USE_GL_FUNCTION(p_glVertexArrayAttribFormat)
DECLARE_GL_FUNCTION(glVertexArrayAttribIFormat, void, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset))
#define glVertexArrayAttribIFormat USE_GL_FUNCTION(p_glVertexArrayAttribIFormat)
DECLARE_GL_FUNCTION(glVertexArrayAttribLFormat, void, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset))
#define glVertexArrayAttribLFormat USE_GL_FUNCTION(p_glVertexArrayAttribLFormat)
DECLARE_GL_FUNCTION(glVertexArrayBindingDivisor, void, (GLuint vaobj, GLuint bindingindex, GLuint divisor))
#define glVertexArrayBindingDivisor USE_GL_FUNCTION(p_glVertexArrayBindingDivisor)
DECLARE_GL_FUNCTION(glGetVertexArrayiv, void, (GLuint vaobj, GLenum pname, GLint *param))
#define glGetVertexArrayiv USE_GL_FUNCTION(p_glGetVertexArrayiv)
DECLARE_GL_FUNCTION(glGetVertexArrayIndexediv, void, (GLuint vaobj, GLuint index, GLenum pname, GLint *param))
#define glGetVertexArrayIndexediv USE_GL_FUNCTION(p_glGetVertexArrayIndexediv)
DECLARE_GL_FUNCTION(glGetVertexArrayIndexed64iv, void, (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param))
#define glGetVertexArrayIndexed64iv USE_GL_FUNCTION(p_glGetVertexArrayIndexed64iv)
DECLARE_GL_FUNCTION(glCreateSamplers, void, (GLsizei n, GLuint *samplers))
#define glCreateSamplers USE_GL_FUNCTION(p_glCreateSamplers)
DECLARE_GL_FUNCTION(glCreateProgramPipelines, void, (GLsizei n, GLuint *pipelines))
#define glCreateProgramPipelines USE_GL_FUNCTION(p_glCreateProgramPipelines)
DECLARE_GL_FUNCTION(glCreateQueries, void, (GLenum target, GLsizei n, GLuint *ids))
#define glCreateQueries USE_GL_FUNCTION(p_glCreateQueries)
DECLARE_GL_FUNCTION(glGetQueryBufferObjecti64v, void, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset))
#define glGetQueryBufferObjecti64v USE_GL_FUNCTION(p_glGetQueryBufferObjecti64v)
DECLARE_GL_FUNCTION(glGetQueryBufferObjectiv, void, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset))
#define glGetQueryBufferObjectiv USE_GL_FUNCTION(p_glGetQueryBufferObjectiv)
DECLARE_GL_FUNCTION(glGetQueryBufferObjectui64v, void, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset))
#define glGetQueryBufferObjectui64v USE_GL_FUNCTION(p_glGetQueryBufferObjectui64v)
DECLARE_GL_FUNCTION(glGetQueryBufferObjectuiv, void, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset))
#define glGetQueryBufferObjectuiv USE_GL_FUNCTION(p_glGetQueryBufferObjectuiv)
DECLARE_GL_FUNCTION(glMemoryBarrierByRegion, void, (GLbitfield barriers))
#define glMemoryBarrierByRegion USE_GL_FUNCTION(p_glMemoryBarrierByRegion)
DECLARE_GL_FUNCTION(glGetTextureSubImage, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels))
#define glGetTextureSubImage USE_GL_FUNCTION(p_glGetTextureSubImage)
DECLARE_GL_FUNCTION(glGetCompressedTextureSubImage, void, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels))
#define glGetCompressedTextureSubImage USE_GL_FUNCTION(p_glGetCompressedTextureSubImage)
DECLARE_GL_FUNCTION(glGetGraphicsResetStatus, GLenum, (void))
#define glGetGraphicsResetStatus USE_GL_FUNCTION(p_glGetGraphicsResetStatus)
DECLARE_GL_FUNCTION(glGetnCompressedTexImage, void, (GLenum target, GLint lod, GLsizei bufSize, void *pixels))
#define glGetnCompressedTexImage USE_GL_FUNCTION(p_glGetnCompressedTexImage)
DECLARE_GL_FUNCTION(glGetnTexImage, void, (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels))
#define glGetnTexImage USE_GL_FUNCTION(p_glGetnTexImage)
DECLARE_GL_FUNCTION(glGetnUniformdv, void, (GLuint program, GLint location, GLsizei bufSize, GLdouble *params))
#define glGetnUniformdv USE_GL_FUNCTION(p_glGetnUniformdv)
DECLARE_GL_FUNCTION(glGetnUniformfv, void, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params))
#define glGetnUniformfv USE_GL_FUNCTION(p_glGetnUniformfv)
DECLARE_GL_FUNCTION(glGetnUniformiv, void, (GLuint program, GLint location, GLsizei bufSize, GLint *params))
#define glGetnUniformiv USE_GL_FUNCTION(p_glGetnUniformiv)
DECLARE_GL_FUNCTION(glGetnUniformuiv, void, (GLuint program, GLint location, GLsizei bufSize, GLuint *params))
#define glGetnUniformuiv USE_GL_FUNCTION(p_glGetnUniformuiv)
DECLARE_GL_FUNCTION(glReadnPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data))
#define glReadnPixels USE_GL_FUNCTION(p_glReadnPixels)
DECLARE_GL_FUNCTION(glTextureBarrier, void, (void))
#define glTextureBarrier USE_GL_FUNCTION(p_glTextureBarrier)
#endif /* GL_VERSION_4_5 */

#ifndef GL_VERSION_4_6
#define GL_VERSION_4_6 1
#define GL_SHADER_BINARY_FORMAT_SPIR_V    0x9551
#define GL_SPIR_V_BINARY                  0x9552
#define GL_PARAMETER_BUFFER               0x80EE
#define GL_PARAMETER_BUFFER_BINDING       0x80EF
#define GL_CONTEXT_FLAG_NO_ERROR_BIT      0x00000008
#define GL_VERTICES_SUBMITTED             0x82EE
#define GL_PRIMITIVES_SUBMITTED           0x82EF
#define GL_VERTEX_SHADER_INVOCATIONS      0x82F0
#define GL_TESS_CONTROL_SHADER_PATCHES    0x82F1
#define GL_TESS_EVALUATION_SHADER_INVOCATIONS 0x82F2
#define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED 0x82F3
#define GL_FRAGMENT_SHADER_INVOCATIONS    0x82F4
#define GL_COMPUTE_SHADER_INVOCATIONS     0x82F5
#define GL_CLIPPING_INPUT_PRIMITIVES      0x82F6
#define GL_CLIPPING_OUTPUT_PRIMITIVES     0x82F7
#define GL_POLYGON_OFFSET_CLAMP           0x8E1B
#define GL_SPIR_V_EXTENSIONS              0x9553
#define GL_NUM_SPIR_V_EXTENSIONS          0x9554
#define GL_TEXTURE_MAX_ANISOTROPY         0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY     0x84FF
#define GL_TRANSFORM_FEEDBACK_OVERFLOW    0x82EC
#define GL_TRANSFORM_FEEDBACK_STREAM_OVERFLOW 0x82ED
DECLARE_GL_FUNCTION(glSpecializeShader, void, (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue))
#define glSpecializeShader USE_GL_FUNCTION(p_glSpecializeShader)
DECLARE_GL_FUNCTION(glMultiDrawArraysIndirectCount, void, (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))
#define glMultiDrawArraysIndirectCount USE_GL_FUNCTION(p_glMultiDrawArraysIndirectCount)
DECLARE_GL_FUNCTION(glMultiDrawElementsIndirectCount, void, (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))
#define glMultiDrawElementsIndirectCount USE_GL_FUNCTION(p_glMultiDrawElementsIndirectCount)
DECLARE_GL_FUNCTION(glPolygonOffsetClamp, void, (GLfloat factor, GLfloat units, GLfloat clamp))
#define glPolygonOffsetClamp USE_GL_FUNCTION(p_glPolygonOffsetClamp)
#endif /* GL_VERSION_4_6 */

