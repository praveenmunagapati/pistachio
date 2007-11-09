/*********************************************************************
 *                
 * Copyright (C) 2002-2003, 2007,  Karlsruhe University
 *                
 * File path:     arch/x86/x32/trapgate.h
 * Description:   defines macros for implementation of trap and 
 *		  interrupt gates in C
 *                
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *                
 * $Id: trapgate.h,v 1.8 2003/09/24 19:04:27 skoglund Exp $
 *                
 ********************************************************************/
#ifndef __ARCH__X86__X32__TRAPGATE_H__
#define __ARCH__X86__X32__TRAPGATE_H__

class x86_exceptionframe_t;

class x86_exceptionregs_t
{
public:
    static const word_t		num_regs = 17;
    
    enum reg_e {
	 esreg =  1,	 dsreg =  2,	 Dreg  =  3, 	 Sreg  =  4,  
	 Breg  =  5,  	 breg  =  7,  	 dreg  =  8,	 creg  =  9, 
	 areg  = 10, 	 ereg  = 11,	 ipreg = 12, 	 csreg = 13, 
	 freg  = 14,	 spreg = 15,	 ssreg = 16, 
    };
    union
    {
	struct
	{    
	    u32_t reason;		/*  0 */
	    u32_t es;			/*  1 */
	    u32_t ds;			/*  2 */
	    u32_t edi;			/*  3 */
	    u32_t esi;			/*  4 */
	    u32_t ebp;			/*  5 */
	    u32_t __esp;		        
	    u32_t ebx;			/*  7 */
	    u32_t edx;			/*  8 */
	    u32_t ecx;			/*  9 */
	    u32_t eax;			/* 10 */
	    /* default trapgate frame */	
	    u32_t error;		/* 11 */
	    u32_t eip;			/* 12 */
	    u32_t cs;			/* 13 */
	    u32_t eflags;		/* 14 */
	    u32_t esp;			/* 15 */
	    u32_t ss;			/* 16 */
	};
	word_t			regs[num_regs];
    };

#if defined(CONFIG_DEBUG)
    static const word_t		num_dbgregs = 12;
#endif
    
};

/*
 * If KEEP_LAST_BRANCHES is enabled we should clear the LBR flag prior
 * to handling the exception so as to avoid overwriting the last
 * branch records by regular kernel code.
 */
#if defined(CONFIG_X86_KEEP_LAST_BRANCHES)
#define clr_dbgctl_lbr()		\
	"mov	$0x1d9, %%ecx		\n"\
	"rdmsr				\n"\
	"andl	$0xfffffffe, %%eax	\n"\
	"wrmsr				\n"

#define set_dbgctl_lbr()		\
	"mov	$0x1d9, %%ecx		\n"\
	"rdmsr				\n"\
	"orl	$0x1, %%eax		\n"\
	"wrmsr				\n"
#else
#define clr_dbgctl_lbr()
#define set_dbgctl_lbr()
#endif

#if defined(CONFIG_X86_SMALL_SPACES)
#define set_kds(reg)						\
	"mov $" MKSTR(X86_KDS) ", %%" #reg "	\n"		\
	"mov %%" #reg ", %%ds	    		\n"
#else
#define set_kds(reg)
#endif


/**
 * X86_EXCWITH_ERRORCODE: allows C implementation of 
 *   exception handlers and trap/interrupt gates with error 
 *   code.
 *
 * Usage: X86_EXCWITH_ERRORCODE(exc_gp)
 */
#define X86_EXCWITH_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(x86_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"pusha				\n"			\
	"push	%%ds			\n"			\
	"push	%%es			\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	set_kds (eax)						\
	clr_dbgctl_lbr ()					\
	"call	%0			\n"			\
	set_dbgctl_lbr ()					\
	"addl	$8, %%esp		\n"			\
	"popl	%%es			\n"			\
	"popl	%%ds			\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(x86_exceptionframe_t * frame)




#define X86_EXCNO_ERRORCODE(name, reason)			\
extern "C" void name (void);					\
static void name##handler(x86_exceptionframe_t * frame);	\
void name##_wrapper()						\
{								\
    __asm__ (							\
        ".global "#name "		\n"			\
	"\t.type "#name",@function	\n"			\
	#name":				\n"			\
	"subl	$4, %%esp		\n"			\
	"pusha				\n"			\
	"push	%%ds			\n"			\
	"push	%%es			\n"			\
	"push	%1			\n"			\
	"push	%%esp			\n"			\
	set_kds (eax)						\
	clr_dbgctl_lbr ()					\
	"call	%0			\n"			\
	set_dbgctl_lbr ()					\
	"addl	$8, %%esp		\n"			\
	"popl	%%es			\n"			\
	"popl	%%ds			\n"			\
	"popa				\n"			\
	"addl	$4, %%esp		\n"			\
	"iret				\n"			\
	:							\
	: "m"(*(u32_t*)name##handler), "i"(reason)		\
	);							\
}								\
static void name##handler(x86_exceptionframe_t * frame)


#endif /* !__ARCH__X86__X32__TRAPGATE_H__ */