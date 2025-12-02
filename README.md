# pbGL

pbGL is a partial implementation of OpenGL 1.x for the Xbox using NXDK and PBKit. It's partially based on [xgu-gl](https://github.com/JayFoxRox/xgu-gl) by JayFoxRox.

# Status

Currently a subset of GL1.2 with some extensions from later versions is implemented.

To see what exactly is or isn't implemented, look at [the GL headers](include/GL/gl.h).

Extension list:
* `GL_ARB_multitexture` (4 texture units available)
* `GL_ARB_texture_env_combine`
* `GL_ARB_texture_env_add`
* `GL_PBGL_texture_generate_mipmap` (provides `glGenerateMipmap` and `GL_GENERATE_MIPMAP`)

Known limitations:
* pbGL assumes that the color buffer is RGBA8888 and the depth buffer is D24S8;
* `glVertexPointer` etc do not make copies of the vertex data;
* `GL_TEXTURE_1D`, `GL_TEXTURE_3D`, `GL_TEXTURE_CUBE_MAP` are not supported yet;
* `glTexImage2D` etc only convert RGB888 <-> RGBA8888, otherwise you have to use the same format;
* GL lighting and material system is not fully implemented;
* maximum theoretical texture size is 4096x4096;
* NPOT textures are not supported and will likely explode;
* supported texture formats:
  * `RGB888`, `RGBA8888`
  * `ALPHA8`, `LUMINANCE8`, `LUMINANCE8_ALPHA8` (untested, no format conversion)
  * `RGBA4444`, `RGB555`, `RGBA5551` (untested, no format conversion)

# Usage

Link your project to `libpbgl.lib` and add the `include` folder to your include directories.
If you are using the NXDK `Makefile` system, the easiest way to do this is something like this:
```
...
PBGL_DIR := path/to/pbgl

include $(NXDK_DIR)/Makefile
include $(PBGL_DIR)/config_pbgl.make
CFLAGS += $(PBGL_CFLAGS)
CXXFLAGS += $(PBGL_CFLAGS)

main.exe: $(PBGL_LIB)
```
You should then be able to use pbGL like any other GL implementation/loader, barring initialization and buffer swapping.
Do keep in mind that you still have to set the display resolution before calling `pbgl_init()`.

```
// define this to get GL function prototypes
#define GL_GLEXT_PROTOTYPES
#include <pbgl.h>
#include <GL/gl.h>
#include <GL/glext.h>
...
// argument indicates whether or not to init PBKit as well
pbgl_init(GL_TRUE);
// you're free to use GL functions after this
...
glClearColor(1.f, 0.f, 0.f, 1.f);
glClear(GL_COLOR_BUFFER_BIT);
// don't forget to swap buffers when you're done with the frame
pbgl_swap_buffers();
...
pbgl_shutdown(); // this will also shutdown PBKit
```

See the [samples](https://github.com/fgsfdsfgs/pbgl-samples) repository for more in-depth usage examples.

# Credits
* NXDK authors and contributors for NXDK and the libraries included in it
* mborgerson for xsm64, which taught me a lot about PBKit
* xemu authors and abaire for figuring out how the GPU works
* dracc for XGU
* JayFoxRox for xgu-gl
* Ryzee120 for figuring out the Z bug
* XboxDev Discord server for help
