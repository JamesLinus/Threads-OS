#ifndef THREAD_FIXED_POINT
#define THREAD_FIXED_POINT
/* to implement fixed point in the kernel */
#define EPS_F 16384
#define TO_FIXED(n) ((n) * 16384)
#define TO_INT_Z(x)	((x) / 16384)
#define TO_INT_NEAR(x) ((x)>=0?(((x) + 8192) / 16384) : (((x) - 8192) / 16384))
#define ADD_X_Y(x,y) ((x) + (y))
#define ADD_X_N(x,n) ((x) + (n) * 16384)
#define SUB_X_Y(x,y) ((x) - (y))
#define SUB_X_N(x,n) ((x) - (n) * 16384)
#define MUL_X_Y(x,y) (((int64_t) (x)) * (y) / 16384)
#define MUL_X_N(x,n) ((x) * (n))
#define DIV_X_Y(x,y) (((int64_t) (x)) * 16384 / (y))
#define DIV_X_N(x,n) ((x) / (n))

#endif