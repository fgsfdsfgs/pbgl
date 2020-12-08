# pbGL

pbGL is a partial implementation of OpenGL 1.x for the Xbox using NXDK and PBKit. It's partially based on [xgu-gl](https://github.com/JayFoxRox/xgu-gl) by JayFoxRox.

# Status

Currently a subset of GL1.1 with some extensions from later versions is implemented.

To see what exactly is or isn't implemented, look at [the GL headers](include/GL/gl.h).

Extension list:
* `GL_ARB_multitexture`
* `GL_ARB_texture_env_combine`

Do note that NPOT textures are not supported and will probably explode.

# Usage

Link your project to `libpbgl.lib` and add the `include` folder to your include directories.
You should then be able to use pbGL like any other GL implementation/loader, barring initialization and buffer swapping.
Do keep in mind that you still have to set the display resolution before calling `pbgl_init()`.

```
// define this to get GL function prototypes 
// or use pbgl_get_proc_address() (not yet implemented)
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
* dracc for XGU
* JayFoxRox for xgu-gl
