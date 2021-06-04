#include <stdint.h>


typedef void * RsAllocation;
typedef void * RsContext;
typedef void * RsDevice;
typedef void * RsElement;
typedef void * RsSampler;
typedef void * RsScript;
typedef void * RsMesh;
typedef void * RsType;
typedef void * RsProgramFragment;
typedef void * RsProgramStore;

typedef struct {
    float m[16];
} rsc_Matrix;


typedef struct {
    float v[4];
} rsc_Vector4;

#define RS_PROGRAM_VERTEX_MODELVIEW_OFFSET 0
#define RS_PROGRAM_VERTEX_PROJECTION_OFFSET 16
#define RS_PROGRAM_VERTEX_TEXTURE_OFFSET 32
#define RS_PROGRAM_VERTEX_MVP_OFFSET 48

#define RS_KERNEL_MAX_ARGUMENTS 256
#define RS_CLOSURE_MAX_NUMBER_ARGS_AND_BINDINGS 1024
#define RS_SCRIPT_GROUP_MAX_NUMBER_CLOSURES (2<<20)
