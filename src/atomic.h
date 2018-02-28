#ifndef _URCU_ARCH_UATOMIC_X86_H
#define _URCU_ARCH_UATOMIC_X86_H

struct __uatomic_dummy {
	unsigned long v[10];
};
#define __hp(x)	((struct __uatomic_dummy *)(x))


/* uatomic_inc */

static inline __attribute__((always_inline))
void __uatomic_inc(void *addr, int len)
{
	switch (len) {
	case 1:
	{
		__asm__ __volatile__(
		"lock; incb %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
	case 2:
	{
		__asm__ __volatile__(
		"lock; incw %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
	case 4:
	{
		__asm__ __volatile__(
		"lock; incl %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
#if (CAA_BITS_PER_LONG == 64)
	case 8:
	{
		__asm__ __volatile__(
		"lock; incq %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
#endif
	}
	/* generate an illegal instruction. Cannot catch this with linker tricks
	 * when optimizations are disabled. */
	__asm__ __volatile__("ud2");
	return;
}

#define _uatomic_inc(addr)	(__uatomic_inc((addr), sizeof(*(addr))))

/* uatomic_dec */

static inline __attribute__((always_inline))
void __uatomic_dec(void *addr, int len)
{
	switch (len) {
	case 1:
	{
		__asm__ __volatile__(
		"lock; decb %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
	case 2:
	{
		__asm__ __volatile__(
		"lock; decw %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
	case 4:
	{
		__asm__ __volatile__(
		"lock; decl %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
#if (CAA_BITS_PER_LONG == 64)
	case 8:
	{
		__asm__ __volatile__(
		"lock; decq %0"
			: "=m"(*__hp(addr))
			:
			: "memory");
		return;
	}
#endif
	}
	/*
	 * generate an illegal instruction. Cannot catch this with
	 * linker tricks when optimizations are disabled.
	 */
	__asm__ __volatile__("ud2");
	return;
}

#define _uatomic_dec(addr)	(__uatomic_dec((addr), sizeof(*(addr))))


#endif /* _URCU_ARCH_UATOMIC_X86_H */
