/*
 * GLAD replacement for AmigaOS4 — uses GLES2 headers + libGLESv2.a
 *
 * GL functions are declared by our clean GLES2/gl2.h (standard Khronos)
 * and resolved at link time against libGLESv2.a. No runtime loading needed.
 */
#ifndef GLAD_GL_H_AMIGAOS4_SHIM
#define GLAD_GL_H_AMIGAOS4_SHIM

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

/* GLAD compatibility stubs — axmol calls these during init */
#define GLAD_VERSION_MAJOR 2
#define GLAD_VERSION_MINOR 0
#define GLAD_GL 1

typedef void* (*GLADloadproc)(const char *name);
static inline int gladLoadGL(GLADloadproc loader) { (void)loader; return 1; }
static inline int gladLoadGLES2(GLADloadproc loader) { (void)loader; return 1; }
static inline int gladLoaderLoadGLES2(void) { return 1; }

#endif /* GLAD_GL_H_AMIGAOS4_SHIM */
