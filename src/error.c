#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "GL/gl.h"
#include "GL/glext.h"
#include "state.h"
#include "error.h"

/* PBGL INTERNAL FUNCTIONS BEGIN */

#ifdef PBGL_DEBUG

#include <windows.h>

#define DEBUG_LOGFILE "D:\\pbgl.log"

static FILE *logf = NULL;

void pbgl_debug_log(const char *fmt, ...) {
  static char msg[1024];

  va_list args;
  va_start(args, fmt);
  vsnprintf(msg, sizeof(msg), fmt, args);
  va_end(args);

  DbgPrint("PBGL: %s\n", msg);

  if (logf == NULL) {
    logf = fopen(DEBUG_LOGFILE, "w");
    if (!logf) return;
  }

  fprintf(logf, "PBGL: %s\n", msg);
  fflush(logf);
}

const char *pbgl_get_error_str(GLenum error) {
  #define ERR(e) case e: return #e;
  switch (error) {
    ERR(GL_INVALID_OPERATION)
    ERR(GL_INVALID_ENUM)
    ERR(GL_INVALID_VALUE)
    ERR(GL_STACK_OVERFLOW)
    ERR(GL_STACK_UNDERFLOW)
    ERR(GL_OUT_OF_MEMORY)
    ERR(GL_NO_ERROR)
    default: return "";
  }
  #undef ERR
}

#endif

void pbgl_set_gl_error(GLenum error, GLenum arg, const char *func, const int line) {
#ifdef PBGL_DEBUG
  pbgl_debug_log("%s (%d): error: %s (current: %s) arg: 0x%x", func, line, pbgl_get_error_str(error), pbgl_get_error_str(pbgl.error), arg);
#else
  (void)arg;
#endif
  // spec: can only set error when it's unset
  if (pbgl.error == GL_NO_ERROR)
    pbgl.error = error;
}

/* GL FUNCTIONS BEGIN */

GL_API GLenum glGetError(void) {
  const GLenum err = pbgl.error;
  pbgl.error = GL_NO_ERROR;
  return err;
}
