	.file	"copy_n_std_nsg_probe.cpp"
	.text
	.section	.rodata._ZNK5boost9container9exception4whatEv.str1.8,"aMS",@progbits,1
	.align 8
.LC0:
	.string	"unknown boost::container exception"
	.section	.text._ZNK5boost9container9exception4whatEv,"axG",@progbits,_ZNK5boost9container9exception4whatEv,comdat
	.align 2
	.p2align 4
	.weak	_ZNK5boost9container9exception4whatEv
	.type	_ZNK5boost9container9exception4whatEv, @function
_ZNK5boost9container9exception4whatEv:
.LFB219:
	.cfi_startproc
	endbr64
	movq	8(%rdi), %rax
	leaq	.LC0(%rip), %rdx
	testq	%rax, %rax
	cmove	%rdx, %rax
	ret
	.cfi_endproc
.LFE219:
	.size	_ZNK5boost9container9exception4whatEv, .-_ZNK5boost9container9exception4whatEv
	.section	.text._ZN5boost9container9exceptionD2Ev,"axG",@progbits,_ZN5boost9container9exceptionD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container9exceptionD2Ev
	.type	_ZN5boost9container9exceptionD2Ev, @function
_ZN5boost9container9exceptionD2Ev:
.LFB222:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	movq	%rax, (%rdi)
	jmp	_ZNSt9exceptionD2Ev@PLT
	.cfi_endproc
.LFE222:
	.size	_ZN5boost9container9exceptionD2Ev, .-_ZN5boost9container9exceptionD2Ev
	.weak	_ZN5boost9container9exceptionD1Ev
	.set	_ZN5boost9container9exceptionD1Ev,_ZN5boost9container9exceptionD2Ev
	.section	.text._ZN5boost9container9exceptionD0Ev,"axG",@progbits,_ZN5boost9container9exceptionD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container9exceptionD0Ev
	.type	_ZN5boost9container9exceptionD0Ev, @function
_ZN5boost9container9exceptionD0Ev:
.LFB224:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rax, (%rdi)
	movq	%rdi, 8(%rsp)
	call	_ZNSt9exceptionD2Ev@PLT
	movq	8(%rsp), %rdi
	movl	$16, %esi
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	jmp	_ZdlPvm@PLT
	.cfi_endproc
.LFE224:
	.size	_ZN5boost9container9exceptionD0Ev, .-_ZN5boost9container9exceptionD0Ev
	.section	.text._ZN5boost9container12length_errorD2Ev,"axG",@progbits,_ZN5boost9container12length_errorD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container12length_errorD2Ev
	.type	_ZN5boost9container12length_errorD2Ev, @function
_ZN5boost9container12length_errorD2Ev:
.LFB251:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	movq	%rax, (%rdi)
	jmp	_ZNSt9exceptionD2Ev@PLT
	.cfi_endproc
.LFE251:
	.size	_ZN5boost9container12length_errorD2Ev, .-_ZN5boost9container12length_errorD2Ev
	.weak	_ZN5boost9container12length_errorD1Ev
	.set	_ZN5boost9container12length_errorD1Ev,_ZN5boost9container12length_errorD2Ev
	.section	.text._ZN5boost9container12length_errorD0Ev,"axG",@progbits,_ZN5boost9container12length_errorD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container12length_errorD0Ev
	.type	_ZN5boost9container12length_errorD0Ev, @function
_ZN5boost9container12length_errorD0Ev:
.LFB253:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rax, (%rdi)
	movq	%rdi, 8(%rsp)
	call	_ZNSt9exceptionD2Ev@PLT
	movq	8(%rsp), %rdi
	movl	$16, %esi
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	jmp	_ZdlPvm@PLT
	.cfi_endproc
.LFE253:
	.size	_ZN5boost9container12length_errorD0Ev, .-_ZN5boost9container12length_errorD0Ev
	.section	.text._ZN5boost9container9bad_allocD2Ev,"axG",@progbits,_ZN5boost9container9bad_allocD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container9bad_allocD2Ev
	.type	_ZN5boost9container9bad_allocD2Ev, @function
_ZN5boost9container9bad_allocD2Ev:
.LFB241:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	movq	%rax, (%rdi)
	jmp	_ZNSt9exceptionD2Ev@PLT
	.cfi_endproc
.LFE241:
	.size	_ZN5boost9container9bad_allocD2Ev, .-_ZN5boost9container9bad_allocD2Ev
	.weak	_ZN5boost9container9bad_allocD1Ev
	.set	_ZN5boost9container9bad_allocD1Ev,_ZN5boost9container9bad_allocD2Ev
	.section	.text._ZN5boost9container9bad_allocD0Ev,"axG",@progbits,_ZN5boost9container9bad_allocD5Ev,comdat
	.align 2
	.p2align 4
	.weak	_ZN5boost9container9bad_allocD0Ev
	.type	_ZN5boost9container9bad_allocD0Ev, @function
_ZN5boost9container9bad_allocD0Ev:
.LFB243:
	.cfi_startproc
	endbr64
	leaq	16+_ZTVN5boost9container9exceptionE(%rip), %rax
	subq	$24, %rsp
	.cfi_def_cfa_offset 32
	movq	%rax, (%rdi)
	movq	%rdi, 8(%rsp)
	call	_ZNSt9exceptionD2Ev@PLT
	movq	8(%rsp), %rdi
	movl	$16, %esi
	addq	$24, %rsp
	.cfi_def_cfa_offset 8
	jmp	_ZdlPvm@PLT
	.cfi_endproc
.LFE243:
	.size	_ZN5boost9container9bad_allocD0Ev, .-_ZN5boost9container9bad_allocD0Ev
	.text
	.p2align 4
	.type	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0, @function
_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0:
.LFB5737:
	.cfi_startproc
	movq	%rsi, %rax
	subq	%rdi, %rax
	cmpq	$128, %rax
	jle	.L87
	subq	$56, %rsp
	.cfi_def_cfa_offset 64
	movq	%rsi, %r9
	movq	%r12, 24(%rsp)
	.cfi_offset 12, -40
	movq	%rax, %r12
	sarq	$4, %rax
	movq	%r13, 32(%rsp)
	.cfi_offset 13, -32
	movq	%rdx, %r13
	sarq	$3, %r12
	movq	%rbp, 16(%rsp)
	.cfi_offset 6, -48
	movq	%rdi, %rbp
	movq	%rbx, 8(%rsp)
	testq	%r13, %r13
	.cfi_offset 3, -56
	je	.L88
.L15:
	movdqu	0(%rbp), %xmm0
	movq	-8(%r9), %rsi
	subq	$1, %r13
	leaq	8(%rbp), %rbx
	leaq	0(%rbp,%rax,8), %rdi
	movhlps	%xmm0, %xmm2
	movq	(%rdi), %rax
	movdqa	%xmm0, %xmm1
	movq	%xmm0, %rdx
	movq	%xmm2, %rcx
	shufpd	$1, %xmm0, %xmm1
	cmpq	%rax, %rcx
	jnb	.L45
	cmpq	%rsi, %rax
	jb	.L51
	cmpq	%rsi, %rcx
	jb	.L85
.L84:
	movups	%xmm1, 0(%rbp)
.L47:
	movq	%r9, %rdi
	jmp	.L86
	.p2align 4
	.p2align 4,,10
	.p2align 3
.L53:
	movq	8(%rbx), %rdx
	addq	$8, %rbx
.L86:
	cmpq	%rcx, %rdx
	jb	.L53
	movq	-8(%rdi), %rsi
	cmpq	%rsi, %rcx
	jnb	.L54
	leaq	-16(%rdi), %rax
	.p2align 4
	.p2align 4
	.p2align 3
.L55:
	movq	%rax, %rdi
	movq	(%rax), %rsi
	subq	$8, %rax
	cmpq	%rsi, %rcx
	jb	.L55
	cmpq	%rdi, %rbx
	jnb	.L89
.L57:
	movq	%rsi, (%rbx)
	addq	$8, %rbx
	movq	%rdx, (%rdi)
	movq	(%rbx), %rdx
	movq	0(%rbp), %rcx
	jmp	.L86
.L45:
	cmpq	%rsi, %rcx
	jb	.L84
	cmpq	%rsi, %rax
	jnb	.L51
.L85:
	movq	%rsi, 0(%rbp)
	movq	%rdx, -8(%r9)
	movq	8(%rbp), %rdx
	movq	0(%rbp), %rcx
	jmp	.L47
	.p2align 4,,10
	.p2align 3
.L54:
	subq	$8, %rdi
	cmpq	%rdi, %rbx
	jb	.L57
	.p2align 4
	.p2align 3
.L89:
	movq	%r13, %rdx
	movq	%r9, %rsi
	movq	%rbx, %rdi
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	movq	%rbx, %rax
	subq	%rbp, %rax
	cmpq	$128, %rax
	jle	.L13
	movq	%rax, %r12
	movq	%rbx, %r9
	sarq	$4, %rax
	sarq	$3, %r12
	testq	%r13, %r13
	jne	.L15
.L88:
	leaq	-1(%r12), %r11
	leaq	-8(%rbp,%rax,8), %rbx
	movq	%r14, 40(%rsp)
	leaq	-1(%rax), %r8
	sarq	%r11
	movq	%r15, 48(%rsp)
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	movq	(%rbx), %r10
	movq	%r8, %r13
	movq	%rbx, %rcx
	cmpq	%r11, %r8
	jge	.L90
.L25:
	movq	%r8, %rsi
	jmp	.L18
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L62:
	movq	%rax, %rsi
.L18:
	leaq	1(%rsi), %rdx
	leaq	(%rdx,%rdx), %rdi
	salq	$4, %rdx
	leaq	-1(%rdi), %rax
	addq	%rbp, %rdx
	leaq	0(%rbp,%rax,8), %rcx
	movq	(%rdx), %r15
	movq	(%rcx), %r14
	cmpq	%r14, %r15
	cmovnb	%r15, %r14
	cmovnb	%rdi, %rax
	cmovnb	%rdx, %rcx
	movq	%r14, 0(%rbp,%rsi,8)
	cmpq	%rax, %r11
	jg	.L62
	testb	$1, %r12b
	jne	.L80
	cmpq	%rax, %r13
	je	.L21
.L80:
	leaq	-1(%rax), %rdx
.L20:
	sarq	%rdx
	cmpq	%rax, %r8
	jl	.L24
	jmp	.L22
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L92:
	movq	%rcx, (%rax)
	leaq	-1(%rdx), %rax
	shrq	$63, %rax
	leaq	-1(%rax,%rdx), %rcx
	movq	%rdx, %rax
	cmpq	%rdx, %r8
	jge	.L91
	movq	%rcx, %rdx
	sarq	%rdx
.L24:
	leaq	0(%rbp,%rdx,8), %rsi
	leaq	0(%rbp,%rax,8), %rax
	movq	(%rsi), %rcx
	cmpq	%r10, %rcx
	jb	.L92
.L23:
	movq	%r10, (%rax)
	testq	%r8, %r8
	je	.L28
.L27:
	subq	$8, %rbx
	subq	$1, %r8
	movq	(%rbx), %r10
	movq	%rbx, %rcx
	cmpq	%r11, %r8
	jl	.L25
.L90:
	testb	$1, %r12b
	jne	.L81
	cmpq	%r13, %r8
	je	.L93
.L22:
	movq	%r10, (%rcx)
	jmp	.L27
	.p2align 4,,10
	.p2align 3
.L51:
	.cfi_restore 14
	.cfi_restore 15
	movq	%rax, 0(%rbp)
	movq	%rdx, (%rdi)
	movq	8(%rbp), %rdx
	movq	0(%rbp), %rcx
	jmp	.L47
.L79:
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	movq	40(%rsp), %r14
	.cfi_restore 14
	movq	48(%rsp), %r15
	.cfi_restore 15
.L13:
	movq	8(%rsp), %rbx
	movq	16(%rsp), %rbp
	movq	24(%rsp), %r12
	movq	32(%rsp), %r13
	addq	$56, %rsp
	.cfi_restore 3
	.cfi_restore 6
	.cfi_restore 12
	.cfi_restore 13
	.cfi_def_cfa_offset 8
	ret
.L28:
	.cfi_def_cfa_offset 64
	.cfi_offset 3, -56
	.cfi_offset 6, -48
	.cfi_offset 12, -40
	.cfi_offset 13, -32
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	movq	%r9, %rax
	subq	%rbp, %rax
	cmpq	$8, %rax
	jle	.L79
	leaq	-8(%r9), %r8
	leaq	-8(%rax), %rdi
.L44:
	movq	0(%rbp), %rax
	movq	%rdi, %r13
	movq	(%r8), %rsi
	sarq	$3, %r13
	movq	%rax, (%r8)
	cmpq	$16, %rdi
	jle	.L31
	leaq	-1(%r13), %r12
	xorl	%ebx, %ebx
	sarq	%r12
	jmp	.L33
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L66:
	movq	%rax, %rbx
.L33:
	leaq	1(%rbx), %rdx
	leaq	(%rdx,%rdx), %r10
	salq	$4, %rdx
	leaq	-1(%r10), %rax
	addq	%rbp, %rdx
	leaq	0(%rbp,%rax,8), %r9
	movq	(%rdx), %r11
	movq	(%r9), %rcx
	cmpq	%rcx, %r11
	cmovnb	%r11, %rcx
	cmovnb	%r10, %rax
	cmovnb	%rdx, %r9
	movq	%rcx, 0(%rbp,%rbx,8)
	cmpq	%rax, %r12
	jg	.L66
	andl	$1, %r13d
	jne	.L83
	movq	%rdi, %rdx
	sarq	$4, %rdx
	subq	$1, %rdx
	cmpq	%rax, %rdx
	je	.L39
.L83:
	leaq	-1(%rax), %rdx
	shrq	$63, %rdx
	leaq	-1(%rax,%rdx), %rdx
	sarq	%rdx
	testq	%rax, %rax
	jne	.L43
	jmp	.L94
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L96:
	movq	%rcx, (%rax)
	leaq	-1(%rdx), %rax
	shrq	$63, %rax
	leaq	-1(%rax,%rdx), %rcx
	movq	%rdx, %rax
	testq	%rdx, %rdx
	je	.L95
	sarq	%rcx
	movq	%rcx, %rdx
.L43:
	leaq	0(%rbp,%rdx,8), %r9
	leaq	0(%rbp,%rax,8), %rax
	movq	(%r9), %rcx
	cmpq	%rsi, %rcx
	jb	.L96
.L40:
	movq	%rsi, (%rax)
	cmpq	$8, %rdi
	jle	.L79
.L82:
	subq	$8, %r8
	subq	$8, %rdi
	jmp	.L44
.L95:
	movq	%r9, %rax
	jmp	.L40
.L91:
	movq	%rsi, %rax
	jmp	.L23
.L31:
	andl	$1, %r13d
	jne	.L97
	movq	%rbp, %rax
	cmpq	$16, %rdi
	jne	.L40
	movq	%rbp, %r9
	xorl	%eax, %eax
.L39:
	leaq	1(%rax,%rax), %rcx
	movq	0(%rbp,%rcx,8), %rdx
	movq	%rdx, (%r9)
	movq	%rax, %rdx
	movq	%rcx, %rax
	jmp	.L43
.L93:
	movq	%r8, %rax
.L21:
	leaq	(%rax,%rax), %rdx
	leaq	1(%rdx), %rax
	leaq	0(%rbp,%rax,8), %rsi
	movq	(%rsi), %rdi
	movq	%rdi, (%rcx)
	movq	%rsi, %rcx
	jmp	.L20
.L97:
	movq	40(%rsp), %r14
	.cfi_remember_state
	.cfi_restore 14
	movq	48(%rsp), %r15
	.cfi_restore 15
	movq	%rsi, 0(%rbp)
	jmp	.L13
.L98:
	.cfi_restore_state
	subq	$2, %r8
	movq	-16(%rax), %r10
	leaq	-16(%rax), %rbx
	cmpq	%r8, %r11
	jg	.L25
.L81:
	movq	%r10, (%rbx)
	movq	%rbx, %rax
	leaq	-1(%r8), %rdx
	subq	$8, %rbx
	movq	(%rbx), %r10
	cmpq	%r11, %rdx
	jge	.L98
	movq	%rdx, %r8
	jmp	.L25
.L87:
	.cfi_def_cfa_offset 8
	.cfi_restore 3
	.cfi_restore 6
	.cfi_restore 12
	.cfi_restore 13
	.cfi_restore 14
	.cfi_restore 15
	ret
.L94:
	.cfi_def_cfa_offset 64
	.cfi_offset 3, -56
	.cfi_offset 6, -48
	.cfi_offset 12, -40
	.cfi_offset 13, -32
	.cfi_offset 14, -24
	.cfi_offset 15, -16
	movq	%rsi, (%r9)
	jmp	.L82
	.cfi_endproc
.LFE5737:
	.size	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0, .-_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	.section	.text.unlikely._ZN5boost9container18throw_length_errorEPKc,"axG",@progbits,_ZN5boost9container18throw_length_errorEPKc,comdat
	.weak	_ZN5boost9container18throw_length_errorEPKc
	.type	_ZN5boost9container18throw_length_errorEPKc, @function
_ZN5boost9container18throw_length_errorEPKc:
.LFB249:
	.cfi_startproc
	endbr64
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
	movl	$16, %edi
	call	__cxa_allocate_exception@PLT
	leaq	16+_ZTVN5boost9container12length_errorE(%rip), %rcx
	leaq	_ZN5boost9container12length_errorD1Ev(%rip), %rdx
	movq	%rbx, 8(%rax)
	leaq	_ZTIN5boost9container12length_errorE(%rip), %rsi
	movq	%rax, %rdi
	movq	%rcx, (%rax)
	call	__cxa_throw@PLT
	.cfi_endproc
.LFE249:
	.size	_ZN5boost9container18throw_length_errorEPKc, .-_ZN5boost9container18throw_length_errorEPKc
	.text
	.p2align 4
	.globl	_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.type	_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, @function
_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl:
.LFB4769:
	.cfi_startproc
	endbr64
	movq	16(%rdi), %rcx
	movq	(%rdi), %rax
	movq	%rsi, %r8
	movq	%rcx, %rsi
	shrq	$7, %rsi
	leaq	(%rax,%rsi,8), %r9
	xorl	%eax, %eax
	testq	%r9, %r9
	je	.L102
	movq	(%r9), %rax
	andl	$127, %ecx
	leaq	(%rax,%rcx,4), %rax
.L102:
	testq	%rdx, %rdx
	jle	.L101
	movq	(%r8), %rdi
	movl	(%rax), %ecx
	movl	%ecx, (%rdi)
	cmpq	$1, %rdx
	je	.L101
	movq	(%r9), %rsi
	subq	$1, %rdx
	xorl	%ecx, %ecx
	leaq	508(%rsi), %r8
	jmp	.L104
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L105:
	movl	4(%rax), %esi
	movl	%esi, 4(%rdi,%rcx,4)
	addq	$1, %rcx
	cmpq	%rcx, %rdx
	je	.L101
	addq	$4, %rax
.L104:
	cmpq	%r8, %rax
	jne	.L105
	movq	8(%r9), %rax
	movl	(%rax), %esi
	movl	%esi, 4(%rdi,%rcx,4)
	addq	$1, %rcx
	cmpq	%rcx, %rdx
	je	.L113
	addq	$8, %r9
	leaq	508(%rax), %r8
	jmp	.L104
	.p2align 4,,10
	.p2align 3
.L101:
	ret
.L113:
	ret
	.cfi_endproc
.LFE4769:
	.size	_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, .-_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.p2align 4
	.globl	_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.type	_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, @function
_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl:
.LFB4770:
	.cfi_startproc
	endbr64
	movq	16(%rdi), %rcx
	movq	(%rdi), %rax
	movq	%rsi, %r8
	movq	%rcx, %rsi
	shrq	$7, %rsi
	leaq	(%rax,%rsi,8), %r9
	xorl	%eax, %eax
	testq	%r9, %r9
	je	.L115
	movq	(%r9), %rax
	andl	$127, %ecx
	leaq	(%rax,%rcx,4), %rax
.L115:
	testq	%rdx, %rdx
	jle	.L114
	movq	(%r8), %rdi
	movl	(%rax), %ecx
	movl	%ecx, (%rdi)
	cmpq	$1, %rdx
	je	.L114
	movq	(%r9), %rsi
	subq	$1, %rdx
	xorl	%ecx, %ecx
	leaq	508(%rsi), %r8
	jmp	.L117
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L118:
	movl	4(%rax), %esi
	movl	%esi, 4(%rdi,%rcx,4)
	addq	$1, %rcx
	cmpq	%rcx, %rdx
	je	.L114
	addq	$4, %rax
.L117:
	cmpq	%r8, %rax
	jne	.L118
	movq	8(%r9), %rax
	movl	(%rax), %esi
	movl	%esi, 4(%rdi,%rcx,4)
	addq	$1, %rcx
	cmpq	%rcx, %rdx
	je	.L126
	addq	$8, %r9
	leaq	508(%rax), %r8
	jmp	.L117
	.p2align 4,,10
	.p2align 3
.L114:
	ret
.L126:
	ret
	.cfi_endproc
.LFE4770:
	.size	_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, .-_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.p2align 4
	.globl	_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.type	_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, @function
_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl:
.LFB4771:
	.cfi_startproc
	endbr64
	movq	%rdi, %rcx
	movq	%rsi, %r9
	movq	%rdx, %rdi
	movq	16(%rcx), %rax
	movq	(%rcx), %rdx
	xorl	%ecx, %ecx
	movq	%rax, %rsi
	shrq	$7, %rsi
	leaq	(%rdx,%rsi,8), %r8
	testq	%r8, %r8
	je	.L128
	movq	(%r8), %rdx
	andl	$127, %eax
	leaq	(%rdx,%rax,4), %rcx
.L128:
	testq	%rdi, %rdi
	jle	.L152
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	(%r9), %rsi
	.p2align 4
	.p2align 3
.L135:
	movq	(%r8), %rax
	addq	$512, %rax
	subq	%rcx, %rax
	sarq	$2, %rax
	cmpq	%rdi, %rax
	cmovg	%rdi, %rax
	subq	%rax, %rdi
	testq	%rax, %rax
	jle	.L130
	movl	(%rcx), %edx
	movq	%rax, %r10
	leaq	4(%rsi), %rbx
	movl	%edx, (%rsi)
	subq	$1, %r10
	je	.L137
	leaq	-2(%rax), %rdx
	cmpq	$2, %rdx
	jle	.L131
	leaq	-4(%rsi), %rax
	subq	%rcx, %rax
	cmpq	$8, %rax
	jbe	.L131
	movq	%r10, %r11
	xorl	%eax, %eax
	shrq	$2, %r11
	movq	%r11, %r9
	salq	$4, %r9
	.p2align 5
	.p2align 4
	.p2align 3
.L132:
	movdqu	4(%rcx,%rax), %xmm0
	movups	%xmm0, 4(%rsi,%rax)
	addq	$16, %rax
	cmpq	%r9, %rax
	jne	.L132
	salq	$2, %r11
	cmpq	%r11, %r10
	je	.L133
	addq	%rax, %rcx
	subq	%r11, %r10
	movl	4(%rcx), %esi
	movl	%esi, (%rbx,%rax)
	cmpq	$1, %r10
	je	.L133
	movl	8(%rcx), %esi
	movl	%esi, 4(%rbx,%rax)
	cmpq	$2, %r10
	je	.L133
	movl	12(%rcx), %ecx
	movl	%ecx, 8(%rbx,%rax)
.L133:
	leaq	4(%rbx,%rdx,4), %rsi
.L130:
	testq	%rdi, %rdi
	je	.L127
	addq	$8, %r8
	movq	(%r8), %rcx
	jmp	.L135
	.p2align 4,,10
	.p2align 3
.L131:
	salq	$2, %r10
	xorl	%eax, %eax
	.p2align 5
	.p2align 4
	.p2align 3
.L134:
	movl	4(%rcx,%rax), %r9d
	movl	%r9d, 4(%rsi,%rax)
	addq	$4, %rax
	cmpq	%rax, %r10
	jne	.L134
	jmp	.L133
	.p2align 4,,10
	.p2align 3
.L127:
	popq	%rbx
	.cfi_remember_state
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L137:
	.cfi_restore_state
	movq	%rbx, %rsi
	jmp	.L130
.L152:
	.cfi_def_cfa_offset 8
	.cfi_restore 3
	ret
	.cfi_endproc
.LFE4771:
	.size	_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl, .-_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	.section	.rodata._Z5benchI8call_stdEdT_md.str1.1,"aMS",@progbits,1
.LC1:
	.string	"vector::_M_realloc_append"
	.section	.text._Z5benchI8call_stdEdT_md,"axG",@progbits,_Z5benchI8call_stdEdT_md,comdat
	.p2align 4
	.weak	_Z5benchI8call_stdEdT_md
	.type	_Z5benchI8call_stdEdT_md, @function
_Z5benchI8call_stdEdT_md:
.LFB5132:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA5132
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	xorl	%r12d, %r12d
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$104, %rsp
	.cfi_def_cfa_offset 160
	movq	%rdi, 48(%rsp)
	movq	176(%rsp), %rax
	movsd	%xmm0, 40(%rsp)
	movq	168(%rsp), %rsi
	movq	%rax, %rdx
	movq	%fs:40, %rdi
	movq	%rdi, 88(%rsp)
	movq	160(%rsp), %rdi
	movq	%rsi, 24(%rsp)
	movq	%rax, 16(%rsp)
	movq	%rdi, 8(%rsp)
	call	_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	movl	$32768, %edi
.LEHB0:
	call	_Znwm@PLT
.LEHE0:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	movq	%rax, %rbp
	leaq	32768(%rax), %r14
	call	clock_gettime@PLT
	movq	%rbp, %r13
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	movq	%rax, 32(%rsp)
	jmp	.L162
	.p2align 4,,10
	.p2align 3
.L241:
	movsd	40(%rsp), %xmm4
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	comisd	%xmm0, %xmm4
	jbe	.L240
.L162:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	movq	16(%rsp), %rdx
	movq	24(%rsp), %rsi
	movq	8(%rsp), %rdi
	movq	72(%rsp), %r15
	imulq	$1000000000, 64(%rsp), %rbx
	call	_Z6do_stdRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	subq	%r15, %rax
	addq	72(%rsp), %rax
	subq	%rbx, %rax
	movq	%rax, %rdx
	cmpq	%rbp, %r14
	je	.L156
	movq	%rax, 0(%rbp)
	addq	$8, %rbp
.L157:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	addq	$1, %r12
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	subq	32(%rsp), %rax
	jns	.L241
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	movsd	40(%rsp), %xmm4
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	comisd	%xmm0, %xmm4
	ja	.L162
.L240:
	cmpq	%r13, %rbp
	je	.L165
	movq	%rbp, %rbx
	leaq	8(%r13), %rcx
	subq	%r13, %rbx
	movq	%rcx, 8(%rsp)
	movq	%rbx, %r15
	sarq	$3, %r15
	je	.L242
	bsrq	%r15, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	addq	%rdx, %rdx
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	cmpq	$128, %rbx
	movq	8(%rsp), %rcx
	jle	.L169
	leaq	128(%r13), %rdi
	movl	$8, %r9d
	jmp	.L179
	.p2align 4,,10
	.p2align 3
.L244:
	movl	$8, %eax
	subq	%r9, %rax
	addq	%rcx, %rax
	cmpq	$8, %r9
	je	.L171
	cmpl	$32, %r9d
	ja	.L173
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm1
	movdqu	-16(%r13,%rdx), %xmm0
	movups	%xmm1, (%rax)
	movups	%xmm0, -16(%rax,%rdx)
.L175:
	addq	$8, %rcx
	movq	%rsi, 0(%r13)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	je	.L243
.L179:
	movq	(%rcx), %rsi
	movq	0(%r13), %rdx
	movq	%rcx, %r8
	cmpq	%rdx, %rsi
	jb	.L244
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %rsi
	jnb	.L177
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L178:
	movq	%rdx, 8(%rax)
	movq	%rax, %r8
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rsi
	jb	.L178
.L177:
	addq	$8, %rcx
	movq	%rsi, (%r8)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	jne	.L179
.L243:
	cmpq	%rbp, %rdi
	je	.L180
	.p2align 4
	.p2align 3
.L183:
	movq	(%rdi), %rcx
	movq	-8(%rdi), %rdx
	cmpq	%rdx, %rcx
	jnb	.L212
	leaq	-8(%rdi), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L182:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rcx
	jb	.L182
.L181:
	addq	$8, %rdi
	movq	%rcx, (%rsi)
	cmpq	%rdi, %rbp
	jne	.L183
.L180:
	movabsq	$-3689348814741910323, %rax
	mulq	%r15
	shrq	$4, %rdx
	leaq	(%rdx,%rdx), %rax
	cmpq	%r15, %rax
	jb	.L209
	movq	%r15, %rcx
	xorl	%edx, %edx
.L192:
	movq	%rcx, %rax
	shrq	%rax
	addq	%rax, %rdx
	andl	$1, %ecx
	movq	0(%r13,%rdx,8), %rax
	jne	.L193
.L210:
	addq	-8(%r13,%rdx,8), %rax
	shrq	%rax
.L193:
	imulq	%r15, %rax
	testq	%rax, %rax
	js	.L194
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L195:
	testq	%r12, %r12
	js	.L196
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
.L197:
	movq	48(%rsp), %rax
	testq	%rax, %rax
	js	.L198
	pxor	%xmm2, %xmm2
	cvtsi2sdq	%rax, %xmm2
.L199:
	mulsd	%xmm2, %xmm1
	divsd	%xmm1, %xmm0
	testq	%r13, %r13
	je	.L155
	movq	%r14, %rsi
	subq	%r13, %rsi
	jmp	.L208
	.p2align 4,,10
	.p2align 3
.L156:
	movabsq	$1152921504606846975, %rax
	movq	%r14, %rbp
	subq	%r13, %rbp
	movq	%rbp, %rcx
	sarq	$3, %rcx
	cmpq	%rax, %rcx
	je	.L245
	testq	%rcx, %rcx
	movl	$1, %eax
	movq	%rdx, 56(%rsp)
	cmovne	%rcx, %rax
	addq	%rcx, %rax
	movabsq	$1152921504606846975, %rcx
	cmpq	%rcx, %rax
	cmova	%rcx, %rax
	leaq	0(,%rax,8), %rbx
	movq	%rbx, %rdi
.LEHB1:
	call	_Znwm@PLT
	movq	56(%rsp), %rdx
	movq	%rax, %rcx
	movq	%rdx, (%rax,%rbp)
	testq	%rbp, %rbp
	jne	.L246
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	je	.L161
.L247:
	movq	%r14, %rsi
	movq	%r13, %rdi
	movq	%rcx, 56(%rsp)
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	movq	56(%rsp), %rcx
.L161:
	leaq	(%rcx,%rbx), %r14
	movq	%rcx, %r13
	jmp	.L157
	.p2align 4,,10
	.p2align 3
.L246:
	movq	%rbp, %rdx
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	memcpy@PLT
	movq	%rax, %rcx
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	jne	.L247
	jmp	.L161
	.p2align 4,,10
	.p2align 3
.L242:
	movq	$-2, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	movq	8(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L248
.L167:
	movl	$8, %ebx
	jmp	.L190
	.p2align 4,,10
	.p2align 3
.L250:
	movl	$8, %edi
	subq	%rbx, %rdi
	addq	%rcx, %rdi
	cmpq	$8, %rbx
	jle	.L185
	movq	%rbx, %rdx
	movq	%r13, %rsi
	movq	%rcx, 16(%rsp)
	movq	%r8, 8(%rsp)
	call	memmove@PLT
	movq	8(%rsp), %r8
	movq	16(%rsp), %rcx
.L186:
	addq	$8, %rcx
	movq	%r8, 0(%r13)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	je	.L249
.L190:
	movq	(%rcx), %r8
	movq	0(%r13), %rax
	cmpq	%rax, %r8
	jb	.L250
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %r8
	jnb	.L213
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L189:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %r8
	jb	.L189
.L188:
	addq	$8, %rcx
	movq	%r8, (%rsi)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	jne	.L190
.L249:
	xorl	%eax, %eax
	xorl	%edx, %edx
.L209:
	movq	%r15, %rcx
	subq	%rax, %rcx
	jmp	.L192
	.p2align 4,,10
	.p2align 3
.L173:
	cmpl	$64, %r9d
	jb	.L174
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm7
	movdqu	16(%r13), %xmm6
	leaq	0(%r13,%rdx), %r8
	movdqu	32(%r13), %xmm5
	movdqu	48(%r13), %xmm4
	movdqu	-16(%r8), %xmm3
	movdqu	-32(%r8), %xmm2
	movdqu	-48(%r8), %xmm1
	movdqu	-64(%r8), %xmm0
	movups	%xmm7, (%rax)
	movups	%xmm6, 16(%rax)
	movups	%xmm5, 32(%rax)
	movups	%xmm4, 48(%rax)
	movups	%xmm3, -16(%rax,%rdx)
	movups	%xmm2, -32(%rax,%rdx)
	movups	%xmm1, -48(%rax,%rdx)
	movups	%xmm0, -64(%rax,%rdx)
	jmp	.L175
	.p2align 4,,10
	.p2align 3
.L165:
	testq	%r12, %r12
	js	.L204
	movq	48(%rsp), %rax
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
	testq	%rax, %rax
	js	.L206
.L251:
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L207:
	mulsd	%xmm0, %xmm1
	pxor	%xmm0, %xmm0
	movq	%r14, %rsi
	subq	%r13, %rsi
	divsd	%xmm1, %xmm0
.L208:
	movq	%r13, %rdi
	movsd	%xmm0, 8(%rsp)
	call	_ZdlPvm@PLT
	movsd	8(%rsp), %xmm0
.L155:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L239
	addq	$104, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L174:
	.cfi_restore_state
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm3
	movdqu	16(%r13), %xmm2
	leaq	0(%r13,%rdx), %r8
	movdqu	-16(%r8), %xmm1
	movdqu	-32(%r8), %xmm0
	movups	%xmm3, (%rax)
	movups	%xmm2, 16(%rax)
	movups	%xmm1, -16(%rax,%rdx)
	movups	%xmm0, -32(%rax,%rdx)
	jmp	.L175
.L204:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	movq	48(%rsp), %rax
	addsd	%xmm1, %xmm1
	testq	%rax, %rax
	jns	.L251
.L206:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm0, %xmm0
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L207
	.p2align 4,,10
	.p2align 3
.L198:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm2, %xmm2
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm2
	addsd	%xmm2, %xmm2
	jmp	.L199
	.p2align 4,,10
	.p2align 3
.L196:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	addsd	%xmm1, %xmm1
	jmp	.L197
	.p2align 4,,10
	.p2align 3
.L194:
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L195
	.p2align 4,,10
	.p2align 3
.L212:
	movq	%rdi, %rsi
	jmp	.L181
.L185:
	jne	.L186
	movq	%rax, (%rdi)
	jmp	.L186
.L213:
	movq	%rcx, %rsi
	jmp	.L188
.L171:
	movq	%rdx, (%rax)
	jmp	.L175
.L248:
	movq	0(%r13), %rax
	xorl	%edx, %edx
	jmp	.L210
.L201:
	testq	%r13, %r13
	jne	.L252
.L202:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	je	.L203
.L239:
	call	__stack_chk_fail@PLT
.L169:
	cmpq	%rbp, %rcx
	jne	.L167
	jmp	.L180
.L215:
	endbr64
	movq	%rax, %rbx
	jmp	.L201
.L245:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L239
	leaq	.LC1(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
.LEHE1:
.L252:
	movq	%r14, %rsi
	movq	%r13, %rdi
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	jmp	.L202
.L203:
	movq	%rbx, %rdi
.LEHB2:
	call	_Unwind_Resume@PLT
.LEHE2:
	.cfi_endproc
.LFE5132:
	.section	.gcc_except_table._Z5benchI8call_stdEdT_md,"aG",@progbits,_Z5benchI8call_stdEdT_md,comdat
.LLSDA5132:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE5132-.LLSDACSB5132
.LLSDACSB5132:
	.uleb128 .LEHB0-.LFB5132
	.uleb128 .LEHE0-.LEHB0
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB1-.LFB5132
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L215-.LFB5132
	.uleb128 0
	.uleb128 .LEHB2-.LFB5132
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
.LLSDACSE5132:
	.section	.text._Z5benchI8call_stdEdT_md,"axG",@progbits,_Z5benchI8call_stdEdT_md,comdat
	.size	_Z5benchI8call_stdEdT_md, .-_Z5benchI8call_stdEdT_md
	.section	.text._Z5benchI8call_nsgEdT_md,"axG",@progbits,_Z5benchI8call_nsgEdT_md,comdat
	.p2align 4
	.weak	_Z5benchI8call_nsgEdT_md
	.type	_Z5benchI8call_nsgEdT_md, @function
_Z5benchI8call_nsgEdT_md:
.LFB5136:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA5136
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	xorl	%r12d, %r12d
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$104, %rsp
	.cfi_def_cfa_offset 160
	movq	%rdi, 48(%rsp)
	movq	176(%rsp), %rax
	movsd	%xmm0, 40(%rsp)
	movq	168(%rsp), %rsi
	movq	%rax, %rdx
	movq	%fs:40, %rdi
	movq	%rdi, 88(%rsp)
	movq	160(%rsp), %rdi
	movq	%rsi, 24(%rsp)
	movq	%rax, 16(%rsp)
	movq	%rdi, 8(%rsp)
	call	_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	movl	$32768, %edi
.LEHB3:
	call	_Znwm@PLT
.LEHE3:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	movq	%rax, %rbp
	leaq	32768(%rax), %r14
	call	clock_gettime@PLT
	movq	%rbp, %r13
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	movq	%rax, 32(%rsp)
	jmp	.L260
	.p2align 4,,10
	.p2align 3
.L339:
	movsd	40(%rsp), %xmm4
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	comisd	%xmm0, %xmm4
	jbe	.L338
.L260:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	movq	16(%rsp), %rdx
	movq	24(%rsp), %rsi
	movq	8(%rsp), %rdi
	movq	72(%rsp), %r15
	imulq	$1000000000, 64(%rsp), %rbx
	call	_Z6do_nsgRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	subq	%r15, %rax
	addq	72(%rsp), %rax
	subq	%rbx, %rax
	movq	%rax, %rdx
	cmpq	%rbp, %r14
	je	.L254
	movq	%rax, 0(%rbp)
	addq	$8, %rbp
.L255:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	addq	$1, %r12
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	subq	32(%rsp), %rax
	jns	.L339
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	movsd	40(%rsp), %xmm4
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	comisd	%xmm0, %xmm4
	ja	.L260
.L338:
	cmpq	%r13, %rbp
	je	.L263
	movq	%rbp, %rbx
	leaq	8(%r13), %rcx
	subq	%r13, %rbx
	movq	%rcx, 8(%rsp)
	movq	%rbx, %r15
	sarq	$3, %r15
	je	.L340
	bsrq	%r15, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	addq	%rdx, %rdx
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	cmpq	$128, %rbx
	movq	8(%rsp), %rcx
	jle	.L267
	leaq	128(%r13), %rdi
	movl	$8, %r9d
	jmp	.L277
	.p2align 4,,10
	.p2align 3
.L342:
	movl	$8, %eax
	subq	%r9, %rax
	addq	%rcx, %rax
	cmpq	$8, %r9
	je	.L269
	cmpl	$32, %r9d
	ja	.L271
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm1
	movdqu	-16(%r13,%rdx), %xmm0
	movups	%xmm1, (%rax)
	movups	%xmm0, -16(%rax,%rdx)
.L273:
	addq	$8, %rcx
	movq	%rsi, 0(%r13)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	je	.L341
.L277:
	movq	(%rcx), %rsi
	movq	0(%r13), %rdx
	movq	%rcx, %r8
	cmpq	%rdx, %rsi
	jb	.L342
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %rsi
	jnb	.L275
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L276:
	movq	%rdx, 8(%rax)
	movq	%rax, %r8
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rsi
	jb	.L276
.L275:
	addq	$8, %rcx
	movq	%rsi, (%r8)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	jne	.L277
.L341:
	cmpq	%rbp, %rdi
	je	.L278
	.p2align 4
	.p2align 3
.L281:
	movq	(%rdi), %rcx
	movq	-8(%rdi), %rdx
	cmpq	%rdx, %rcx
	jnb	.L310
	leaq	-8(%rdi), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L280:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rcx
	jb	.L280
.L279:
	addq	$8, %rdi
	movq	%rcx, (%rsi)
	cmpq	%rdi, %rbp
	jne	.L281
.L278:
	movabsq	$-3689348814741910323, %rax
	mulq	%r15
	shrq	$4, %rdx
	leaq	(%rdx,%rdx), %rax
	cmpq	%r15, %rax
	jb	.L307
	movq	%r15, %rcx
	xorl	%edx, %edx
.L290:
	movq	%rcx, %rax
	shrq	%rax
	addq	%rax, %rdx
	andl	$1, %ecx
	movq	0(%r13,%rdx,8), %rax
	jne	.L291
.L308:
	addq	-8(%r13,%rdx,8), %rax
	shrq	%rax
.L291:
	imulq	%r15, %rax
	testq	%rax, %rax
	js	.L292
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L293:
	testq	%r12, %r12
	js	.L294
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
.L295:
	movq	48(%rsp), %rax
	testq	%rax, %rax
	js	.L296
	pxor	%xmm2, %xmm2
	cvtsi2sdq	%rax, %xmm2
.L297:
	mulsd	%xmm2, %xmm1
	divsd	%xmm1, %xmm0
	testq	%r13, %r13
	je	.L253
	movq	%r14, %rsi
	subq	%r13, %rsi
	jmp	.L306
	.p2align 4,,10
	.p2align 3
.L254:
	movabsq	$1152921504606846975, %rax
	movq	%r14, %rbp
	subq	%r13, %rbp
	movq	%rbp, %rcx
	sarq	$3, %rcx
	cmpq	%rax, %rcx
	je	.L343
	testq	%rcx, %rcx
	movl	$1, %eax
	movq	%rdx, 56(%rsp)
	cmovne	%rcx, %rax
	addq	%rcx, %rax
	movabsq	$1152921504606846975, %rcx
	cmpq	%rcx, %rax
	cmova	%rcx, %rax
	leaq	0(,%rax,8), %rbx
	movq	%rbx, %rdi
.LEHB4:
	call	_Znwm@PLT
	movq	56(%rsp), %rdx
	movq	%rax, %rcx
	movq	%rdx, (%rax,%rbp)
	testq	%rbp, %rbp
	jne	.L344
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	je	.L259
.L345:
	movq	%r14, %rsi
	movq	%r13, %rdi
	movq	%rcx, 56(%rsp)
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	movq	56(%rsp), %rcx
.L259:
	leaq	(%rcx,%rbx), %r14
	movq	%rcx, %r13
	jmp	.L255
	.p2align 4,,10
	.p2align 3
.L344:
	movq	%rbp, %rdx
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	memcpy@PLT
	movq	%rax, %rcx
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	jne	.L345
	jmp	.L259
	.p2align 4,,10
	.p2align 3
.L340:
	movq	$-2, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	movq	8(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L346
.L265:
	movl	$8, %ebx
	jmp	.L288
	.p2align 4,,10
	.p2align 3
.L348:
	movl	$8, %edi
	subq	%rbx, %rdi
	addq	%rcx, %rdi
	cmpq	$8, %rbx
	jle	.L283
	movq	%rbx, %rdx
	movq	%r13, %rsi
	movq	%rcx, 16(%rsp)
	movq	%r8, 8(%rsp)
	call	memmove@PLT
	movq	8(%rsp), %r8
	movq	16(%rsp), %rcx
.L284:
	addq	$8, %rcx
	movq	%r8, 0(%r13)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	je	.L347
.L288:
	movq	(%rcx), %r8
	movq	0(%r13), %rax
	cmpq	%rax, %r8
	jb	.L348
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %r8
	jnb	.L311
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L287:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %r8
	jb	.L287
.L286:
	addq	$8, %rcx
	movq	%r8, (%rsi)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	jne	.L288
.L347:
	xorl	%eax, %eax
	xorl	%edx, %edx
.L307:
	movq	%r15, %rcx
	subq	%rax, %rcx
	jmp	.L290
	.p2align 4,,10
	.p2align 3
.L271:
	cmpl	$64, %r9d
	jb	.L272
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm7
	movdqu	16(%r13), %xmm6
	leaq	0(%r13,%rdx), %r8
	movdqu	32(%r13), %xmm5
	movdqu	48(%r13), %xmm4
	movdqu	-16(%r8), %xmm3
	movdqu	-32(%r8), %xmm2
	movdqu	-48(%r8), %xmm1
	movdqu	-64(%r8), %xmm0
	movups	%xmm7, (%rax)
	movups	%xmm6, 16(%rax)
	movups	%xmm5, 32(%rax)
	movups	%xmm4, 48(%rax)
	movups	%xmm3, -16(%rax,%rdx)
	movups	%xmm2, -32(%rax,%rdx)
	movups	%xmm1, -48(%rax,%rdx)
	movups	%xmm0, -64(%rax,%rdx)
	jmp	.L273
	.p2align 4,,10
	.p2align 3
.L263:
	testq	%r12, %r12
	js	.L302
	movq	48(%rsp), %rax
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
	testq	%rax, %rax
	js	.L304
.L349:
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L305:
	mulsd	%xmm0, %xmm1
	pxor	%xmm0, %xmm0
	movq	%r14, %rsi
	subq	%r13, %rsi
	divsd	%xmm1, %xmm0
.L306:
	movq	%r13, %rdi
	movsd	%xmm0, 8(%rsp)
	call	_ZdlPvm@PLT
	movsd	8(%rsp), %xmm0
.L253:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L337
	addq	$104, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L272:
	.cfi_restore_state
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm3
	movdqu	16(%r13), %xmm2
	leaq	0(%r13,%rdx), %r8
	movdqu	-16(%r8), %xmm1
	movdqu	-32(%r8), %xmm0
	movups	%xmm3, (%rax)
	movups	%xmm2, 16(%rax)
	movups	%xmm1, -16(%rax,%rdx)
	movups	%xmm0, -32(%rax,%rdx)
	jmp	.L273
.L302:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	movq	48(%rsp), %rax
	addsd	%xmm1, %xmm1
	testq	%rax, %rax
	jns	.L349
.L304:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm0, %xmm0
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L305
	.p2align 4,,10
	.p2align 3
.L296:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm2, %xmm2
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm2
	addsd	%xmm2, %xmm2
	jmp	.L297
	.p2align 4,,10
	.p2align 3
.L294:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	addsd	%xmm1, %xmm1
	jmp	.L295
	.p2align 4,,10
	.p2align 3
.L292:
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L293
	.p2align 4,,10
	.p2align 3
.L310:
	movq	%rdi, %rsi
	jmp	.L279
.L283:
	jne	.L284
	movq	%rax, (%rdi)
	jmp	.L284
.L311:
	movq	%rcx, %rsi
	jmp	.L286
.L269:
	movq	%rdx, (%rax)
	jmp	.L273
.L346:
	movq	0(%r13), %rax
	xorl	%edx, %edx
	jmp	.L308
.L299:
	testq	%r13, %r13
	jne	.L350
.L300:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	je	.L301
.L337:
	call	__stack_chk_fail@PLT
.L267:
	cmpq	%rbp, %rcx
	jne	.L265
	jmp	.L278
.L313:
	endbr64
	movq	%rax, %rbx
	jmp	.L299
.L343:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L337
	leaq	.LC1(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
.LEHE4:
.L350:
	movq	%r14, %rsi
	movq	%r13, %rdi
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	jmp	.L300
.L301:
	movq	%rbx, %rdi
.LEHB5:
	call	_Unwind_Resume@PLT
.LEHE5:
	.cfi_endproc
.LFE5136:
	.section	.gcc_except_table._Z5benchI8call_nsgEdT_md,"aG",@progbits,_Z5benchI8call_nsgEdT_md,comdat
.LLSDA5136:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE5136-.LLSDACSB5136
.LLSDACSB5136:
	.uleb128 .LEHB3-.LFB5136
	.uleb128 .LEHE3-.LEHB3
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB4-.LFB5136
	.uleb128 .LEHE4-.LEHB4
	.uleb128 .L313-.LFB5136
	.uleb128 0
	.uleb128 .LEHB5-.LFB5136
	.uleb128 .LEHE5-.LEHB5
	.uleb128 0
	.uleb128 0
.LLSDACSE5136:
	.section	.text._Z5benchI8call_nsgEdT_md,"axG",@progbits,_Z5benchI8call_nsgEdT_md,comdat
	.size	_Z5benchI8call_nsgEdT_md, .-_Z5benchI8call_nsgEdT_md
	.section	.text._Z5benchI8call_segEdT_md,"axG",@progbits,_Z5benchI8call_segEdT_md,comdat
	.p2align 4
	.weak	_Z5benchI8call_segEdT_md
	.type	_Z5benchI8call_segEdT_md, @function
_Z5benchI8call_segEdT_md:
.LFB5137:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA5137
	endbr64
	pushq	%r15
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	xorl	%r12d, %r12d
	pushq	%rbp
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$104, %rsp
	.cfi_def_cfa_offset 160
	movq	%rdi, 48(%rsp)
	movq	176(%rsp), %rax
	movsd	%xmm0, 40(%rsp)
	movq	168(%rsp), %rsi
	movq	%rax, %rdx
	movq	%fs:40, %rdi
	movq	%rdi, 88(%rsp)
	movq	160(%rsp), %rdi
	movq	%rsi, 24(%rsp)
	movq	%rax, 16(%rsp)
	movq	%rdi, 8(%rsp)
	call	_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	movl	$32768, %edi
.LEHB6:
	call	_Znwm@PLT
.LEHE6:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	movq	%rax, %rbp
	leaq	32768(%rax), %r14
	call	clock_gettime@PLT
	movq	%rbp, %r13
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	movq	%rax, 32(%rsp)
	jmp	.L358
	.p2align 4,,10
	.p2align 3
.L437:
	movsd	40(%rsp), %xmm4
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	comisd	%xmm0, %xmm4
	jbe	.L436
.L358:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	movq	16(%rsp), %rdx
	movq	24(%rsp), %rsi
	movq	8(%rsp), %rdi
	movq	72(%rsp), %r15
	imulq	$1000000000, 64(%rsp), %rbx
	call	_Z6do_segRKN5boost9container5dequeI5MyIntvNS0_9deque_optILm0ELm128EvLb0EEEEERNS0_6vectorIS2_vvEEl
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	subq	%r15, %rax
	addq	72(%rsp), %rax
	subq	%rbx, %rax
	movq	%rax, %rdx
	cmpq	%rbp, %r14
	je	.L352
	movq	%rax, 0(%rbp)
	addq	$8, %rbp
.L353:
	leaq	64(%rsp), %rsi
	movl	$4, %edi
	addq	$1, %r12
	call	clock_gettime@PLT
	imulq	$1000000000, 64(%rsp), %rax
	addq	72(%rsp), %rax
	subq	32(%rsp), %rax
	jns	.L437
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	movsd	40(%rsp), %xmm4
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	comisd	%xmm0, %xmm4
	ja	.L358
.L436:
	cmpq	%r13, %rbp
	je	.L361
	movq	%rbp, %rbx
	leaq	8(%r13), %rcx
	subq	%r13, %rbx
	movq	%rcx, 8(%rsp)
	movq	%rbx, %r15
	sarq	$3, %r15
	je	.L438
	bsrq	%r15, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	addq	%rdx, %rdx
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	cmpq	$128, %rbx
	movq	8(%rsp), %rcx
	jle	.L365
	leaq	128(%r13), %rdi
	movl	$8, %r9d
	jmp	.L375
	.p2align 4,,10
	.p2align 3
.L440:
	movl	$8, %eax
	subq	%r9, %rax
	addq	%rcx, %rax
	cmpq	$8, %r9
	je	.L367
	cmpl	$32, %r9d
	ja	.L369
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm1
	movdqu	-16(%r13,%rdx), %xmm0
	movups	%xmm1, (%rax)
	movups	%xmm0, -16(%rax,%rdx)
.L371:
	addq	$8, %rcx
	movq	%rsi, 0(%r13)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	je	.L439
.L375:
	movq	(%rcx), %rsi
	movq	0(%r13), %rdx
	movq	%rcx, %r8
	cmpq	%rdx, %rsi
	jb	.L440
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %rsi
	jnb	.L373
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L374:
	movq	%rdx, 8(%rax)
	movq	%rax, %r8
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rsi
	jb	.L374
.L373:
	addq	$8, %rcx
	movq	%rsi, (%r8)
	addq	$8, %r9
	cmpq	%rdi, %rcx
	jne	.L375
.L439:
	cmpq	%rbp, %rdi
	je	.L376
	.p2align 4
	.p2align 3
.L379:
	movq	(%rdi), %rcx
	movq	-8(%rdi), %rdx
	cmpq	%rdx, %rcx
	jnb	.L408
	leaq	-8(%rdi), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L378:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %rcx
	jb	.L378
.L377:
	addq	$8, %rdi
	movq	%rcx, (%rsi)
	cmpq	%rdi, %rbp
	jne	.L379
.L376:
	movabsq	$-3689348814741910323, %rax
	mulq	%r15
	shrq	$4, %rdx
	leaq	(%rdx,%rdx), %rax
	cmpq	%r15, %rax
	jb	.L405
	movq	%r15, %rcx
	xorl	%edx, %edx
.L388:
	movq	%rcx, %rax
	shrq	%rax
	addq	%rax, %rdx
	andl	$1, %ecx
	movq	0(%r13,%rdx,8), %rax
	jne	.L389
.L406:
	addq	-8(%r13,%rdx,8), %rax
	shrq	%rax
.L389:
	imulq	%r15, %rax
	testq	%rax, %rax
	js	.L390
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L391:
	testq	%r12, %r12
	js	.L392
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
.L393:
	movq	48(%rsp), %rax
	testq	%rax, %rax
	js	.L394
	pxor	%xmm2, %xmm2
	cvtsi2sdq	%rax, %xmm2
.L395:
	mulsd	%xmm2, %xmm1
	divsd	%xmm1, %xmm0
	testq	%r13, %r13
	je	.L351
	movq	%r14, %rsi
	subq	%r13, %rsi
	jmp	.L404
	.p2align 4,,10
	.p2align 3
.L352:
	movabsq	$1152921504606846975, %rax
	movq	%r14, %rbp
	subq	%r13, %rbp
	movq	%rbp, %rcx
	sarq	$3, %rcx
	cmpq	%rax, %rcx
	je	.L441
	testq	%rcx, %rcx
	movl	$1, %eax
	movq	%rdx, 56(%rsp)
	cmovne	%rcx, %rax
	addq	%rcx, %rax
	movabsq	$1152921504606846975, %rcx
	cmpq	%rcx, %rax
	cmova	%rcx, %rax
	leaq	0(,%rax,8), %rbx
	movq	%rbx, %rdi
.LEHB7:
	call	_Znwm@PLT
	movq	56(%rsp), %rdx
	movq	%rax, %rcx
	movq	%rdx, (%rax,%rbp)
	testq	%rbp, %rbp
	jne	.L442
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	je	.L357
.L443:
	movq	%r14, %rsi
	movq	%r13, %rdi
	movq	%rcx, 56(%rsp)
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	movq	56(%rsp), %rcx
.L357:
	leaq	(%rcx,%rbx), %r14
	movq	%rcx, %r13
	jmp	.L353
	.p2align 4,,10
	.p2align 3
.L442:
	movq	%rbp, %rdx
	movq	%r13, %rsi
	movq	%rax, %rdi
	call	memcpy@PLT
	movq	%rax, %rcx
	leaq	8(%rcx,%rbp), %rbp
	testq	%r13, %r13
	jne	.L443
	jmp	.L357
	.p2align 4,,10
	.p2align 3
.L438:
	movq	$-2, %rdx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	call	_ZSt16__introsort_loopIN9__gnu_cxx17__normal_iteratorIPmSt6vectorImSaImEEEElSt4lessIvEEvT_S9_T0_T1_.isra.0
	movq	8(%rsp), %rcx
	cmpq	%rbp, %rcx
	je	.L444
.L363:
	movl	$8, %ebx
	jmp	.L386
	.p2align 4,,10
	.p2align 3
.L446:
	movl	$8, %edi
	subq	%rbx, %rdi
	addq	%rcx, %rdi
	cmpq	$8, %rbx
	jle	.L381
	movq	%rbx, %rdx
	movq	%r13, %rsi
	movq	%rcx, 16(%rsp)
	movq	%r8, 8(%rsp)
	call	memmove@PLT
	movq	8(%rsp), %r8
	movq	16(%rsp), %rcx
.L382:
	addq	$8, %rcx
	movq	%r8, 0(%r13)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	je	.L445
.L386:
	movq	(%rcx), %r8
	movq	0(%r13), %rax
	cmpq	%rax, %r8
	jb	.L446
	movq	-8(%rcx), %rdx
	cmpq	%rdx, %r8
	jnb	.L409
	leaq	-8(%rcx), %rax
	.p2align 5
	.p2align 4
	.p2align 3
.L385:
	movq	%rdx, 8(%rax)
	movq	%rax, %rsi
	movq	-8(%rax), %rdx
	subq	$8, %rax
	cmpq	%rdx, %r8
	jb	.L385
.L384:
	addq	$8, %rcx
	movq	%r8, (%rsi)
	addq	$8, %rbx
	cmpq	%rcx, %rbp
	jne	.L386
.L445:
	xorl	%eax, %eax
	xorl	%edx, %edx
.L405:
	movq	%r15, %rcx
	subq	%rax, %rcx
	jmp	.L388
	.p2align 4,,10
	.p2align 3
.L369:
	cmpl	$64, %r9d
	jb	.L370
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm7
	movdqu	16(%r13), %xmm6
	leaq	0(%r13,%rdx), %r8
	movdqu	32(%r13), %xmm5
	movdqu	48(%r13), %xmm4
	movdqu	-16(%r8), %xmm3
	movdqu	-32(%r8), %xmm2
	movdqu	-48(%r8), %xmm1
	movdqu	-64(%r8), %xmm0
	movups	%xmm7, (%rax)
	movups	%xmm6, 16(%rax)
	movups	%xmm5, 32(%rax)
	movups	%xmm4, 48(%rax)
	movups	%xmm3, -16(%rax,%rdx)
	movups	%xmm2, -32(%rax,%rdx)
	movups	%xmm1, -48(%rax,%rdx)
	movups	%xmm0, -64(%rax,%rdx)
	jmp	.L371
	.p2align 4,,10
	.p2align 3
.L361:
	testq	%r12, %r12
	js	.L400
	movq	48(%rsp), %rax
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%r12, %xmm1
	testq	%rax, %rax
	js	.L402
.L447:
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
.L403:
	mulsd	%xmm0, %xmm1
	pxor	%xmm0, %xmm0
	movq	%r14, %rsi
	subq	%r13, %rsi
	divsd	%xmm1, %xmm0
.L404:
	movq	%r13, %rdi
	movsd	%xmm0, 8(%rsp)
	call	_ZdlPvm@PLT
	movsd	8(%rsp), %xmm0
.L351:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L435
	addq	$104, %rsp
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx
	.cfi_def_cfa_offset 48
	popq	%rbp
	.cfi_def_cfa_offset 40
	popq	%r12
	.cfi_def_cfa_offset 32
	popq	%r13
	.cfi_def_cfa_offset 24
	popq	%r14
	.cfi_def_cfa_offset 16
	popq	%r15
	.cfi_def_cfa_offset 8
	ret
	.p2align 4,,10
	.p2align 3
.L370:
	.cfi_restore_state
	movl	%r9d, %edx
	movdqu	0(%r13), %xmm3
	movdqu	16(%r13), %xmm2
	leaq	0(%r13,%rdx), %r8
	movdqu	-16(%r8), %xmm1
	movdqu	-32(%r8), %xmm0
	movups	%xmm3, (%rax)
	movups	%xmm2, 16(%rax)
	movups	%xmm1, -16(%rax,%rdx)
	movups	%xmm0, -32(%rax,%rdx)
	jmp	.L371
.L400:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	movq	48(%rsp), %rax
	addsd	%xmm1, %xmm1
	testq	%rax, %rax
	jns	.L447
.L402:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm0, %xmm0
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L403
	.p2align 4,,10
	.p2align 3
.L394:
	movq	%rax, %rcx
	shrq	%rax
	pxor	%xmm2, %xmm2
	andl	$1, %ecx
	orq	%rcx, %rax
	cvtsi2sdq	%rax, %xmm2
	addsd	%xmm2, %xmm2
	jmp	.L395
	.p2align 4,,10
	.p2align 3
.L392:
	movq	%r12, %rax
	andl	$1, %r12d
	pxor	%xmm1, %xmm1
	shrq	%rax
	orq	%r12, %rax
	cvtsi2sdq	%rax, %xmm1
	addsd	%xmm1, %xmm1
	jmp	.L393
	.p2align 4,,10
	.p2align 3
.L390:
	movq	%rax, %rdx
	andl	$1, %eax
	pxor	%xmm0, %xmm0
	shrq	%rdx
	orq	%rax, %rdx
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
	jmp	.L391
	.p2align 4,,10
	.p2align 3
.L408:
	movq	%rdi, %rsi
	jmp	.L377
.L381:
	jne	.L382
	movq	%rax, (%rdi)
	jmp	.L382
.L409:
	movq	%rcx, %rsi
	jmp	.L384
.L367:
	movq	%rdx, (%rax)
	jmp	.L371
.L444:
	movq	0(%r13), %rax
	xorl	%edx, %edx
	jmp	.L406
.L397:
	testq	%r13, %r13
	jne	.L448
.L398:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	je	.L399
.L435:
	call	__stack_chk_fail@PLT
.L365:
	cmpq	%rbp, %rcx
	jne	.L363
	jmp	.L376
.L411:
	endbr64
	movq	%rax, %rbx
	jmp	.L397
.L441:
	movq	88(%rsp), %rax
	subq	%fs:40, %rax
	jne	.L435
	leaq	.LC1(%rip), %rdi
	call	_ZSt20__throw_length_errorPKc@PLT
.LEHE7:
.L448:
	movq	%r14, %rsi
	movq	%r13, %rdi
	subq	%r13, %rsi
	call	_ZdlPvm@PLT
	jmp	.L398
.L399:
	movq	%rbx, %rdi
.LEHB8:
	call	_Unwind_Resume@PLT
.LEHE8:
	.cfi_endproc
.LFE5137:
	.section	.gcc_except_table._Z5benchI8call_segEdT_md,"aG",@progbits,_Z5benchI8call_segEdT_md,comdat
.LLSDA5137:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE5137-.LLSDACSB5137
.LLSDACSB5137:
	.uleb128 .LEHB6-.LFB5137
	.uleb128 .LEHE6-.LEHB6
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB7-.LFB5137
	.uleb128 .LEHE7-.LEHB7
	.uleb128 .L411-.LFB5137
	.uleb128 0
	.uleb128 .LEHB8-.LFB5137
	.uleb128 .LEHE8-.LEHB8
	.uleb128 0
	.uleb128 0
.LLSDACSE5137:
	.section	.text._Z5benchI8call_segEdT_md,"axG",@progbits,_Z5benchI8call_segEdT_md,comdat
	.size	_Z5benchI8call_segEdT_md, .-_Z5benchI8call_segEdT_md
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC4:
	.string	"get_next_capacity, allocator's max size reached"
	.align 8
.LC5:
	.string	"boost::container::bad_alloc thrown"
	.align 8
.LC7:
	.string	"std=%.4f  nsg=%.4f  seg=%.4f  ns/elem\n"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB10:
	.section	.text.startup,"ax",@progbits
.LHOTB10:
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB4776:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA4776
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pxor	%xmm0, %xmm0
	movl	$400000, %edi
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$272, %rsp
	movq	%r13, -24(%rbp)
	movq	%r15, -8(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -56(%rbp)
	xorl	%eax, %eax
	movups	%xmm0, -88(%rbp)
	movdqa	.LC3(%rip), %xmm0
	movq	$0, -96(%rbp)
	movq	$0, -72(%rbp)
	movq	$0, -224(%rbp)
	movups	%xmm0, -216(%rbp)
.LEHB9:
	.cfi_offset 13, -40
	.cfi_offset 15, -24
	call	_Znwm@PLT
.LEHE9:
	xorl	%esi, %esi
	movl	$400000, %edx
	movq	%rax, %rdi
	movq	%rbx, -40(%rbp)
	movq	%r12, -32(%rbp)
	movq	%r14, -16(%rbp)
	movq	%rax, -224(%rbp)
	movq	$100000, -208(%rbp)
	.cfi_offset 3, -56
	.cfi_offset 12, -48
	.cfi_offset 14, -32
	call	memset@PLT
	movq	-72(%rbp), %r12
	movq	-80(%rbp), %rsi
	movq	-96(%rbp), %r8
	movq	%r12, %rbx
	movq	%r12, %rax
	subq	%rsi, %rbx
	cmpq	$100000, %rbx
	jbe	.L450
	cmpq	%rsi, %r12
	je	.L451
	shrq	$7, %rax
	subq	$100000, %rbx
	leaq	(%r8,%rax,8), %rcx
	movq	%rbx, %r14
	testq	%rcx, %rcx
	je	.L452
	andl	$127, %r12d
	subq	%rbx, %r12
	movq	%rcx, %rbx
	cmpq	$127, %r12
	ja	.L507
.L453:
	leaq	8(%rcx), %r12
	addq	$8, %rbx
	cmpq	%r12, %rbx
	jnb	.L456
	.p2align 4
	.p2align 3
.L457:
	movq	(%rbx), %rdi
	movl	$512, %esi
	addq	$8, %rbx
	call	_ZdlPvm@PLT
	cmpq	%r12, %rbx
	jb	.L457
	movq	-80(%rbp), %rsi
	movq	-96(%rbp), %r8
.L456:
	subq	%r14, -72(%rbp)
.L451:
	xorl	%edx, %edx
	.p2align 6
	.p2align 4
	.p2align 3
.L496:
	leaq	(%rsi,%rdx), %rax
	movq	%rax, %rcx
	shrq	$7, %rcx
	leaq	(%r8,%rcx,8), %rcx
	testq	%rcx, %rcx
	je	.L495
	movq	(%rcx), %rcx
	andl	$127, %eax
	movl	%edx, (%rcx,%rax,4)
	addq	$1, %rdx
	cmpq	$100000, %rdx
	jne	.L496
	leaq	-224(%rbp), %rax
	leaq	-96(%rbp), %rbx
	subq	$32, %rsp
	movl	$100000, %edi
	movq	%rbx, %xmm0
	movq	%rax, %xmm3
	movq	$100000, 16(%rsp)
	punpcklqdq	%xmm3, %xmm0
	movq	$100000, -112(%rbp)
	movaps	%xmm0, -192(%rbp)
	movaps	%xmm0, -160(%rbp)
	movaps	%xmm0, -128(%rbp)
	movups	%xmm0, (%rsp)
	movsd	.LC6(%rip), %xmm0
	movq	$100000, -176(%rbp)
	movq	$100000, -144(%rbp)
.LEHB10:
	.cfi_escape 0x2e,0x20
	call	_Z5benchI8call_segEdT_md
	movq	-144(%rbp), %rax
	movsd	%xmm0, -240(%rbp)
	movl	$100000, %edi
	movdqa	-160(%rbp), %xmm0
	movq	%rax, 16(%rsp)
	movups	%xmm0, (%rsp)
	movsd	.LC6(%rip), %xmm0
	call	_Z5benchI8call_nsgEdT_md
	movq	-176(%rbp), %rax
	movsd	%xmm0, -232(%rbp)
	movl	$100000, %edi
	movdqa	-192(%rbp), %xmm0
	movq	%rax, 16(%rsp)
	movups	%xmm0, (%rsp)
	movsd	.LC6(%rip), %xmm0
	call	_Z5benchI8call_stdEdT_md
	addq	$32, %rsp
	movsd	-240(%rbp), %xmm2
	movsd	-232(%rbp), %xmm1
	leaq	.LC7(%rip), %rsi
	movl	$2, %edi
	movl	$3, %eax
	.cfi_escape 0x2e,0
	call	__printf_chk@PLT
	movq	-208(%rbp), %rax
	testq	%rax, %rax
	jne	.L574
.L497:
	movq	-96(%rbp), %r8
	testq	%r8, %r8
	je	.L498
	movq	-72(%rbp), %rax
	shrq	$7, %rax
	leaq	8(%r8,%rax,8), %rdi
	movq	-80(%rbp), %rax
	movq	%rdi, %r14
	shrq	$7, %rax
	leaq	(%r8,%rax,8), %rbx
	cmpq	%rdi, %rbx
	jnb	.L499
	.p2align 4
	.p2align 3
.L500:
	movq	(%rbx), %rdi
	movl	$512, %esi
	addq	$8, %rbx
	call	_ZdlPvm@PLT
	cmpq	%r14, %rbx
	jb	.L500
	movq	-96(%rbp), %r8
.L499:
	movq	-88(%rbp), %rax
	movq	%r8, %rdi
	leaq	0(,%rax,8), %rsi
	call	_ZdlPvm@PLT
.L498:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L575
	movq	-40(%rbp), %rbx
	.cfi_remember_state
	.cfi_restore 3
	movq	-32(%rbp), %r12
	.cfi_restore 12
	xorl	%eax, %eax
	movq	-16(%rbp), %r14
	.cfi_restore 14
	movq	-24(%rbp), %r13
	movq	-8(%rbp), %r15
	leave
	.cfi_def_cfa 7, 8
	ret
.L450:
	.cfi_restore_state
	leaq	100000(%rsi), %rbx
	subq	%r12, %rbx
	testq	%r8, %r8
	je	.L458
	movq	%r12, %rdx
	notq	%rdx
	andl	$127, %edx
	cmpq	%rbx, %rdx
	jb	.L576
.L459:
	movq	%r12, %rax
	shrq	$7, %rax
	leaq	(%r8,%rax,8), %r14
	testq	%r14, %r14
	je	.L513
.L487:
	movq	(%r14), %rdx
	movq	%r12, %rcx
	movq	%r14, %rax
	andl	$127, %ecx
	leaq	(%rdx,%rcx,4), %r14
.L490:
	movq	-80(%rbp), %rsi
	testq	%rbx, %rbx
	je	.L491
	movq	%r14, %rdi
	movq	%rax, %r14
.L488:
	movq	%rdi, %rax
	subq	(%r14), %rax
	movl	$128, %r15d
	sarq	$2, %rax
	subq	%rax, %r15
	cmpq	%rbx, %r15
	cmova	%rbx, %r15
	testq	%r15, %r15
	je	.L514
	leaq	0(,%r15,4), %rdx
	xorl	%esi, %esi
	movq	%rbx, %r13
	call	memset@PLT
	subq	%r15, %r13
	je	.L573
	.p2align 4
	.p2align 3
.L494:
	addq	$8, %r14
	movl	$128, %r15d
	cmpq	$128, %r13
	movq	(%r14), %rdi
	cmovbe	%r13, %r15
	xorl	%esi, %esi
	leaq	0(,%r15,4), %rdx
	call	memset@PLT
	subq	%r15, %r13
	jne	.L494
.L573:
	movq	-80(%rbp), %rsi
	movq	-96(%rbp), %r8
.L491:
	addq	%rbx, %r12
	movq	%r12, -72(%rbp)
	jmp	.L451
.L574:
	movq	-224(%rbp), %rdi
	leaq	0(,%rax,4), %rsi
	call	_ZdlPvm@PLT
	jmp	.L497
.L576:
	movq	-88(%rbp), %r13
	leaq	-1(%rbx), %rcx
	movq	%r12, %r9
	subq	%rdx, %rcx
	shrq	$7, %r9
	shrq	$7, %rcx
	leaq	-1(%r13), %rdx
	movq	%rcx, %r15
	subq	%r9, %rdx
	addq	$1, %rcx
	cmpq	%rcx, %rdx
	jnb	.L460
	movq	%rsi, %rax
	leaq	8(,%r9,8), %rdi
	shrq	$7, %rax
	salq	$3, %rax
	subq	%rax, %rdi
	leaq	(%r8,%rax), %r9
	movq	%r13, %rax
	movq	%rdi, %rdx
	shrq	%rax
	movq	%rdi, -232(%rbp)
	sarq	$3, %rdx
	leaq	(%rcx,%rdx), %r14
	cmpq	%r14, %rax
	jnb	.L577
	leaq	1(%r14), %rsi
	leaq	0(%r13,%r13), %rax
	cmpq	%rax, %rsi
	cmovnb	%rsi, %rax
	movq	%rax, %r12
	salq	$7, %rax
	subq	$1, %rax
	shrq	$61, %rax
	jne	.L563
	movq	%r12, %rax
	shrq	$60, %rax
	jne	.L564
	leaq	0(,%r12,8), %rcx
	movq	%r8, -272(%rbp)
	movq	%rcx, %rdi
	movq	%rdx, -248(%rbp)
	movq	%r9, -264(%rbp)
	movq	%rcx, -256(%rbp)
	call	_Znwm@PLT
.LEHE10:
	movq	%rax, %rdi
	movq	%rax, -240(%rbp)
	movq	%r12, %rax
	movq	-272(%rbp), %r8
	subq	%r14, %rax
	shrq	%rax
	salq	$3, %rax
	cmpq	$0, -248(%rbp)
	leaq	(%rdi,%rax), %r14
	je	.L469
	movq	-256(%rbp), %rcx
	movq	-232(%rbp), %rdx
	movq	%r14, %rdi
	movq	%r8, -248(%rbp)
	movq	-264(%rbp), %rsi
	cmpq	%rax, %rcx
	cmovb	%rax, %rcx
	subq	%rax, %rcx
	call	__memmove_chk@PLT
	movq	-248(%rbp), %r8
.L469:
	leaq	0(,%r13,8), %rsi
	movq	%r8, %rdi
	call	_ZdlPvm@PLT
	movq	-240(%rbp), %r8
	movq	%r12, -88(%rbp)
	movq	-80(%rbp), %rsi
	movq	-72(%rbp), %r12
	movq	%r8, -96(%rbp)
.L464:
	movq	%r14, %rax
	andl	$127, %esi
	andl	$127, %r12d
	subq	%r8, %rax
	salq	$4, %rax
	addq	%rsi, %rax
	movq	%rax, -80(%rbp)
	movq	-232(%rbp), %rax
	leaq	-8(%r14,%rax), %rax
	subq	%r8, %rax
	salq	$4, %rax
	addq	%r12, %rax
	movq	%rax, -72(%rbp)
.L460:
	shrq	$7, %rax
	xorl	%r12d, %r12d
	leaq	8(%r8,%rax,8), %r13
	jmp	.L470
	.p2align 4,,10
	.p2align 3
.L512:
	addq	$1, %r12
.L470:
	movl	$512, %edi
.LEHB11:
	call	_Znwm@PLT
.LEHE11:
	movq	%rax, 0(%r13,%r12,8)
	cmpq	%r15, %r12
	jne	.L512
	movq	-72(%rbp), %r12
	movq	-96(%rbp), %r8
	jmp	.L459
.L458:
	testq	%rbx, %rbx
	je	.L578
	movq	%rbx, %rax
	shrq	$7, %rax
	leaq	3(%rax), %rdi
	leaq	1(%rax), %r14
	movl	$4, %eax
	cmpq	%rax, %rdi
	cmovnb	%rdi, %rax
	movq	%rax, %r12
	movq	%rax, %rdi
	salq	$7, %rax
	subq	%r14, %r12
	subq	$1, %rax
	shrq	%r12
	shrq	$61, %rax
	jne	.L565
	movq	%rdi, %r15
	leaq	0(,%rdi,8), %rdi
.LEHB12:
	call	_Znwm@PLT
.LEHE12:
	movq	%rax, -96(%rbp)
	leaq	(%rax,%r12,8), %r13
	movq	%r15, -88(%rbp)
	xorl	%r15d, %r15d
.L479:
	movl	$512, %edi
.LEHB13:
	call	_Znwm@PLT
.LEHE13:
	leaq	1(%r15), %rdi
	movq	%rax, 0(%r13,%r15,8)
	movq	%rdi, %r15
	cmpq	%r14, %rdi
	jb	.L479
	salq	$7, %r12
	movq	-96(%rbp), %r8
	movq	%r12, %r14
	movq	%r12, -80(%rbp)
	shrq	$4, %r14
	addq	%r8, %r14
	jne	.L487
	xorl	%edi, %edi
	jmp	.L488
.L452:
	movq	0, %r12
	negq	%r12
	sarq	$2, %r12
	subq	%rbx, %r12
	cmpq	$127, %r12
	jbe	.L456
.L507:
	testq	%r12, %r12
	jle	.L454
	sarq	$7, %r12
.L455:
	leaq	(%rcx,%r12,8), %rbx
	jmp	.L453
.L513:
	xorl	%eax, %eax
	jmp	.L490
.L578:
	movq	%r12, %r14
	shrq	$7, %r14
	salq	$3, %r14
	jne	.L487
	jmp	.L491
.L454:
	movq	%r12, %rax
	movl	$128, %edi
	notq	%rax
	cqto
	idivq	%rdi
	notq	%rax
	movq	%rax, %r12
	jmp	.L455
.L577:
	movq	%r13, %rax
	subq	%r14, %rax
	shrq	%rax
	leaq	(%r8,%rax,8), %r14
	cmpq	%r9, %r14
	jnb	.L462
	testq	%rdx, %rdx
	je	.L464
	movq	%rdi, %rdx
.L572:
	movq	%r9, %rsi
	movq	%r14, %rdi
	call	memmove@PLT
	movq	-72(%rbp), %r12
	movq	-80(%rbp), %rsi
	movq	-96(%rbp), %r8
	jmp	.L464
.L514:
	movq	%rbx, %r13
	jmp	.L494
.L462:
	testq	%rdx, %rdx
	je	.L464
	movq	-232(%rbp), %rdx
	jmp	.L572
.L575:
	call	__stack_chk_fail@PLT
.L517:
	endbr64
	jmp	.L471
.L515:
	.cfi_restore 3
	.cfi_restore 12
	.cfi_restore 14
	endbr64
	movq	%rbx, -40(%rbp)
	.cfi_offset 3, -56
	movq	%rax, %rbx
	jmp	.L502
.L520:
	.cfi_offset 12, -48
	.cfi_offset 14, -32
	endbr64
	jmp	.L481
.L561:
	endbr64
	jmp	.L562
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA4776:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATT4776-.LLSDATTD4776
.LLSDATTD4776:
	.byte	0x1
	.uleb128 .LLSDACSE4776-.LLSDACSB4776
.LLSDACSB4776:
	.uleb128 .LEHB9-.LFB4776
	.uleb128 .LEHE9-.LEHB9
	.uleb128 .L515-.LFB4776
	.uleb128 0
	.uleb128 .LEHB10-.LFB4776
	.uleb128 .LEHE10-.LEHB10
	.uleb128 .L561-.LFB4776
	.uleb128 0
	.uleb128 .LEHB11-.LFB4776
	.uleb128 .LEHE11-.LEHB11
	.uleb128 .L517-.LFB4776
	.uleb128 0x1
	.uleb128 .LEHB12-.LFB4776
	.uleb128 .LEHE12-.LEHB12
	.uleb128 .L561-.LFB4776
	.uleb128 0
	.uleb128 .LEHB13-.LFB4776
	.uleb128 .LEHE13-.LEHB13
	.uleb128 .L520-.LFB4776
	.uleb128 0x1
.LLSDACSE4776:
	.byte	0x1
	.byte	0
	.align 4
	.long	0

.LLSDATT4776:
	.section	.text.startup
	.cfi_endproc
	.section	.text.unlikely
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDAC4776
	.type	main.cold, @function
main.cold:
.LFSB4776:
.L471:
	.cfi_def_cfa 6, 16
	.cfi_offset 3, -56
	.cfi_offset 6, -16
	.cfi_offset 12, -48
	.cfi_offset 13, -40
	.cfi_offset 14, -32
	.cfi_offset 15, -24
	movq	%rax, %rdi
	call	__cxa_begin_catch@PLT
	testq	%r12, %r12
	je	.L472
	leaq	0(%r13,%r12,8), %rbx
.L473:
	movq	0(%r13), %rdi
	movl	$512, %esi
	addq	$8, %r13
	call	_ZdlPvm@PLT
	cmpq	%rbx, %r13
	jne	.L473
.L472:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L579
.LEHB14:
	call	__cxa_rethrow@PLT
.LEHE14:
.L518:
	endbr64
	movq	%rax, %rbx
	call	__cxa_end_catch@PLT
.L476:
	movq	-208(%rbp), %rax
	testq	%rax, %rax
	je	.L571
	movq	-224(%rbp), %rdi
	leaq	0(,%rax,4), %rsi
	call	_ZdlPvm@PLT
	movq	-32(%rbp), %r12
	.cfi_restore 12
	movq	-16(%rbp), %r14
	.cfi_restore 14
.L502:
	movq	-96(%rbp), %r8
	testq	%r8, %r8
	je	.L503
	movq	-72(%rbp), %rax
	movq	%r12, -32(%rbp)
	shrq	$7, %rax
	.cfi_offset 12, -48
	leaq	8(%r8,%rax,8), %r12
	movq	-80(%rbp), %rax
	shrq	$7, %rax
	leaq	(%r8,%rax,8), %rax
	cmpq	%r12, %rax
	jnb	.L504
	movq	%r14, -16(%rbp)
	.cfi_offset 14, -32
.L505:
	movq	(%rax), %rdi
	movq	%rax, %r14
	movl	$512, %esi
	call	_ZdlPvm@PLT
	leaq	8(%r14), %rax
	cmpq	%r12, %rax
	jb	.L505
	movq	-96(%rbp), %r8
	movq	-16(%rbp), %r14
	.cfi_restore 14
.L504:
	movq	-88(%rbp), %rax
	movq	%r8, %rdi
	leaq	0(,%rax,8), %rsi
	call	_ZdlPvm@PLT
	movq	-32(%rbp), %r12
	.cfi_restore 12
.L503:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	movq	%r12, -32(%rbp)
	movq	%r14, -16(%rbp)
	.cfi_offset 12, -48
	.cfi_offset 14, -32
	jne	.L580
	movq	%rbx, %rdi
.LEHB15:
	call	_Unwind_Resume@PLT
.LEHE15:
.L579:
	call	__stack_chk_fail@PLT
.L564:
	movl	$16, %edi
	call	__cxa_allocate_exception@PLT
	leaq	.LC5(%rip), %rdx
	movq	.LC9(%rip), %xmm0
	movq	%rdx, %xmm4
	punpcklqdq	%xmm4, %xmm0
	movups	%xmm0, (%rax)
	movq	-56(%rbp), %rdx
	subq	%fs:40, %rdx
	jne	.L581
	leaq	_ZN5boost9container9bad_allocD1Ev(%rip), %rdx
	leaq	_ZTIN5boost9container9bad_allocE(%rip), %rsi
	movq	%rax, %rdi
.LEHB16:
	call	__cxa_throw@PLT
.L563:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L582
	leaq	.LC4(%rip), %rdi
	call	_ZN5boost9container18throw_length_errorEPKc
.L581:
	call	__stack_chk_fail@PLT
.L582:
	call	__stack_chk_fail@PLT
.L516:
	endbr64
.L562:
	movq	%rax, %rbx
	jmp	.L476
.L495:
	xorl	%eax, %eax
	movl	%eax, 0
	ud2
.L565:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L583
	leaq	.LC4(%rip), %rdi
	call	_ZN5boost9container18throw_length_errorEPKc
.LEHE16:
.L481:
	movq	%rax, %rdi
	call	__cxa_begin_catch@PLT
	testq	%r15, %r15
	je	.L482
	leaq	0(%r13,%r15,8), %rbx
.L483:
	movq	0(%r13), %rdi
	movl	$512, %esi
	addq	$8, %r13
	call	_ZdlPvm@PLT
	cmpq	%rbx, %r13
	jne	.L483
.L482:
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L584
.LEHB17:
	call	__cxa_rethrow@PLT
.LEHE17:
.L583:
	call	__stack_chk_fail@PLT
.L580:
	call	__stack_chk_fail@PLT
.L571:
	movq	-32(%rbp), %r12
	.cfi_remember_state
	.cfi_restore 12
	movq	-16(%rbp), %r14
	.cfi_restore 14
	jmp	.L502
.L584:
	.cfi_restore_state
	call	__stack_chk_fail@PLT
.L521:
	endbr64
	movq	%rax, %rbx
	call	__cxa_end_catch@PLT
	movq	%rbx, %rdi
	call	__cxa_begin_catch@PLT
	movq	-88(%rbp), %rax
	movq	-96(%rbp), %rdi
	leaq	0(,%rax,8), %rsi
	call	_ZdlPvm@PLT
	xorl	%ecx, %ecx
	movq	%rcx, -96(%rbp)
	movq	%rcx, -88(%rbp)
	movq	-56(%rbp), %rax
	subq	%fs:40, %rax
	jne	.L585
.LEHB18:
	call	__cxa_rethrow@PLT
.LEHE18:
.L585:
	call	__stack_chk_fail@PLT
.L519:
	endbr64
	movq	%rax, %rbx
	call	__cxa_end_catch@PLT
	jmp	.L476
	.cfi_endproc
.LFE4776:
	.section	.gcc_except_table
	.align 4
.LLSDAC4776:
	.byte	0xff
	.byte	0x9b
	.uleb128 .LLSDATTC4776-.LLSDATTDC4776
.LLSDATTDC4776:
	.byte	0x1
	.uleb128 .LLSDACSEC4776-.LLSDACSBC4776
.LLSDACSBC4776:
	.uleb128 .LEHB14-.LCOLDB10
	.uleb128 .LEHE14-.LEHB14
	.uleb128 .L518-.LCOLDB10
	.uleb128 0
	.uleb128 .LEHB15-.LCOLDB10
	.uleb128 .LEHE15-.LEHB15
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB16-.LCOLDB10
	.uleb128 .LEHE16-.LEHB16
	.uleb128 .L516-.LCOLDB10
	.uleb128 0
	.uleb128 .LEHB17-.LCOLDB10
	.uleb128 .LEHE17-.LEHB17
	.uleb128 .L521-.LCOLDB10
	.uleb128 0x1
	.uleb128 .LEHB18-.LCOLDB10
	.uleb128 .LEHE18-.LEHB18
	.uleb128 .L519-.LCOLDB10
	.uleb128 0
.LLSDACSEC4776:
	.byte	0x1
	.byte	0
	.align 4
	.long	0

.LLSDATTC4776:
	.section	.text.unlikely
	.section	.text.startup
	.size	main, .-main
	.section	.text.unlikely
	.size	main.cold, .-main.cold
.LCOLDE10:
	.section	.text.startup
.LHOTE10:
	.weak	_ZTSN5boost9container9exceptionE
	.section	.rodata._ZTSN5boost9container9exceptionE,"aG",@progbits,_ZTSN5boost9container9exceptionE,comdat
	.align 16
	.type	_ZTSN5boost9container9exceptionE, @object
	.size	_ZTSN5boost9container9exceptionE, 29
_ZTSN5boost9container9exceptionE:
	.string	"N5boost9container9exceptionE"
	.weak	_ZTIN5boost9container9exceptionE
	.section	.data.rel.ro._ZTIN5boost9container9exceptionE,"awG",@progbits,_ZTIN5boost9container9exceptionE,comdat
	.align 8
	.type	_ZTIN5boost9container9exceptionE, @object
	.size	_ZTIN5boost9container9exceptionE, 24
_ZTIN5boost9container9exceptionE:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTSN5boost9container9exceptionE
	.quad	_ZTISt9exception
	.weak	_ZTSN5boost9container9bad_allocE
	.section	.rodata._ZTSN5boost9container9bad_allocE,"aG",@progbits,_ZTSN5boost9container9bad_allocE,comdat
	.align 16
	.type	_ZTSN5boost9container9bad_allocE, @object
	.size	_ZTSN5boost9container9bad_allocE, 29
_ZTSN5boost9container9bad_allocE:
	.string	"N5boost9container9bad_allocE"
	.weak	_ZTIN5boost9container9bad_allocE
	.section	.data.rel.ro._ZTIN5boost9container9bad_allocE,"awG",@progbits,_ZTIN5boost9container9bad_allocE,comdat
	.align 8
	.type	_ZTIN5boost9container9bad_allocE, @object
	.size	_ZTIN5boost9container9bad_allocE, 24
_ZTIN5boost9container9bad_allocE:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTSN5boost9container9bad_allocE
	.quad	_ZTIN5boost9container9exceptionE
	.weak	_ZTSN5boost9container12length_errorE
	.section	.rodata._ZTSN5boost9container12length_errorE,"aG",@progbits,_ZTSN5boost9container12length_errorE,comdat
	.align 32
	.type	_ZTSN5boost9container12length_errorE, @object
	.size	_ZTSN5boost9container12length_errorE, 33
_ZTSN5boost9container12length_errorE:
	.string	"N5boost9container12length_errorE"
	.weak	_ZTIN5boost9container12length_errorE
	.section	.data.rel.ro._ZTIN5boost9container12length_errorE,"awG",@progbits,_ZTIN5boost9container12length_errorE,comdat
	.align 8
	.type	_ZTIN5boost9container12length_errorE, @object
	.size	_ZTIN5boost9container12length_errorE, 24
_ZTIN5boost9container12length_errorE:
	.quad	_ZTVN10__cxxabiv120__si_class_type_infoE+16
	.quad	_ZTSN5boost9container12length_errorE
	.quad	_ZTIN5boost9container9exceptionE
	.weak	_ZTVN5boost9container9exceptionE
	.section	.data.rel.ro.local._ZTVN5boost9container9exceptionE,"awG",@progbits,_ZTVN5boost9container9exceptionE,comdat
	.align 8
	.type	_ZTVN5boost9container9exceptionE, @object
	.size	_ZTVN5boost9container9exceptionE, 40
_ZTVN5boost9container9exceptionE:
	.quad	0
	.quad	_ZTIN5boost9container9exceptionE
	.quad	_ZN5boost9container9exceptionD1Ev
	.quad	_ZN5boost9container9exceptionD0Ev
	.quad	_ZNK5boost9container9exception4whatEv
	.weak	_ZTVN5boost9container9bad_allocE
	.section	.data.rel.ro.local._ZTVN5boost9container9bad_allocE,"awG",@progbits,_ZTVN5boost9container9bad_allocE,comdat
	.align 8
	.type	_ZTVN5boost9container9bad_allocE, @object
	.size	_ZTVN5boost9container9bad_allocE, 40
_ZTVN5boost9container9bad_allocE:
	.quad	0
	.quad	_ZTIN5boost9container9bad_allocE
	.quad	_ZN5boost9container9bad_allocD1Ev
	.quad	_ZN5boost9container9bad_allocD0Ev
	.quad	_ZNK5boost9container9exception4whatEv
	.weak	_ZTVN5boost9container12length_errorE
	.section	.data.rel.ro.local._ZTVN5boost9container12length_errorE,"awG",@progbits,_ZTVN5boost9container12length_errorE,comdat
	.align 8
	.type	_ZTVN5boost9container12length_errorE, @object
	.size	_ZTVN5boost9container12length_errorE, 40
_ZTVN5boost9container12length_errorE:
	.quad	0
	.quad	_ZTIN5boost9container12length_errorE
	.quad	_ZN5boost9container12length_errorD1Ev
	.quad	_ZN5boost9container12length_errorD0Ev
	.quad	_ZNK5boost9container9exception4whatEv
	.globl	bench_utils_sink
	.bss
	.align 4
	.type	bench_utils_sink, @object
	.size	bench_utils_sink, 4
bench_utils_sink:
	.zero	4
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC3:
	.quad	100000
	.quad	0
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC6:
	.long	0
	.long	1105055077
	.section	.data.rel.ro.local,"aw"
	.align 8
.LC9:
	.quad	_ZTVN5boost9container9bad_allocE+16
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.globl	__gxx_personality_v0
	.ident	"GCC: (Ubuntu 16-20260322-1ubuntu1) 16.0.1 20260322 (experimental) [trunk r16-8246-g569ace1fa50]"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
