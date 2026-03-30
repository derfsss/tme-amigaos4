/*
 * Minimal GLES2 platform header for AmigaOS4 axmol build.
 *
 * Replaces the SDK's gl2platform.h which routes everything through
 * ogles2.library's interface system. This version provides standard
 * Khronos defines so gl2.h can declare functions normally.
 * Functions are resolved at link time via libGLESv2.a.
 */
#ifndef __gl2platform_h_
#define __gl2platform_h_ 1

#include <KHR/khrplatform.h>

#ifndef GL_APICALL
#define GL_APICALL
#endif

#ifndef GL_APIENTRY
#define GL_APIENTRY
#endif

/* Enable function prototype declarations in gl2.h */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif

#endif /* __gl2platform_h_ */
