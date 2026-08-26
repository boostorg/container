	.file	"ra_onesided_probe2.cpp"
	.text
	.p2align 4
	.globl	copy_generic_ptr
	.type	copy_generic_ptr, @function
copy_generic_ptr:
.LFB2399:
	.cfi_startproc
	endbr64
	cmpq	%rsi, %rdi
	jne	.L3
	jmp	.L4
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L8:
	movl	(%rdi), %eax
	addq	$4, %rdi
	addq	$4, %rdx
	movl	%eax, -4(%rdx)
	cmpq	%rdi, %rsi
	je	.L4
.L3:
	cmpq	%rdx, %rcx
	jne	.L8
	movq	%rcx, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L4:
	movq	%rdx, %rax
	ret
	.cfi_endproc
.LFE2399:
	.size	copy_generic_ptr, .-copy_generic_ptr
	.p2align 4
	.globl	copy_both_ra
	.type	copy_both_ra, @function
copy_both_ra:
.LFB2400:
	.cfi_startproc
	endbr64
	subq	%rdx, %rcx
	subq	%rdi, %rsi
	movq	%rdx, %rax
	movq	%rcx, %r8
	movq	%rsi, %rdx
	sarq	$2, %r8
	sarq	$2, %rdx
	cmpq	%rcx, %rsi
	movq	%r8, %rcx
	cmovl	%rdx, %rcx
	salq	$2, %rcx
	leaq	(%rdi,%rcx), %r10
	cmpq	%r10, %rdi
	je	.L9
	leaq	-4(%rcx), %rdx
	cmpq	$8, %rdx
	jbe	.L20
	leaq	-4(%rax), %rsi
	subq	%rdi, %rsi
	cmpq	$8, %rsi
	jbe	.L20
	shrq	$2, %rdx
	leaq	1(%rdx), %r8
	xorl	%edx, %edx
	movq	%r8, %r9
	shrq	$2, %r9
	movq	%r9, %rsi
	salq	$4, %rsi
	.p2align 5
	.p2align 4
	.p2align 3
.L14:
	movdqu	(%rdi,%rdx), %xmm0
	movups	%xmm0, (%rax,%rdx)
	addq	$16, %rdx
	cmpq	%rdx, %rsi
	jne	.L14
	salq	$2, %r9
	cmpq	%r9, %r8
	je	.L16
	addq	%rsi, %rdi
	movl	(%rdi), %edx
	movl	%edx, (%rax,%rsi)
	leaq	4(%rdi), %rdx
	cmpq	%rdx, %r10
	je	.L16
	movl	4(%rdi), %edx
	movl	%edx, 4(%rax,%rsi)
	leaq	8(%rdi), %rdx
	cmpq	%rdx, %r10
	je	.L16
	movl	8(%rdi), %edx
	movl	%edx, 8(%rax,%rsi)
.L16:
	addq	%rcx, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L9:
	ret
	.p2align 4,,10
	.p2align 3
.L20:
	xorl	%edx, %edx
	.p2align 4
	.p2align 4
	.p2align 3
.L17:
	movl	(%rdi,%rdx), %esi
	movl	%esi, (%rax,%rdx)
	addq	$4, %rdx
	cmpq	%rdx, %rcx
	jne	.L17
	addq	%rcx, %rax
	ret
	.cfi_endproc
.LFE2400:
	.size	copy_both_ra, .-copy_both_ra
	.p2align 4
	.globl	copy_ra_dst_list_src
	.type	copy_ra_dst_list_src, @function
copy_ra_dst_list_src:
.LFB2401:
	.cfi_startproc
	endbr64
	cmpq	%rdx, %rcx
	je	.L27
	subq	%rdx, %rcx
	movq	%rdx, %rax
	sarq	$2, %rcx
	jmp	.L26
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L30:
	movl	(%rdi), %edx
	addq	$4, %rax
	movq	8(%rdi), %rdi
	movl	%edx, -4(%rax)
	subq	$1, %rcx
	je	.L29
.L26:
	cmpq	%rdi, %rsi
	jne	.L30
	ret
	.p2align 4,,10
	.p2align 3
.L29:
	ret
.L27:
	movq	%rcx, %rax
	ret
	.cfi_endproc
.LFE2401:
	.size	copy_ra_dst_list_src, .-copy_ra_dst_list_src
	.p2align 4
	.globl	copy_generic_list_src
	.type	copy_generic_list_src, @function
copy_generic_list_src:
.LFB2402:
	.cfi_startproc
	endbr64
	movq	%rsi, %rax
	movq	%rdi, %rsi
	cmpq	%rax, %rdi
	jne	.L33
	jmp	.L34
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L37:
	movl	(%rsi), %edi
	movq	8(%rsi), %rsi
	addq	$4, %rdx
	movl	%edi, -4(%rdx)
	cmpq	%rax, %rsi
	je	.L34
.L33:
	cmpq	%rdx, %rcx
	jne	.L37
	movq	%rcx, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L34:
	movq	%rdx, %rax
	ret
	.cfi_endproc
.LFE2402:
	.size	copy_generic_list_src, .-copy_generic_list_src
	.p2align 4
	.globl	copy_ra_src_list_dst
	.type	copy_ra_src_list_dst, @function
copy_ra_src_list_dst:
.LFB2403:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rdi, %rsi
	je	.L39
	subq	%rdi, %rsi
	sarq	$2, %rsi
	jmp	.L40
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L42:
	movl	(%rdi), %edx
	addq	$4, %rdi
	movl	%edx, (%rax)
	movq	8(%rax), %rax
	subq	$1, %rsi
	je	.L39
.L40:
	cmpq	%rax, %rcx
	jne	.L42
.L39:
	ret
	.cfi_endproc
.LFE2403:
	.size	copy_ra_src_list_dst, .-copy_ra_src_list_dst
	.p2align 4
	.globl	copy_generic_list_dst
	.type	copy_generic_list_dst, @function
copy_generic_list_dst:
.LFB2404:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rsi, %rdi
	jne	.L45
	ret
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L47:
	movl	(%rdi), %edx
	addq	$4, %rdi
	movl	%edx, (%rax)
	movq	8(%rax), %rax
	cmpq	%rdi, %rsi
	je	.L44
.L45:
	cmpq	%rax, %rcx
	jne	.L47
.L44:
	ret
	.cfi_endproc
.LFE2404:
	.size	copy_generic_list_dst, .-copy_generic_list_dst
	.p2align 4
	.globl	copy_ra_src_unbounded
	.type	copy_ra_src_unbounded, @function
copy_ra_src_unbounded:
.LFB2405:
	.cfi_startproc
	endbr64
	movq	%rsi, %r9
	movq	%rdx, %rax
	cmpq	%rsi, %rdi
	je	.L48
	movq	%rsi, %r8
	subq	%rdi, %r8
	leaq	-4(%r8), %rdx
	cmpq	$8, %rdx
	jbe	.L57
	leaq	-4(%rax), %rcx
	subq	%rdi, %rcx
	cmpq	$8, %rcx
	jbe	.L57
	shrq	$2, %rdx
	leaq	1(%rdx), %rsi
	xorl	%edx, %edx
	movq	%rsi, %r10
	shrq	$2, %r10
	movq	%r10, %rcx
	salq	$4, %rcx
	.p2align 5
	.p2align 4
	.p2align 3
.L53:
	movdqu	(%rdi,%rdx), %xmm0
	movups	%xmm0, (%rax,%rdx)
	addq	$16, %rdx
	cmpq	%rcx, %rdx
	jne	.L53
	salq	$2, %r10
	cmpq	%r10, %rsi
	je	.L55
	addq	%rdx, %rdi
	movl	(%rdi), %ecx
	movl	%ecx, (%rax,%rdx)
	leaq	4(%rdi), %rcx
	cmpq	%rcx, %r9
	je	.L55
	movl	4(%rdi), %ecx
	movl	%ecx, 4(%rax,%rdx)
	leaq	8(%rdi), %rcx
	cmpq	%rcx, %r9
	je	.L55
	movl	8(%rdi), %ecx
	movl	%ecx, 8(%rax,%rdx)
.L55:
	addq	%r8, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L48:
	ret
	.p2align 4,,10
	.p2align 3
.L57:
	xorl	%edx, %edx
	.p2align 4
	.p2align 4
	.p2align 3
.L52:
	movl	(%rdi,%rdx), %ecx
	movl	%ecx, (%rax,%rdx)
	addq	$4, %rdx
	cmpq	%rdx, %r8
	jne	.L52
	addq	%r8, %rax
	ret
	.cfi_endproc
.LFE2405:
	.size	copy_ra_src_unbounded, .-copy_ra_src_unbounded
	.p2align 4
	.globl	copy_if_generic_ptr
	.type	copy_if_generic_ptr, @function
copy_if_generic_ptr:
.LFB2407:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rcx, %rdx
	je	.L62
	cmpq	%rsi, %rdi
	je	.L62
	.p2align 4
	.p2align 3
.L64:
	movl	(%rdi), %edx
	testb	$1, %dl
	je	.L63
	movl	%edx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %rcx
	je	.L62
.L63:
	addq	$4, %rdi
	cmpq	%rdi, %rsi
	jne	.L64
.L62:
	ret
	.cfi_endproc
.LFE2407:
	.size	copy_if_generic_ptr, .-copy_if_generic_ptr
	.p2align 4
	.globl	copy_if_both_ra
	.type	copy_if_both_ra, @function
copy_if_both_ra:
.LFB2408:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	movq	%rcx, %r8
	cmpq	%rcx, %rdx
	je	.L70
	subq	%rdi, %rsi
	movq	%rsi, %rdx
	sarq	$2, %rdx
	cmpq	$60, %rsi
	jg	.L72
	jmp	.L73
	.p2align 4,,10
	.p2align 3
.L91:
	movl	(%rdi), %ecx
	subq	$16, %rdx
	testb	$1, %cl
	je	.L74
	movl	%ecx, (%rax)
	addq	$4, %rax
.L74:
	movl	4(%rdi), %ecx
	testb	$1, %cl
	je	.L75
	movl	%ecx, (%rax)
	addq	$4, %rax
.L75:
	movl	8(%rdi), %ecx
	testb	$1, %cl
	je	.L76
	movl	%ecx, (%rax)
	addq	$4, %rax
.L76:
	movl	12(%rdi), %ecx
	testb	$1, %cl
	je	.L77
	movl	%ecx, (%rax)
	addq	$4, %rax
.L77:
	movl	16(%rdi), %ecx
	testb	$1, %cl
	je	.L78
	movl	%ecx, (%rax)
	addq	$4, %rax
.L78:
	movl	20(%rdi), %ecx
	testb	$1, %cl
	je	.L79
	movl	%ecx, (%rax)
	addq	$4, %rax
.L79:
	movl	24(%rdi), %ecx
	testb	$1, %cl
	je	.L80
	movl	%ecx, (%rax)
	addq	$4, %rax
.L80:
	movl	28(%rdi), %ecx
	testb	$1, %cl
	je	.L81
	movl	%ecx, (%rax)
	addq	$4, %rax
.L81:
	movl	32(%rdi), %ecx
	testb	$1, %cl
	je	.L82
	movl	%ecx, (%rax)
	addq	$4, %rax
.L82:
	movl	36(%rdi), %ecx
	testb	$1, %cl
	je	.L83
	movl	%ecx, (%rax)
	addq	$4, %rax
.L83:
	movl	40(%rdi), %ecx
	testb	$1, %cl
	je	.L84
	movl	%ecx, (%rax)
	addq	$4, %rax
.L84:
	movl	44(%rdi), %ecx
	testb	$1, %cl
	je	.L85
	movl	%ecx, (%rax)
	addq	$4, %rax
.L85:
	movl	48(%rdi), %ecx
	testb	$1, %cl
	je	.L86
	movl	%ecx, (%rax)
	addq	$4, %rax
.L86:
	movl	52(%rdi), %ecx
	testb	$1, %cl
	je	.L87
	movl	%ecx, (%rax)
	addq	$4, %rax
.L87:
	movl	56(%rdi), %ecx
	testb	$1, %cl
	je	.L88
	movl	%ecx, (%rax)
	addq	$4, %rax
.L88:
	movl	60(%rdi), %ecx
	testb	$1, %cl
	je	.L89
	movl	%ecx, (%rax)
	addq	$4, %rax
.L89:
	addq	$64, %rdi
	cmpq	$15, %rdx
	jle	.L90
.L72:
	movq	%r8, %rcx
	subq	%rax, %rcx
	cmpq	$60, %rcx
	jg	.L91
	cmpq	%rax, %r8
	je	.L99
	.p2align 4
	.p2align 3
.L94:
	movl	(%rdi), %ecx
	testb	$1, %cl
	je	.L93
	movl	%ecx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %r8
	je	.L70
.L93:
	addq	$4, %rdi
	subq	$1, %rdx
	jne	.L94
	ret
.L90:
	cmpq	%rax, %r8
	je	.L99
.L73:
	testq	%rdx, %rdx
	jne	.L94
.L70:
	ret
.L99:
	movq	%r8, %rax
	ret
	.cfi_endproc
.LFE2408:
	.size	copy_if_both_ra, .-copy_if_both_ra
	.p2align 4
	.globl	copy_if_ra_dst_list_src
	.type	copy_if_ra_dst_list_src, @function
copy_if_ra_dst_list_src:
.LFB2409:
	.cfi_startproc
	endbr64
	movq	%rdi, %r8
	movq	%rdx, %rax
	movq	%rcx, %rdi
	cmpq	%rcx, %rdx
	je	.L154
	subq	%rax, %rcx
	movq	%r8, %rdx
	cmpq	$60, %rcx
	jg	.L156
	jmp	.L157
	.p2align 4,,10
	.p2align 3
.L235:
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L159
	movl	%ecx, (%rax)
	addq	$4, %rax
.L159:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L161
	movl	%ecx, (%rax)
	addq	$4, %rax
.L161:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L162
	movl	%ecx, (%rax)
	addq	$4, %rax
.L162:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L163
	movl	%ecx, (%rax)
	addq	$4, %rax
.L163:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L164
	movl	%ecx, (%rax)
	addq	$4, %rax
.L164:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L165
	movl	%ecx, (%rax)
	addq	$4, %rax
.L165:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L166
	movl	%ecx, (%rax)
	addq	$4, %rax
.L166:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L167
	movl	%ecx, (%rax)
	addq	$4, %rax
.L167:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L168
	movl	%ecx, (%rax)
	addq	$4, %rax
.L168:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L169
	movl	%ecx, (%rax)
	addq	$4, %rax
.L169:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L170
	movl	%ecx, (%rax)
	addq	$4, %rax
.L170:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L171
	movl	%ecx, (%rax)
	addq	$4, %rax
.L171:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L172
	movl	%ecx, (%rax)
	addq	$4, %rax
.L172:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L173
	movl	%ecx, (%rax)
	addq	$4, %rax
.L173:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L174
	movl	%ecx, (%rax)
	addq	$4, %rax
.L174:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	je	.L160
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L175
	movl	%ecx, (%rax)
	addq	$4, %rax
.L175:
	movq	8(%rdx), %rdx
.L160:
	movq	%rdi, %rcx
	subq	%rax, %rcx
	cmpq	$60, %rcx
	jle	.L158
.L156:
	cmpq	%rdx, %rsi
	jne	.L235
.L158:
	cmpq	%rax, %rdi
	je	.L181
.L157:
	cmpq	%rdx, %rsi
	je	.L154
	.p2align 4
	.p2align 3
.L177:
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L176
	movl	%ecx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %rdi
	je	.L181
.L176:
	movq	8(%rdx), %rdx
	cmpq	%rdx, %rsi
	jne	.L177
	ret
.L181:
	movq	%rdi, %rax
.L154:
	ret
	.cfi_endproc
.LFE2409:
	.size	copy_if_ra_dst_list_src, .-copy_if_ra_dst_list_src
	.p2align 4
	.globl	copy_if_ra_src_unbounded
	.type	copy_if_ra_src_unbounded, @function
copy_if_ra_src_unbounded:
.LFB2410:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rdi, %rsi
	je	.L237
	.p2align 5
	.p2align 4
	.p2align 3
.L239:
	movl	(%rdi), %ecx
	testb	$1, %cl
	je	.L238
	movl	%ecx, (%rax)
	addq	$4, %rax
.L238:
	addq	$4, %rdi
	cmpq	%rdi, %rsi
	jne	.L239
.L237:
	ret
	.cfi_endproc
.LFE2410:
	.size	copy_if_ra_src_unbounded, .-copy_if_ra_src_unbounded
	.p2align 4
	.globl	equal_generic_ptr
	.type	equal_generic_ptr, @function
equal_generic_ptr:
.LFB2411:
	.cfi_startproc
	endbr64
	cmpq	%rsi, %rdi
	jne	.L245
	jmp	.L249
	.p2align 4,,10
	.p2align 3
.L247:
	movl	(%rdx), %eax
	cmpl	%eax, (%rdi)
	jne	.L248
	addq	$4, %rdi
	addq	$4, %rdx
	cmpq	%rdi, %rsi
	je	.L249
.L245:
	cmpq	%rdx, %rcx
	jne	.L247
.L248:
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L249:
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE2411:
	.size	equal_generic_ptr, .-equal_generic_ptr
	.p2align 4
	.globl	equal_both_ra
	.type	equal_both_ra, @function
equal_both_ra:
.LFB2412:
	.cfi_startproc
	endbr64
	subq	%rdx, %rcx
	movq	%rsi, %r8
	subq	%rdi, %rsi
	movq	%rcx, %r9
	movq	%rsi, %rax
	sarq	$2, %r9
	sarq	$2, %rax
	cmpq	%rcx, %rsi
	cmovge	%r9, %rax
	leaq	(%rdi,%rax,4), %rax
	cmpq	%rax, %rdi
	jne	.L257
	jmp	.L255
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L261:
	addq	$4, %rdi
	addq	$4, %rdx
	cmpq	%rdi, %rax
	je	.L255
.L257:
	movl	(%rdx), %esi
	cmpl	%esi, (%rdi)
	je	.L261
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L255:
	cmpq	%rdi, %r8
	sete	%al
	ret
	.cfi_endproc
.LFE2412:
	.size	equal_both_ra, .-equal_both_ra
	.p2align 4
	.globl	equal_ra_a_list_b
	.type	equal_ra_a_list_b, @function
equal_ra_a_list_b:
.LFB2413:
	.cfi_startproc
	endbr64
	movq	%rsi, %rax
	subq	%rdi, %rax
	sarq	$2, %rax
	cmpq	%rdi, %rsi
	jne	.L267
	jmp	.L268
	.p2align 4,,10
	.p2align 3
.L265:
	movl	(%rdx), %esi
	cmpl	%esi, (%rdi)
	jne	.L266
	movq	8(%rdx), %rdx
	addq	$4, %rdi
	subq	$1, %rax
	je	.L268
.L267:
	cmpq	%rdx, %rcx
	jne	.L265
.L266:
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L268:
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE2413:
	.size	equal_ra_a_list_b, .-equal_ra_a_list_b
	.p2align 4
	.globl	equal_list_a_ra_b
	.type	equal_list_a_ra_b, @function
equal_list_a_ra_b:
.LFB2414:
	.cfi_startproc
	endbr64
	cmpq	%rdx, %rcx
	je	.L271
	subq	%rdx, %rcx
	sarq	$2, %rcx
	jmp	.L272
	.p2align 4,,10
	.p2align 3
.L281:
	movl	(%rdx), %eax
	cmpl	%eax, (%rdi)
	jne	.L280
	movq	8(%rdi), %rdi
	addq	$4, %rdx
	subq	$1, %rcx
	je	.L271
.L272:
	cmpq	%rdi, %rsi
	jne	.L281
.L271:
	cmpq	%rdi, %rsi
	sete	%al
	ret
	.p2align 4,,10
	.p2align 3
.L280:
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE2414:
	.size	equal_list_a_ra_b, .-equal_list_a_ra_b
	.p2align 4
	.globl	equal_generic_list_b
	.type	equal_generic_list_b, @function
equal_generic_list_b:
.LFB2415:
	.cfi_startproc
	endbr64
	cmpq	%rsi, %rdi
	jne	.L287
	jmp	.L288
	.p2align 4,,10
	.p2align 3
.L285:
	movl	(%rdx), %eax
	cmpl	%eax, (%rdi)
	jne	.L286
	addq	$4, %rdi
	movq	8(%rdx), %rdx
	cmpq	%rdi, %rsi
	je	.L288
.L287:
	cmpq	%rdx, %rcx
	jne	.L285
.L286:
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L288:
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE2415:
	.size	equal_generic_list_b, .-equal_generic_list_b
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB2416:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE2416:
	.size	main, .-main
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
