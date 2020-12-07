#ifndef _gl_glext_h
#define _gl_glext_h

#ifdef __cplusplus
extern "C" {
#endif

#ifndef GL_API
# define GL_API
#endif

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
#define GL_RESCALE_NORMAL                 0x803A
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SINGLE_COLOR                   0x81F9
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
typedef void(*PFNGLDRAWRANGEELEMENTSPROC)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
typedef void(*PFNGLTEXIMAGE3DPROC)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void(*PFNGLTEXSUBIMAGE3DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
typedef void(*PFNGLCOPYTEXSUBIMAGE3DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#ifdef GL_GLEXT_PROTOTYPES
// GL_API void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);
// GL_API void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
// GL_API void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
// GL_API void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
#endif
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
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1
#define GL_MAX_TEXTURE_UNITS              0x84E2
#define GL_TRANSPOSE_MODELVIEW_MATRIX     0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX    0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX       0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX         0x84E6
#define GL_MULTISAMPLE_BIT                0x20000000
#define GL_NORMAL_MAP                     0x8511
#define GL_REFLECTION_MAP                 0x8512
#define GL_COMPRESSED_ALPHA               0x84E9
#define GL_COMPRESSED_LUMINANCE           0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA     0x84EB
#define GL_COMPRESSED_INTENSITY           0x84EC
#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_SOURCE0_RGB                    0x8580
#define GL_SOURCE1_RGB                    0x8581
#define GL_SOURCE2_RGB                    0x8582
#define GL_SOURCE0_ALPHA                  0x8588
#define GL_SOURCE1_ALPHA                  0x8589
#define GL_SOURCE2_ALPHA                  0x858A
#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_SUBTRACT                       0x84E7
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF
typedef void(*PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void(*PFNGLSAMPLECOVERAGEPROC)(GLfloat value, GLboolean invert);
typedef void(*PFNGLCOMPRESSEDTEXIMAGE3DPROC)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
typedef void(*PFNGLCOMPRESSEDTEXIMAGE2DPROC)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
typedef void(*PFNGLCOMPRESSEDTEXIMAGE1DPROC)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
typedef void(*PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
typedef void(*PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
typedef void(*PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
typedef void(*PFNGLGETCOMPRESSEDTEXIMAGEPROC)(GLenum target, GLint level, void *img);
typedef void(*PFNGLCLIENTACTIVETEXTUREPROC)(GLenum texture);
typedef void(*PFNGLMULTITEXCOORD1DPROC)(GLenum target, GLdouble s);
typedef void(*PFNGLMULTITEXCOORD1DVPROC)(GLenum target, const GLdouble *v);
typedef void(*PFNGLMULTITEXCOORD1FPROC)(GLenum target, GLfloat s);
typedef void(*PFNGLMULTITEXCOORD1FVPROC)(GLenum target, const GLfloat *v);
typedef void(*PFNGLMULTITEXCOORD1IPROC)(GLenum target, GLint s);
typedef void(*PFNGLMULTITEXCOORD1IVPROC)(GLenum target, const GLint *v);
typedef void(*PFNGLMULTITEXCOORD1SPROC)(GLenum target, GLshort s);
typedef void(*PFNGLMULTITEXCOORD1SVPROC)(GLenum target, const GLshort *v);
typedef void(*PFNGLMULTITEXCOORD2DPROC)(GLenum target, GLdouble s, GLdouble t);
typedef void(*PFNGLMULTITEXCOORD2DVPROC)(GLenum target, const GLdouble *v);
typedef void(*PFNGLMULTITEXCOORD2FPROC)(GLenum target, GLfloat s, GLfloat t);
typedef void(*PFNGLMULTITEXCOORD2FVPROC)(GLenum target, const GLfloat *v);
typedef void(*PFNGLMULTITEXCOORD2IPROC)(GLenum target, GLint s, GLint t);
typedef void(*PFNGLMULTITEXCOORD2IVPROC)(GLenum target, const GLint *v);
typedef void(*PFNGLMULTITEXCOORD2SPROC)(GLenum target, GLshort s, GLshort t);
typedef void(*PFNGLMULTITEXCOORD2SVPROC)(GLenum target, const GLshort *v);
typedef void(*PFNGLMULTITEXCOORD3DPROC)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
typedef void(*PFNGLMULTITEXCOORD3DVPROC)(GLenum target, const GLdouble *v);
typedef void(*PFNGLMULTITEXCOORD3FPROC)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
typedef void(*PFNGLMULTITEXCOORD3FVPROC)(GLenum target, const GLfloat *v);
typedef void(*PFNGLMULTITEXCOORD3IPROC)(GLenum target, GLint s, GLint t, GLint r);
typedef void(*PFNGLMULTITEXCOORD3IVPROC)(GLenum target, const GLint *v);
typedef void(*PFNGLMULTITEXCOORD3SPROC)(GLenum target, GLshort s, GLshort t, GLshort r);
typedef void(*PFNGLMULTITEXCOORD3SVPROC)(GLenum target, const GLshort *v);
typedef void(*PFNGLMULTITEXCOORD4DPROC)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef void(*PFNGLMULTITEXCOORD4DVPROC)(GLenum target, const GLdouble *v);
typedef void(*PFNGLMULTITEXCOORD4FPROC)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef void(*PFNGLMULTITEXCOORD4FVPROC)(GLenum target, const GLfloat *v);
typedef void(*PFNGLMULTITEXCOORD4IPROC)(GLenum target, GLint s, GLint t, GLint r, GLint q);
typedef void(*PFNGLMULTITEXCOORD4IVPROC)(GLenum target, const GLint *v);
typedef void(*PFNGLMULTITEXCOORD4SPROC)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
typedef void(*PFNGLMULTITEXCOORD4SVPROC)(GLenum target, const GLshort *v);
typedef void(*PFNGLLOADTRANSPOSEMATRIXFPROC)(const GLfloat *m);
typedef void(*PFNGLLOADTRANSPOSEMATRIXDPROC)(const GLdouble *m);
typedef void(*PFNGLMULTTRANSPOSEMATRIXFPROC)(const GLfloat *m);
typedef void(*PFNGLMULTTRANSPOSEMATRIXDPROC)(const GLdouble *m);
#ifdef GL_GLEXT_PROTOTYPES
GL_API void glActiveTexture(GLenum texture);
// GL_API void glSampleCoverage(GLfloat value, GLboolean invert);
// GL_API void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
// GL_API void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
// GL_API void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
// GL_API void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
// GL_API void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
// GL_API void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
// GL_API void glGetCompressedTexImage(GLenum target, GLint level, void *img);
GL_API void glClientActiveTexture(GLenum texture);
// GL_API void glMultiTexCoord1d(GLenum target, GLdouble s);
// GL_API void glMultiTexCoord1dv(GLenum target, const GLdouble *v);
// GL_API void glMultiTexCoord1f(GLenum target, GLfloat s);
// GL_API void glMultiTexCoord1fv(GLenum target, const GLfloat *v);
// GL_API void glMultiTexCoord1i(GLenum target, GLint s);
// GL_API void glMultiTexCoord1iv(GLenum target, const GLint *v);
// GL_API void glMultiTexCoord1s(GLenum target, GLshort s);
// GL_API void glMultiTexCoord1sv(GLenum target, const GLshort *v);
// GL_API void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t);
// GL_API void glMultiTexCoord2dv(GLenum target, const GLdouble *v);
GL_API void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
// GL_API void glMultiTexCoord2fv(GLenum target, const GLfloat *v);
// GL_API void glMultiTexCoord2i(GLenum target, GLint s, GLint t);
// GL_API void glMultiTexCoord2iv(GLenum target, const GLint *v);
// GL_API void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t);
// GL_API void glMultiTexCoord2sv(GLenum target, const GLshort *v);
// GL_API void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r);
// GL_API void glMultiTexCoord3dv(GLenum target, const GLdouble *v);
GL_API void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
// GL_API void glMultiTexCoord3fv(GLenum target, const GLfloat *v);
// GL_API void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);
// GL_API void glMultiTexCoord3iv(GLenum target, const GLint *v);
// GL_API void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);
// GL_API void glMultiTexCoord3sv(GLenum target, const GLshort *v);
// GL_API void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
// GL_API void glMultiTexCoord4dv(GLenum target, const GLdouble *v);
GL_API void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
// GL_API void glMultiTexCoord4fv(GLenum target, const GLfloat *v);
// GL_API void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q);
// GL_API void glMultiTexCoord4iv(GLenum target, const GLint *v);
// GL_API void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
// GL_API void glMultiTexCoord4sv(GLenum target, const GLshort *v);
GL_API void glLoadTransposeMatrixf(const GLfloat *m);
GL_API void glLoadTransposeMatrixd(const GLdouble *m);
GL_API void glMultTransposeMatrixf(const GLfloat *m);
GL_API void glMultTransposeMatrixd(const GLdouble *m);
#endif
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
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_GENERATE_MIPMAP                0x8191
#define GL_GENERATE_MIPMAP_HINT           0x8192
#define GL_FOG_COORDINATE_SOURCE          0x8450
#define GL_FOG_COORDINATE                 0x8451
#define GL_FRAGMENT_DEPTH                 0x8452
#define GL_CURRENT_FOG_COORDINATE         0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE      0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE    0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER   0x8456
#define GL_FOG_COORDINATE_ARRAY           0x8457
#define GL_COLOR_SUM                      0x8458
#define GL_CURRENT_SECONDARY_COLOR        0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE     0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE     0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE   0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER  0x845D
#define GL_SECONDARY_COLOR_ARRAY          0x845E
#define GL_TEXTURE_FILTER_CONTROL         0x8500
#define GL_DEPTH_TEXTURE_MODE             0x884B
#define GL_COMPARE_R_TO_TEXTURE           0x884E
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
typedef void(*PFNGLBLENDFUNCSEPARATEPROC)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void(*PFNGLMULTIDRAWARRAYSPROC)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
typedef void(*PFNGLMULTIDRAWELEMENTSPROC)(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
typedef void(*PFNGLPOINTPARAMETERFPROC)(GLenum pname, GLfloat param);
typedef void(*PFNGLPOINTPARAMETERFVPROC)(GLenum pname, const GLfloat *params);
typedef void(*PFNGLPOINTPARAMETERIPROC)(GLenum pname, GLint param);
typedef void(*PFNGLPOINTPARAMETERIVPROC)(GLenum pname, const GLint *params);
typedef void(*PFNGLFOGCOORDFPROC)(GLfloat coord);
typedef void(*PFNGLFOGCOORDFVPROC)(const GLfloat *coord);
typedef void(*PFNGLFOGCOORDDPROC)(GLdouble coord);
typedef void(*PFNGLFOGCOORDDVPROC)(const GLdouble *coord);
typedef void(*PFNGLFOGCOORDPOINTERPROC)(GLenum type, GLsizei stride, const void *pointer);
typedef void(*PFNGLSECONDARYCOLOR3BPROC)(GLbyte red, GLbyte green, GLbyte blue);
typedef void(*PFNGLSECONDARYCOLOR3BVPROC)(const GLbyte *v);
typedef void(*PFNGLSECONDARYCOLOR3DPROC)(GLdouble red, GLdouble green, GLdouble blue);
typedef void(*PFNGLSECONDARYCOLOR3DVPROC)(const GLdouble *v);
typedef void(*PFNGLSECONDARYCOLOR3FPROC)(GLfloat red, GLfloat green, GLfloat blue);
typedef void(*PFNGLSECONDARYCOLOR3FVPROC)(const GLfloat *v);
typedef void(*PFNGLSECONDARYCOLOR3IPROC)(GLint red, GLint green, GLint blue);
typedef void(*PFNGLSECONDARYCOLOR3IVPROC)(const GLint *v);
typedef void(*PFNGLSECONDARYCOLOR3SPROC)(GLshort red, GLshort green, GLshort blue);
typedef void(*PFNGLSECONDARYCOLOR3SVPROC)(const GLshort *v);
typedef void(*PFNGLSECONDARYCOLOR3UBPROC)(GLubyte red, GLubyte green, GLubyte blue);
typedef void(*PFNGLSECONDARYCOLOR3UBVPROC)(const GLubyte *v);
typedef void(*PFNGLSECONDARYCOLOR3UIPROC)(GLuint red, GLuint green, GLuint blue);
typedef void(*PFNGLSECONDARYCOLOR3UIVPROC)(const GLuint *v);
typedef void(*PFNGLSECONDARYCOLOR3USPROC)(GLushort red, GLushort green, GLushort blue);
typedef void(*PFNGLSECONDARYCOLOR3USVPROC)(const GLushort *v);
typedef void(*PFNGLSECONDARYCOLORPOINTERPROC)(GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void(*PFNGLWINDOWPOS2DPROC)(GLdouble x, GLdouble y);
typedef void(*PFNGLWINDOWPOS2DVPROC)(const GLdouble *v);
typedef void(*PFNGLWINDOWPOS2FPROC)(GLfloat x, GLfloat y);
typedef void(*PFNGLWINDOWPOS2FVPROC)(const GLfloat *v);
typedef void(*PFNGLWINDOWPOS2IPROC)(GLint x, GLint y);
typedef void(*PFNGLWINDOWPOS2IVPROC)(const GLint *v);
typedef void(*PFNGLWINDOWPOS2SPROC)(GLshort x, GLshort y);
typedef void(*PFNGLWINDOWPOS2SVPROC)(const GLshort *v);
typedef void(*PFNGLWINDOWPOS3DPROC)(GLdouble x, GLdouble y, GLdouble z);
typedef void(*PFNGLWINDOWPOS3DVPROC)(const GLdouble *v);
typedef void(*PFNGLWINDOWPOS3FPROC)(GLfloat x, GLfloat y, GLfloat z);
typedef void(*PFNGLWINDOWPOS3FVPROC)(const GLfloat *v);
typedef void(*PFNGLWINDOWPOS3IPROC)(GLint x, GLint y, GLint z);
typedef void(*PFNGLWINDOWPOS3IVPROC)(const GLint *v);
typedef void(*PFNGLWINDOWPOS3SPROC)(GLshort x, GLshort y, GLshort z);
typedef void(*PFNGLWINDOWPOS3SVPROC)(const GLshort *v);
typedef void(*PFNGLBLENDCOLORPROC)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void(*PFNGLBLENDEQUATIONPROC)(GLenum mode);
#ifdef GL_GLEXT_PROTOTYPES
// GL_API void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
// GL_API void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
// GL_API void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount);
// GL_API void glPointParameterf(GLenum pname, GLfloat param);
// GL_API void glPointParameterfv(GLenum pname, const GLfloat *params);
// GL_API void glPointParameteri(GLenum pname, GLint param);
// GL_API void glPointParameteriv(GLenum pname, const GLint *params);
GL_API void glFogCoordf(GLfloat coord);
// GL_API void glFogCoordfv(const GLfloat *coord);
// GL_API void glFogCoordd(GLdouble coord);
// GL_API void glFogCoorddv(const GLdouble *coord);
GL_API void glFogCoordPointer(GLenum type, GLsizei stride, const void *pointer);
// GL_API void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue);
// GL_API void glSecondaryColor3bv(const GLbyte *v);
// GL_API void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue);
// GL_API void glSecondaryColor3dv(const GLdouble *v);
GL_API void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue);
GL_API void glSecondaryColor4f(GLfloat red, GLfloat green, GLfloat blue);
// GL_API void glSecondaryColor3fv(const GLfloat *v);
// GL_API void glSecondaryColor3i(GLint red, GLint green, GLint blue);
// GL_API void glSecondaryColor3iv(const GLint *v);
// GL_API void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue);
// GL_API void glSecondaryColor3sv(const GLshort *v);
GL_API void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue);
// GL_API void glSecondaryColor3ubv(const GLubyte *v);
// GL_API void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue);
// GL_API void glSecondaryColor3uiv(const GLuint *v);
// GL_API void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue);
// GL_API void glSecondaryColor3usv(const GLushort *v);
GL_API void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
// GL_API void glWindowPos2d(GLdouble x, GLdouble y);
// GL_API void glWindowPos2dv(const GLdouble *v);
// GL_API void glWindowPos2f(GLfloat x, GLfloat y);
// GL_API void glWindowPos2fv(const GLfloat *v);
// GL_API void glWindowPos2i(GLint x, GLint y);
// GL_API void glWindowPos2iv(const GLint *v);
// GL_API void glWindowPos2s(GLshort x, GLshort y);
// GL_API void glWindowPos2sv(const GLshort *v);
// GL_API void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z);
// GL_API void glWindowPos3dv(const GLdouble *v);
// GL_API void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z);
// GL_API void glWindowPos3fv(const GLfloat *v);
// GL_API void glWindowPos3i(GLint x, GLint y, GLint z);
// GL_API void glWindowPos3iv(const GLint *v);
// GL_API void glWindowPos3s(GLshort x, GLshort y, GLshort z);
// GL_API void glWindowPos3sv(const GLshort *v);
GL_API void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
GL_API void glBlendEquation(GLenum mode);
#endif
#endif /* GL_VERSION_1_4 */

#ifndef GL_VERSION_1_5
#define GL_VERSION_1_5 1
#include <stdint.h>
typedef int32_t GLsizeiptr;
typedef int32_t GLintptr;
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
#define GL_VERTEX_ARRAY_BUFFER_BINDING    0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING    0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING     0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING     0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING    0x889E
#define GL_FOG_COORD_SRC                  0x8450
#define GL_FOG_COORD                      0x8451
#define GL_CURRENT_FOG_COORD              0x8453
#define GL_FOG_COORD_ARRAY_TYPE           0x8454
#define GL_FOG_COORD_ARRAY_STRIDE         0x8455
#define GL_FOG_COORD_ARRAY_POINTER        0x8456
#define GL_FOG_COORD_ARRAY                0x8457
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING 0x889D
#define GL_SRC0_RGB                       0x8580
#define GL_SRC1_RGB                       0x8581
#define GL_SRC2_RGB                       0x8582
#define GL_SRC0_ALPHA                     0x8588
#define GL_SRC2_ALPHA                     0x858A
typedef void(*PFNGLGENQUERIESPROC)(GLsizei n, GLuint *ids);
typedef void(*PFNGLDELETEQUERIESPROC)(GLsizei n, const GLuint *ids);
typedef GLboolean(*PFNGLISQUERYPROC)(GLuint id);
typedef void(*PFNGLBEGINQUERYPROC)(GLenum target, GLuint id);
typedef void(*PFNGLENDQUERYPROC)(GLenum target);
typedef void(*PFNGLGETQUERYIVPROC)(GLenum target, GLenum pname, GLint *params);
typedef void(*PFNGLGETQUERYOBJECTIVPROC)(GLuint id, GLenum pname, GLint *params);
typedef void(*PFNGLGETQUERYOBJECTUIVPROC)(GLuint id, GLenum pname, GLuint *params);
typedef void(*PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void(*PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void(*PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef GLboolean(*PFNGLISBUFFERPROC)(GLuint buffer);
typedef void(*PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void(*PFNGLBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void(*PFNGLGETBUFFERSUBDATAPROC)(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
typedef void *(*PFNGLMAPBUFFERPROC)(GLenum target, GLenum access);
typedef GLboolean(*PFNGLUNMAPBUFFERPROC)(GLenum target);
typedef void(*PFNGLGETBUFFERPARAMETERIVPROC)(GLenum target, GLenum pname, GLint *params);
typedef void(*PFNGLGETBUFFERPOINTERVPROC)(GLenum target, GLenum pname, void **params);
#ifdef GL_GLEXT_PROTOTYPES
// GL_API void glGenQueries(GLsizei n, GLuint *ids);
// GL_API void glDeleteQueries(GLsizei n, const GLuint *ids);
// GL_API GLboolean glIsQuery(GLuint id);
// GL_API void glBeginQuery(GLenum target, GLuint id);
// GL_API void glEndQuery(GLenum target);
// GL_API void glGetQueryiv(GLenum target, GLenum pname, GLint *params);
// GL_API void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
// GL_API void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params);
// GL_API void glBindBuffer(GLenum target, GLuint buffer);
// GL_API void glDeleteBuffers(GLsizei n, const GLuint *buffers);
// GL_API void glGenBuffers(GLsizei n, GLuint *buffers);
// GL_API GLboolean glIsBuffer(GLuint buffer);
// GL_API void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
// GL_API void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
// GL_API void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
// GL_API void *glMapBuffer(GLenum target, GLenum access);
// GL_API GLboolean glUnmapBuffer(GLenum target);
// GL_API void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
// GL_API void glGetBufferPointerv(GLenum target, GLenum pname, void **params);
#endif
#endif /* GL_VERSION_1_5 */

#ifdef __cplusplus
}
#endif

#endif /* _gl_glext_h */
