#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef DJGPP

static inline unsigned int BSF(unsigned int x)
{
	unsigned int i;
	asm volatile("bsf %1, %0" : "=r" (i) : "rm" (x));
	return i;
}
//static inline unsigned long long rdtsc(void)
//{
//	return __builtin_ia32_rdtsc();
//}
#define rdtsc() __builtin_ia32_rdtsc()
//static inline void fpu_save(void * buffer) { }
//static inline void fpu_restore(void * buffer) { }
#ifdef PENTIUM4
static unsigned int defmxcsr = 0x1f80;
#define fpu_save(buffer) asm volatile("fxsave %0\n\tfninit\n\tldmxcsr %1" : "=m"((buffer)) : "m"(defmxcsr))
#define fpu_restore(buffer) asm volatile("fxrstor %0" :: "m"(buffer))
#else
#define fpu_save(buffer) asm volatile("fnsave %0" : "=m"((buffer)))
#define fpu_restore(buffer) asm volatile("frstor %0" :: "m"(buffer))
#endif
//static inline void _disable_ints(void) { asm("mov $0x900, %%ax\n\t" "int $0x31" ::: "eax" ); }
//static inline void  _enable_ints(void) { asm("mov $0x901, %%ax\n\t" "int $0x31" ::: "eax" ); }
#define _disable_ints() asm volatile("mov $0x900, %%ax\n\t" "int $0x31" ::: "eax" )
#define _enable_ints() asm volatile("mov $0x901, %%ax\n\t" "int $0x31" ::: "eax" )

#define _my_psp() _go32_info_block.linear_address_of_original_psp
#define _my_rmstack() _go32_info_block.linear_address_of_transfer_buffer
#define _my_rmstksiz() _go32_info_block.size_of_transfer_buffer

#else

unsigned int BSF(unsigned int);
#pragma aux BSF = \
        "bsf eax, edx" \
        parm [edx] \
        modify exact [eax edx]

static  unsigned long long rdtsc(void);
#pragma aux rdtsc = \
        "rdtsc" \
        parm[]  \
        modify exact[eax edx]

void fpu_save(void *buffer);
#pragma aux fpu_save = \
        "fnsave [edx]" \
        parm [edx] \
        modify exact [edx]

void fpu_restore(void *buffer);
#pragma aux fpu_restore = \
        "frstor [edx]" \
        parm [edx] \
        modify exact [edx]

void _disable_ints(void);
#pragma aux _disable_ints = \
        "mov ax, 900h" \
        "int 31h" \
        parm[] \
        modify exact [ax]

void  _enable_ints(void);
#pragma aux _enable_ints = \
        "mov ax, 901h" \
        "int 31h" \
        parm[] \
        modify exact [ax]

unsigned int _my_ds();
#pragma aux _my_ds = \
        "mov eax, ds" \
        parm [] \
        modify exact [ eax ]
extern unsigned int _linear_psp;
extern unsigned int _linear_rmstack;
//extern unsigned int _size_rmstack;
#define _my_psp()      _linear_psp
#define _my_rmstack()  _linear_rmstack
#define _my_rmstksiz() 0x400

#endif

//align down
#define align(x,a) ((uint32_t)((x)+(a)-1)&(uint32_t)(~((a)-1)))

#define countof(x) (sizeof(x)/sizeof(x[0]))

#undef min
#undef max
#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))

#define CPU_CFLAG 0x0001    //carry flag (CF)

#ifdef __cplusplus
}
#endif

#endif
