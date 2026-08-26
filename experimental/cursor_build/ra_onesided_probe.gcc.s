	.file	"ra_onesided_probe.cpp"
	.text
	.p2align 4
	.globl	copy_generic_bounded
	.type	copy_generic_bounded, @function
copy_generic_bounded:
.LFB2406:
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
.LFE2406:
	.size	copy_generic_bounded, .-copy_generic_bounded
	.p2align 4
	.globl	copy_both_ra
	.type	copy_both_ra, @function
copy_both_ra:
.LFB2407:
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
.LFE2407:
	.size	copy_both_ra, .-copy_both_ra
	.p2align 4
	.globl	copy_ra_dst_only
	.type	copy_ra_dst_only, @function
copy_ra_dst_only:
.LFB2408:
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
	addq	$4, %rdi
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
.LFE2408:
	.size	copy_ra_dst_only, .-copy_ra_dst_only
	.p2align 4
	.globl	copy_ra_src_only_bounded
	.type	copy_ra_src_only_bounded, @function
copy_ra_src_only_bounded:
.LFB2409:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rdi, %rsi
	je	.L31
	subq	%rdi, %rsi
	sarq	$2, %rsi
	jmp	.L33
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L35:
	movl	(%rdi), %edx
	addq	$4, %rax
	addq	$4, %rdi
	movl	%edx, -4(%rax)
	subq	$1, %rsi
	je	.L31
.L33:
	cmpq	%rax, %rcx
	jne	.L35
.L31:
	ret
	.cfi_endproc
.LFE2409:
	.size	copy_ra_src_only_bounded, .-copy_ra_src_only_bounded
	.p2align 4
	.globl	copy_ra_src_unbounded
	.type	copy_ra_src_unbounded, @function
copy_ra_src_unbounded:
.LFB2410:
	.cfi_startproc
	endbr64
	movq	%rsi, %r9
	movq	%rdx, %rax
	cmpq	%rsi, %rdi
	je	.L36
	movq	%rsi, %r8
	subq	%rdi, %r8
	leaq	-4(%r8), %rdx
	cmpq	$8, %rdx
	jbe	.L45
	leaq	-4(%rax), %rcx
	subq	%rdi, %rcx
	cmpq	$8, %rcx
	jbe	.L45
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
.L41:
	movdqu	(%rdi,%rdx), %xmm0
	movups	%xmm0, (%rax,%rdx)
	addq	$16, %rdx
	cmpq	%rcx, %rdx
	jne	.L41
	salq	$2, %r10
	cmpq	%r10, %rsi
	je	.L43
	addq	%rdx, %rdi
	movl	(%rdi), %ecx
	movl	%ecx, (%rax,%rdx)
	leaq	4(%rdi), %rcx
	cmpq	%rcx, %r9
	je	.L43
	movl	4(%rdi), %ecx
	movl	%ecx, 4(%rax,%rdx)
	leaq	8(%rdi), %rcx
	cmpq	%rcx, %r9
	je	.L43
	movl	8(%rdi), %ecx
	movl	%ecx, 8(%rax,%rdx)
.L43:
	addq	%r8, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L36:
	ret
	.p2align 4,,10
	.p2align 3
.L45:
	xorl	%edx, %edx
	.p2align 4
	.p2align 4
	.p2align 3
.L40:
	movl	(%rdi,%rdx), %ecx
	movl	%ecx, (%rax,%rdx)
	addq	$4, %rdx
	cmpq	%rdx, %r8
	jne	.L40
	addq	%r8, %rax
	ret
	.cfi_endproc
.LFE2410:
	.size	copy_ra_src_unbounded, .-copy_ra_src_unbounded
	.p2align 4
	.globl	copy_ra_src_counted
	.type	copy_ra_src_counted, @function
copy_ra_src_counted:
.LFB2411:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rdi, %rsi
	je	.L49
	movq	%rsi, %rcx
	subq	%rdi, %rcx
	movq	%rcx, %r8
	sarq	$2, %r8
	leaq	-1(%r8), %rdx
	cmpq	$2, %rdx
	jbe	.L51
	leaq	-4(%rax), %rdx
	subq	%rdi, %rdx
	cmpq	$8, %rdx
	jbe	.L51
	movq	%r8, %r9
	xorl	%edx, %edx
	shrq	$2, %r9
	movq	%r9, %rsi
	salq	$4, %rsi
	.p2align 5
	.p2align 4
	.p2align 3
.L52:
	movdqu	(%rdi,%rdx), %xmm0
	movups	%xmm0, (%rax,%rdx)
	addq	$16, %rdx
	cmpq	%rsi, %rdx
	jne	.L52
	salq	$2, %r9
	cmpq	%r9, %r8
	je	.L54
	addq	%rdx, %rdi
	subq	%r9, %r8
	movl	(%rdi), %esi
	movl	%esi, (%rax,%rdx)
	cmpq	$1, %r8
	je	.L54
	movl	4(%rdi), %esi
	movl	%esi, 4(%rax,%rdx)
	cmpq	$2, %r8
	je	.L54
	movl	8(%rdi), %esi
	movl	%esi, 8(%rax,%rdx)
.L54:
	addq	%rcx, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L49:
	ret
	.p2align 4,,10
	.p2align 3
.L51:
	xorl	%edx, %edx
	.p2align 4
	.p2align 4
	.p2align 3
.L55:
	movl	(%rdi,%rdx), %esi
	movl	%esi, (%rax,%rdx)
	addq	$4, %rdx
	cmpq	%rcx, %rdx
	jne	.L55
	addq	%rcx, %rax
	ret
	.cfi_endproc
.LFE2411:
	.size	copy_ra_src_counted, .-copy_ra_src_counted
	.p2align 4
	.globl	copy_if_generic
	.type	copy_if_generic, @function
copy_if_generic:
.LFB2413:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rcx, %rdx
	je	.L67
	cmpq	%rsi, %rdi
	je	.L67
	.p2align 4
	.p2align 3
.L69:
	movl	(%rdi), %edx
	testb	$1, %dl
	je	.L68
	movl	%edx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %rcx
	je	.L67
.L68:
	addq	$4, %rdi
	cmpq	%rdi, %rsi
	jne	.L69
.L67:
	ret
	.cfi_endproc
.LFE2413:
	.size	copy_if_generic, .-copy_if_generic
	.p2align 4
	.globl	copy_if_both_ra
	.type	copy_if_both_ra, @function
copy_if_both_ra:
.LFB2414:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	movq	%rcx, %r8
	cmpq	%rcx, %rdx
	je	.L75
	subq	%rdi, %rsi
	movq	%rsi, %rdx
	sarq	$2, %rdx
	cmpq	$60, %rsi
	jg	.L77
	jmp	.L78
	.p2align 4,,10
	.p2align 3
.L96:
	movl	(%rdi), %ecx
	subq	$16, %rdx
	testb	$1, %cl
	je	.L79
	movl	%ecx, (%rax)
	addq	$4, %rax
.L79:
	movl	4(%rdi), %ecx
	testb	$1, %cl
	je	.L80
	movl	%ecx, (%rax)
	addq	$4, %rax
.L80:
	movl	8(%rdi), %ecx
	testb	$1, %cl
	je	.L81
	movl	%ecx, (%rax)
	addq	$4, %rax
.L81:
	movl	12(%rdi), %ecx
	testb	$1, %cl
	je	.L82
	movl	%ecx, (%rax)
	addq	$4, %rax
.L82:
	movl	16(%rdi), %ecx
	testb	$1, %cl
	je	.L83
	movl	%ecx, (%rax)
	addq	$4, %rax
.L83:
	movl	20(%rdi), %ecx
	testb	$1, %cl
	je	.L84
	movl	%ecx, (%rax)
	addq	$4, %rax
.L84:
	movl	24(%rdi), %ecx
	testb	$1, %cl
	je	.L85
	movl	%ecx, (%rax)
	addq	$4, %rax
.L85:
	movl	28(%rdi), %ecx
	testb	$1, %cl
	je	.L86
	movl	%ecx, (%rax)
	addq	$4, %rax
.L86:
	movl	32(%rdi), %ecx
	testb	$1, %cl
	je	.L87
	movl	%ecx, (%rax)
	addq	$4, %rax
.L87:
	movl	36(%rdi), %ecx
	testb	$1, %cl
	je	.L88
	movl	%ecx, (%rax)
	addq	$4, %rax
.L88:
	movl	40(%rdi), %ecx
	testb	$1, %cl
	je	.L89
	movl	%ecx, (%rax)
	addq	$4, %rax
.L89:
	movl	44(%rdi), %ecx
	testb	$1, %cl
	je	.L90
	movl	%ecx, (%rax)
	addq	$4, %rax
.L90:
	movl	48(%rdi), %ecx
	testb	$1, %cl
	je	.L91
	movl	%ecx, (%rax)
	addq	$4, %rax
.L91:
	movl	52(%rdi), %ecx
	testb	$1, %cl
	je	.L92
	movl	%ecx, (%rax)
	addq	$4, %rax
.L92:
	movl	56(%rdi), %ecx
	testb	$1, %cl
	je	.L93
	movl	%ecx, (%rax)
	addq	$4, %rax
.L93:
	movl	60(%rdi), %ecx
	testb	$1, %cl
	je	.L94
	movl	%ecx, (%rax)
	addq	$4, %rax
.L94:
	addq	$64, %rdi
	cmpq	$15, %rdx
	jle	.L95
.L77:
	movq	%r8, %rcx
	subq	%rax, %rcx
	cmpq	$60, %rcx
	jg	.L96
	cmpq	%rax, %r8
	je	.L104
	.p2align 4
	.p2align 3
.L99:
	movl	(%rdi), %ecx
	testb	$1, %cl
	je	.L98
	movl	%ecx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %r8
	je	.L75
.L98:
	addq	$4, %rdi
	subq	$1, %rdx
	jne	.L99
	ret
.L95:
	cmpq	%rax, %r8
	je	.L104
.L78:
	testq	%rdx, %rdx
	jne	.L99
.L75:
	ret
.L104:
	movq	%r8, %rax
	ret
	.cfi_endproc
.LFE2414:
	.size	copy_if_both_ra, .-copy_if_both_ra
	.p2align 4
	.globl	copy_if_ra_dst_only
	.type	copy_if_ra_dst_only, @function
copy_if_ra_dst_only:
.LFB2415:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	movq	%rdi, %rdx
	movq	%rcx, %rdi
	cmpq	%rcx, %rax
	je	.L181
	.p2align 4
	.p2align 3
.L180:
	movq	%rdi, %rcx
	subq	%rax, %rcx
	cmpq	$60, %rcx
	jle	.L162
	cmpq	%rdx, %rsi
	je	.L159
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L164
	movl	%ecx, (%rax)
	addq	$4, %rax
.L164:
	leaq	4(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	4(%rdx), %ecx
	testb	$1, %cl
	je	.L165
	movl	%ecx, (%rax)
	addq	$4, %rax
.L165:
	leaq	8(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	8(%rdx), %ecx
	testb	$1, %cl
	je	.L166
	movl	%ecx, (%rax)
	addq	$4, %rax
.L166:
	leaq	12(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	12(%rdx), %ecx
	testb	$1, %cl
	je	.L167
	movl	%ecx, (%rax)
	addq	$4, %rax
.L167:
	leaq	16(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	16(%rdx), %ecx
	testb	$1, %cl
	je	.L168
	movl	%ecx, (%rax)
	addq	$4, %rax
.L168:
	leaq	20(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	20(%rdx), %ecx
	testb	$1, %cl
	je	.L169
	movl	%ecx, (%rax)
	addq	$4, %rax
.L169:
	leaq	24(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	24(%rdx), %ecx
	testb	$1, %cl
	je	.L170
	movl	%ecx, (%rax)
	addq	$4, %rax
.L170:
	leaq	28(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	28(%rdx), %ecx
	testb	$1, %cl
	je	.L171
	movl	%ecx, (%rax)
	addq	$4, %rax
.L171:
	leaq	32(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	32(%rdx), %ecx
	testb	$1, %cl
	je	.L172
	movl	%ecx, (%rax)
	addq	$4, %rax
.L172:
	leaq	36(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	36(%rdx), %ecx
	testb	$1, %cl
	je	.L173
	movl	%ecx, (%rax)
	addq	$4, %rax
.L173:
	leaq	40(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	40(%rdx), %ecx
	testb	$1, %cl
	je	.L174
	movl	%ecx, (%rax)
	addq	$4, %rax
.L174:
	leaq	44(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	44(%rdx), %ecx
	testb	$1, %cl
	je	.L175
	movl	%ecx, (%rax)
	addq	$4, %rax
.L175:
	leaq	48(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	48(%rdx), %ecx
	testb	$1, %cl
	je	.L176
	movl	%ecx, (%rax)
	addq	$4, %rax
.L176:
	leaq	52(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	52(%rdx), %ecx
	testb	$1, %cl
	je	.L177
	movl	%ecx, (%rax)
	addq	$4, %rax
.L177:
	leaq	56(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	56(%rdx), %ecx
	testb	$1, %cl
	je	.L178
	movl	%ecx, (%rax)
	addq	$4, %rax
.L178:
	leaq	60(%rdx), %rcx
	cmpq	%rcx, %rsi
	je	.L159
	movl	60(%rdx), %ecx
	testb	$1, %cl
	je	.L179
	movl	%ecx, (%rax)
	addq	$4, %rax
.L179:
	addq	$64, %rdx
	cmpq	%rdx, %rsi
	jne	.L180
	ret
	.p2align 4,,10
	.p2align 3
.L162:
	cmpq	%rdi, %rax
	jne	.L254
	jmp	.L181
	.p2align 4,,10
	.p2align 3
.L184:
	movl	(%rdx), %ecx
	testb	$1, %cl
	je	.L183
	movl	%ecx, (%rax)
	addq	$4, %rax
	cmpq	%rax, %rdi
	je	.L159
.L183:
	addq	$4, %rdx
.L254:
	cmpq	%rdx, %rsi
	jne	.L184
.L159:
	ret
.L181:
	movq	%rdi, %rax
	ret
	.cfi_endproc
.LFE2415:
	.size	copy_if_ra_dst_only, .-copy_if_ra_dst_only
	.p2align 4
	.globl	copy_if_ra_src_unbounded
	.type	copy_if_ra_src_unbounded, @function
copy_if_ra_src_unbounded:
.LFB2416:
	.cfi_startproc
	endbr64
	movq	%rdx, %rax
	cmpq	%rdi, %rsi
	je	.L256
	.p2align 5
	.p2align 4
	.p2align 3
.L258:
	movl	(%rdi), %ecx
	testb	$1, %cl
	je	.L257
	movl	%ecx, (%rax)
	addq	$4, %rax
.L257:
	addq	$4, %rdi
	cmpq	%rdi, %rsi
	jne	.L258
.L256:
	ret
	.cfi_endproc
.LFE2416:
	.size	copy_if_ra_src_unbounded, .-copy_if_ra_src_unbounded
	.p2align 4
	.globl	equal_generic
	.type	equal_generic, @function
equal_generic:
.LFB2417:
	.cfi_startproc
	endbr64
	cmpq	%rsi, %rdi
	jne	.L264
	jmp	.L268
	.p2align 4,,10
	.p2align 3
.L266:
	movl	(%rdx), %eax
	cmpl	%eax, (%rdi)
	jne	.L267
	addq	$4, %rdi
	addq	$4, %rdx
	cmpq	%rdi, %rsi
	je	.L268
.L264:
	cmpq	%rdx, %rcx
	jne	.L266
.L267:
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L268:
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE2417:
	.size	equal_generic, .-equal_generic
	.p2align 4
	.globl	equal_both_ra
	.type	equal_both_ra, @function
equal_both_ra:
.LFB2418:
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
	jne	.L276
	jmp	.L274
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L280:
	addq	$4, %rdi
	addq	$4, %rdx
	cmpq	%rdi, %rax
	je	.L274
.L276:
	movl	(%rdx), %esi
	cmpl	%esi, (%rdi)
	je	.L280
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L274:
	cmpq	%rdi, %r8
	sete	%al
	ret
	.cfi_endproc
.LFE2418:
	.size	equal_both_ra, .-equal_both_ra
	.p2align 4
	.globl	equal_ra_src_only
	.type	equal_ra_src_only, @function
equal_ra_src_only:
.LFB2419:
	.cfi_startproc
	endbr64
	movq	%rsi, %rax
	subq	%rdi, %rax
	sarq	$2, %rax
	cmpq	%rdi, %rsi
	jne	.L286
	jmp	.L287
	.p2align 4,,10
	.p2align 3
.L284:
	movl	(%rdx), %esi
	cmpl	%esi, (%rdi)
	jne	.L285
	addq	$4, %rdi
	addq	$4, %rdx
	subq	$1, %rax
	je	.L287
.L286:
	cmpq	%rdx, %rcx
	jne	.L284
.L285:
	xorl	%eax, %eax
	ret
	.p2align 4,,10
	.p2align 3
.L287:
	movl	$1, %eax
	ret
	.cfi_endproc
.LFE2419:
	.size	equal_ra_src_only, .-equal_ra_src_only
	.p2align 4
	.globl	equal_ra_dst_only
	.type	equal_ra_dst_only, @function
equal_ra_dst_only:
.LFB2420:
	.cfi_startproc
	endbr64
	cmpq	%rdx, %rcx
	je	.L290
	subq	%rdx, %rcx
	sarq	$2, %rcx
	jmp	.L291
	.p2align 4,,10
	.p2align 3
.L300:
	movl	(%rdx), %eax
	cmpl	%eax, (%rdi)
	jne	.L299
	addq	$4, %rdi
	addq	$4, %rdx
	subq	$1, %rcx
	je	.L290
.L291:
	cmpq	%rdi, %rsi
	jne	.L300
.L290:
	cmpq	%rdi, %rsi
	sete	%al
	ret
	.p2align 4,,10
	.p2align 3
.L299:
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE2420:
	.size	equal_ra_dst_only, .-equal_ra_dst_only
	.p2align 4
	.globl	merge_ra_dst_block
	.type	merge_ra_dst_block, @function
merge_ra_dst_block:
.LFB2421:
	.cfi_startproc
	endbr64
	movq	%r9, %rax
	movq	%rsi, %r10
	movq	%rcx, %r11
	subq	%r8, %rax
	cmpq	$124, %rax
	jle	.L308
	subq	$-128, %r8
	leaq	-128(%r8), %rax
	cmpq	%rdi, %r10
	je	.L301
	.p2align 4
	.p2align 3
.L313:
	cmpq	%rdx, %r11
	jne	.L306
	ret
	.p2align 5
	.p2align 4,,10
	.p2align 3
.L312:
	movl	%esi, %ecx
	addq	$4, %rax
	addq	$4, %rdx
	movl	%ecx, -4(%rax)
	cmpq	%r8, %rax
	je	.L311
.L306:
	movl	(%rdx), %esi
	movl	(%rdi), %ecx
	cmpl	%ecx, %esi
	jl	.L312
	movl	%ecx, (%rax)
	addq	$4, %rax
	addq	$4, %rdi
	cmpq	%r8, %rax
	jne	.L306
.L311:
	movq	%r9, %rcx
	subq	%r8, %rcx
	cmpq	$124, %rcx
	jle	.L301
	subq	$-128, %r8
	leaq	-128(%r8), %rax
	cmpq	%rdi, %r10
	jne	.L313
.L301:
	ret
.L308:
	movq	%r8, %rax
	ret
	.cfi_endproc
.LFE2421:
	.size	merge_ra_dst_block, .-merge_ra_dst_block
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB2422:
	.cfi_startproc
	endbr64
	xorl	%eax, %eax
	ret
	.cfi_endproc
.LFE2422:
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
