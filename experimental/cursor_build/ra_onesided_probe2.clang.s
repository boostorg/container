	.file	"ra_onesided_probe2.cpp"
	.text
	.globl	copy_generic_ptr                # -- Begin function copy_generic_ptr
	.p2align	4
	.type	copy_generic_ptr,@function
copy_generic_ptr:                       # @copy_generic_ptr
	.cfi_startproc
# %bb.0:
	cmpq	%rsi, %rdi
	je	.LBB0_1
# %bb.3:
	cmpq	%rcx, %rdx
	je	.LBB0_1
# %bb.4:
	movq	%rcx, %rax
	subq	%rdx, %rax
	addq	$-4, %rax
	shrq	$2, %rax
	movq	%rsi, %r8
	subq	%rdi, %r8
	addq	$-4, %r8
	shrq	$2, %r8
	cmpq	%r8, %rax
	cmovbq	%rax, %r8
	cmpq	$19, %r8
	jb	.LBB0_5
# %bb.6:
	movl	%esi, %eax
	subl	%edi, %eax
	movl	%ecx, %r9d
	subl	%edx, %r9d
	orl	%eax, %r9d
	testb	$3, %r9b
	setne	%al
	movq	%rdx, %r9
	subq	%rdi, %r9
	cmpq	$32, %r9
	setb	%r9b
	orb	%al, %r9b
	je	.LBB0_7
.LBB0_5:
	movq	%rdi, %r9
	movq	%rdx, %rax
.LBB0_10:
	addq	$4, %r9
	.p2align	4
.LBB0_11:                               # =>This Inner Loop Header: Depth=1
	movl	-4(%r9), %edx
	movl	%edx, (%rax)
	addq	$4, %rax
	cmpq	%rsi, %r9
	je	.LBB0_2
# %bb.12:                               #   in Loop: Header=BB0_11 Depth=1
	addq	$4, %r9
	cmpq	%rcx, %rax
	jne	.LBB0_11
.LBB0_2:
	retq
.LBB0_1:
	movq	%rdx, %rax
	retq
.LBB0_7:
	incq	%r8
	movq	%r8, %r10
	andq	$-8, %r10
	leaq	(%rdi,%r10,4), %r9
	leaq	(%rdx,%r10,4), %rax
	xorl	%r11d, %r11d
	.p2align	4
.LBB0_8:                                # =>This Inner Loop Header: Depth=1
	movups	(%rdi,%r11,4), %xmm0
	movups	16(%rdi,%r11,4), %xmm1
	movups	%xmm0, (%rdx,%r11,4)
	movups	%xmm1, 16(%rdx,%r11,4)
	addq	$8, %r11
	cmpq	%r11, %r10
	jne	.LBB0_8
# %bb.9:
	cmpq	%r10, %r8
	jne	.LBB0_10
	jmp	.LBB0_2
.Lfunc_end0:
	.size	copy_generic_ptr, .Lfunc_end0-copy_generic_ptr
	.cfi_endproc
                                        # -- End function
	.globl	copy_both_ra                    # -- Begin function copy_both_ra
	.p2align	4
	.type	copy_both_ra,@function
copy_both_ra:                           # @copy_both_ra
	.cfi_startproc
# %bb.0:
	subq	%rdi, %rsi
	sarq	$2, %rsi
	subq	%rdx, %rcx
	sarq	$2, %rcx
	cmpq	%rcx, %rsi
	cmovlq	%rsi, %rcx
	testq	%rcx, %rcx
	je	.LBB1_1
# %bb.3:
	leaq	(,%rcx,4), %r8
	addq	$-4, %r8
	cmpq	$28, %r8
	setb	%al
	movq	%rdx, %rsi
	subq	%rdi, %rsi
	cmpq	$32, %rsi
	setb	%sil
	orb	%al, %sil
	je	.LBB1_5
# %bb.4:
	movq	%rdi, %rsi
	movq	%rdx, %rax
	jmp	.LBB1_8
.LBB1_1:
	movq	%rdx, %rax
	retq
.LBB1_5:
	shrq	$2, %r8
	incq	%r8
	movq	%r8, %r9
	andq	$-8, %r9
	leaq	(%rdi,%r9,4), %rsi
	leaq	(%rdx,%r9,4), %rax
	xorl	%r10d, %r10d
	.p2align	4
.LBB1_6:                                # =>This Inner Loop Header: Depth=1
	movups	(%rdi,%r10,4), %xmm0
	movups	16(%rdi,%r10,4), %xmm1
	movups	%xmm0, (%rdx,%r10,4)
	movups	%xmm1, 16(%rdx,%r10,4)
	addq	$8, %r10
	cmpq	%r10, %r9
	jne	.LBB1_6
# %bb.7:
	cmpq	%r9, %r8
	je	.LBB1_2
.LBB1_8:
	leaq	(%rdi,%rcx,4), %rcx
	.p2align	4
.LBB1_9:                                # =>This Inner Loop Header: Depth=1
	movl	(%rsi), %edx
	movl	%edx, (%rax)
	addq	$4, %rsi
	addq	$4, %rax
	cmpq	%rcx, %rsi
	jne	.LBB1_9
.LBB1_2:
	retq
.Lfunc_end1:
	.size	copy_both_ra, .Lfunc_end1-copy_both_ra
	.cfi_endproc
                                        # -- End function
	.globl	copy_ra_dst_list_src            # -- Begin function copy_ra_dst_list_src
	.p2align	4
	.type	copy_ra_dst_list_src,@function
copy_ra_dst_list_src:                   # @copy_ra_dst_list_src
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	subq	%rdx, %rcx
	setne	%dl
	cmpq	%rsi, %rdi
	setne	%r8b
	andb	%dl, %r8b
	cmpb	$1, %r8b
	jne	.LBB2_4
# %bb.1:
	sarq	$2, %rcx
	decq	%rcx
	.p2align	4
.LBB2_2:                                # =>This Inner Loop Header: Depth=1
	movl	(%rdi), %edx
	movl	%edx, (%rax)
	addq	$4, %rax
	addq	$-1, %rcx
	jae	.LBB2_4
# %bb.3:                                #   in Loop: Header=BB2_2 Depth=1
	movq	8(%rdi), %rdi
	cmpq	%rsi, %rdi
	jne	.LBB2_2
.LBB2_4:
	retq
.Lfunc_end2:
	.size	copy_ra_dst_list_src, .Lfunc_end2-copy_ra_dst_list_src
	.cfi_endproc
                                        # -- End function
	.globl	copy_generic_list_src           # -- Begin function copy_generic_list_src
	.p2align	4
	.type	copy_generic_list_src,@function
copy_generic_list_src:                  # @copy_generic_list_src
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	cmpq	%rsi, %rdi
	je	.LBB3_4
	.p2align	4
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
	cmpq	%rcx, %rax
	je	.LBB3_4
# %bb.3:                                #   in Loop: Header=BB3_2 Depth=1
	movl	(%rdi), %edx
	movl	%edx, (%rax)
	addq	$4, %rax
	movq	8(%rdi), %rdi
	cmpq	%rsi, %rdi
	jne	.LBB3_2
.LBB3_4:
	retq
.Lfunc_end3:
	.size	copy_generic_list_src, .Lfunc_end3-copy_generic_list_src
	.cfi_endproc
                                        # -- End function
	.globl	copy_ra_src_list_dst            # -- Begin function copy_ra_src_list_dst
	.p2align	4
	.type	copy_ra_src_list_dst,@function
copy_ra_src_list_dst:                   # @copy_ra_src_list_dst
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	subq	%rdi, %rsi
	setne	%dl
	cmpq	%rcx, %rax
	setne	%r8b
	andb	%dl, %r8b
	cmpb	$1, %r8b
	jne	.LBB4_4
# %bb.1:
	sarq	$2, %rsi
	decq	%rsi
	xorl	%edx, %edx
	.p2align	4
.LBB4_2:                                # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%rdx,4), %r8d
	movl	%r8d, (%rax)
	movq	8(%rax), %rax
	cmpq	%rdx, %rsi
	je	.LBB4_4
# %bb.3:                                #   in Loop: Header=BB4_2 Depth=1
	incq	%rdx
	cmpq	%rcx, %rax
	jne	.LBB4_2
.LBB4_4:
	retq
.Lfunc_end4:
	.size	copy_ra_src_list_dst, .Lfunc_end4-copy_ra_src_list_dst
	.cfi_endproc
                                        # -- End function
	.globl	copy_generic_list_dst           # -- Begin function copy_generic_list_dst
	.p2align	4
	.type	copy_generic_list_dst,@function
copy_generic_list_dst:                  # @copy_generic_list_dst
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	cmpq	%rsi, %rdi
	je	.LBB5_5
# %bb.1:
	cmpq	%rcx, %rax
	je	.LBB5_5
# %bb.2:
	addq	$4, %rdi
	.p2align	4
.LBB5_3:                                # =>This Inner Loop Header: Depth=1
	movl	-4(%rdi), %edx
	movl	%edx, (%rax)
	movq	8(%rax), %rax
	cmpq	%rsi, %rdi
	je	.LBB5_5
# %bb.4:                                #   in Loop: Header=BB5_3 Depth=1
	addq	$4, %rdi
	cmpq	%rcx, %rax
	jne	.LBB5_3
.LBB5_5:
	retq
.Lfunc_end5:
	.size	copy_generic_list_dst, .Lfunc_end5-copy_generic_list_dst
	.cfi_endproc
                                        # -- End function
	.globl	copy_ra_src_unbounded           # -- Begin function copy_ra_src_unbounded
	.p2align	4
	.type	copy_ra_src_unbounded,@function
copy_ra_src_unbounded:                  # @copy_ra_src_unbounded
	.cfi_startproc
# %bb.0:
	cmpq	%rsi, %rdi
	je	.LBB6_1
# %bb.3:
	movq	%rsi, %r8
	subq	%rdi, %r8
	addq	$-4, %r8
	cmpq	$28, %r8
	setb	%al
	movq	%rdx, %rcx
	subq	%rdi, %rcx
	cmpq	$32, %rcx
	setb	%cl
	orb	%al, %cl
	je	.LBB6_5
# %bb.4:
	movq	%rdx, %rax
	movq	%rdi, %rcx
	jmp	.LBB6_8
.LBB6_1:
	movq	%rdx, %rax
	retq
.LBB6_5:
	shrq	$2, %r8
	incq	%r8
	movq	%r8, %r9
	andq	$-8, %r9
	leaq	(%rdx,%r9,4), %rax
	leaq	(%rdi,%r9,4), %rcx
	xorl	%r10d, %r10d
	.p2align	4
.LBB6_6:                                # =>This Inner Loop Header: Depth=1
	movups	(%rdi,%r10,4), %xmm0
	movups	16(%rdi,%r10,4), %xmm1
	movups	%xmm0, (%rdx,%r10,4)
	movups	%xmm1, 16(%rdx,%r10,4)
	addq	$8, %r10
	cmpq	%r10, %r9
	jne	.LBB6_6
# %bb.7:
	cmpq	%r9, %r8
	je	.LBB6_2
	.p2align	4
.LBB6_8:                                # =>This Inner Loop Header: Depth=1
	movl	(%rcx), %edx
	movl	%edx, (%rax)
	addq	$4, %rcx
	addq	$4, %rax
	cmpq	%rsi, %rcx
	jne	.LBB6_8
.LBB6_2:
	retq
.Lfunc_end6:
	.size	copy_ra_src_unbounded, .Lfunc_end6-copy_ra_src_unbounded
	.cfi_endproc
                                        # -- End function
	.globl	copy_if_generic_ptr             # -- Begin function copy_if_generic_ptr
	.p2align	4
	.type	copy_if_generic_ptr,@function
copy_if_generic_ptr:                    # @copy_if_generic_ptr
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	cmpq	%rcx, %rdx
	jne	.LBB7_1
	jmp	.LBB7_5
	.p2align	4
.LBB7_4:                                #   in Loop: Header=BB7_1 Depth=1
	addq	$4, %rdi
.LBB7_1:                                # =>This Inner Loop Header: Depth=1
	cmpq	%rsi, %rdi
	je	.LBB7_5
# %bb.2:                                #   in Loop: Header=BB7_1 Depth=1
	movl	(%rdi), %edx
	testb	$1, %dl
	je	.LBB7_4
# %bb.3:                                #   in Loop: Header=BB7_1 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	cmpq	%rcx, %rax
	jne	.LBB7_4
.LBB7_5:
	retq
.Lfunc_end7:
	.size	copy_if_generic_ptr, .Lfunc_end7-copy_if_generic_ptr
	.cfi_endproc
                                        # -- End function
	.globl	copy_if_both_ra                 # -- Begin function copy_if_both_ra
	.p2align	4
	.type	copy_if_both_ra,@function
copy_if_both_ra:                        # @copy_if_both_ra
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	cmpq	%rcx, %rdx
	je	.LBB8_43
# %bb.1:
	subq	%rdi, %rsi
	sarq	$2, %rsi
	cmpq	$16, %rsi
	setl	%dl
	movq	%rcx, %r8
	subq	%rax, %r8
	cmpq	$61, %r8
	setl	%r8b
	orb	%dl, %r8b
	jne	.LBB8_2
	.p2align	4
.LBB8_3:                                # =>This Inner Loop Header: Depth=1
	movl	(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_4
# %bb.5:                                #   in Loop: Header=BB8_3 Depth=1
	movl	4(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_6
.LBB8_7:                                #   in Loop: Header=BB8_3 Depth=1
	movl	8(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_8
.LBB8_9:                                #   in Loop: Header=BB8_3 Depth=1
	movl	12(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_10
.LBB8_11:                               #   in Loop: Header=BB8_3 Depth=1
	movl	16(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_12
.LBB8_13:                               #   in Loop: Header=BB8_3 Depth=1
	movl	20(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_14
.LBB8_15:                               #   in Loop: Header=BB8_3 Depth=1
	movl	24(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_16
.LBB8_17:                               #   in Loop: Header=BB8_3 Depth=1
	movl	28(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_18
.LBB8_19:                               #   in Loop: Header=BB8_3 Depth=1
	movl	32(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_20
.LBB8_21:                               #   in Loop: Header=BB8_3 Depth=1
	movl	36(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_22
.LBB8_23:                               #   in Loop: Header=BB8_3 Depth=1
	movl	40(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_24
.LBB8_25:                               #   in Loop: Header=BB8_3 Depth=1
	movl	44(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_26
.LBB8_27:                               #   in Loop: Header=BB8_3 Depth=1
	movl	48(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_28
.LBB8_29:                               #   in Loop: Header=BB8_3 Depth=1
	movl	52(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_30
.LBB8_31:                               #   in Loop: Header=BB8_3 Depth=1
	movl	56(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_32
.LBB8_33:                               #   in Loop: Header=BB8_3 Depth=1
	movl	60(%rdi), %edx
	testb	$1, %dl
	jne	.LBB8_34
.LBB8_35:                               #   in Loop: Header=BB8_3 Depth=1
	addq	$64, %rdi
	leaq	-16(%rsi), %rdx
	cmpq	$32, %rsi
	jge	.LBB8_36
	jmp	.LBB8_37
	.p2align	4
.LBB8_4:                                #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	4(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_7
.LBB8_6:                                #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	8(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_9
.LBB8_8:                                #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	12(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_11
.LBB8_10:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	16(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_13
.LBB8_12:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	20(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_15
.LBB8_14:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	24(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_17
.LBB8_16:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	28(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_19
.LBB8_18:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	32(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_21
.LBB8_20:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	36(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_23
.LBB8_22:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	40(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_25
.LBB8_24:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	44(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_27
.LBB8_26:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	48(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_29
.LBB8_28:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	52(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_31
.LBB8_30:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	56(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_33
.LBB8_32:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	movl	60(%rdi), %edx
	testb	$1, %dl
	je	.LBB8_35
.LBB8_34:                               #   in Loop: Header=BB8_3 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	addq	$64, %rdi
	leaq	-16(%rsi), %rdx
	cmpq	$32, %rsi
	jl	.LBB8_37
.LBB8_36:                               #   in Loop: Header=BB8_3 Depth=1
	movq	%rcx, %r8
	subq	%rax, %r8
	movq	%rdx, %rsi
	cmpq	$60, %r8
	jg	.LBB8_3
	jmp	.LBB8_37
.LBB8_2:
	movq	%rsi, %rdx
.LBB8_37:
	cmpq	%rcx, %rax
	je	.LBB8_43
# %bb.38:
	testq	%rdx, %rdx
	je	.LBB8_43
# %bb.39:
	xorl	%esi, %esi
	jmp	.LBB8_40
	.p2align	4
.LBB8_42:                               #   in Loop: Header=BB8_40 Depth=1
	incq	%rsi
	cmpq	%rsi, %rdx
	je	.LBB8_43
.LBB8_40:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%rsi,4), %r8d
	testb	$1, %r8b
	je	.LBB8_42
# %bb.41:                               #   in Loop: Header=BB8_40 Depth=1
	movl	%r8d, (%rax)
	addq	$4, %rax
	cmpq	%rcx, %rax
	jne	.LBB8_42
.LBB8_43:
	retq
.Lfunc_end8:
	.size	copy_if_both_ra, .Lfunc_end8-copy_if_both_ra
	.cfi_endproc
                                        # -- End function
	.globl	copy_if_ra_dst_list_src         # -- Begin function copy_if_ra_dst_list_src
	.p2align	4
	.type	copy_if_ra_dst_list_src,@function
copy_if_ra_dst_list_src:                # @copy_if_ra_dst_list_src
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	cmpq	%rcx, %rdx
	je	.LBB9_13
# %bb.1:
	movq	%rcx, %rdx
	subq	%rax, %rdx
	cmpq	$61, %rdx
	setge	%dl
	cmpq	%rsi, %rdi
	setne	%r8b
	andb	%dl, %r8b
	cmpb	$1, %r8b
	jne	.LBB9_8
# %bb.2:
	movl	$16, %edx
	.p2align	4
.LBB9_5:                                # =>This Inner Loop Header: Depth=1
	movl	(%rdi), %r8d
	testb	$1, %r8b
	je	.LBB9_7
# %bb.6:                                #   in Loop: Header=BB9_5 Depth=1
	movl	%r8d, (%rax)
	addq	$4, %rax
.LBB9_7:                                #   in Loop: Header=BB9_5 Depth=1
	decq	%rdx
	movq	8(%rdi), %rdi
	setne	%r8b
	cmpq	%rsi, %rdi
	setne	%r9b
	testb	%r9b, %r8b
	jne	.LBB9_5
# %bb.3:                                #   in Loop: Header=BB9_5 Depth=1
	movq	%rcx, %rdx
	subq	%rax, %rdx
	cmpq	$61, %rdx
	jl	.LBB9_8
# %bb.4:                                #   in Loop: Header=BB9_5 Depth=1
	movl	$16, %edx
	cmpq	%rsi, %rdi
	jne	.LBB9_5
.LBB9_8:
	cmpq	%rcx, %rax
	jne	.LBB9_9
	jmp	.LBB9_13
	.p2align	4
.LBB9_12:                               #   in Loop: Header=BB9_9 Depth=1
	movq	8(%rdi), %rdi
.LBB9_9:                                # =>This Inner Loop Header: Depth=1
	cmpq	%rsi, %rdi
	je	.LBB9_13
# %bb.10:                               #   in Loop: Header=BB9_9 Depth=1
	movl	(%rdi), %edx
	testb	$1, %dl
	je	.LBB9_12
# %bb.11:                               #   in Loop: Header=BB9_9 Depth=1
	movl	%edx, (%rax)
	addq	$4, %rax
	cmpq	%rcx, %rax
	jne	.LBB9_12
.LBB9_13:
	retq
.Lfunc_end9:
	.size	copy_if_ra_dst_list_src, .Lfunc_end9-copy_if_ra_dst_list_src
	.cfi_endproc
                                        # -- End function
	.globl	copy_if_ra_src_unbounded        # -- Begin function copy_if_ra_src_unbounded
	.p2align	4
	.type	copy_if_ra_src_unbounded,@function
copy_if_ra_src_unbounded:               # @copy_if_ra_src_unbounded
	.cfi_startproc
# %bb.0:
	movq	%rdx, %rax
	subq	%rdi, %rsi
	je	.LBB10_12
# %bb.1:
	movq	%rsi, %rcx
	sarq	$2, %rcx
	testb	$4, %sil
	je	.LBB10_5
# %bb.2:
	movl	(%rdi), %edx
	testb	$1, %dl
	je	.LBB10_4
# %bb.3:
	movl	%edx, (%rax)
	addq	$4, %rax
.LBB10_4:
	decq	%rcx
	addq	$4, %rdi
.LBB10_5:
	cmpq	$4, %rsi
	jne	.LBB10_6
.LBB10_12:
	retq
.LBB10_6:
	xorl	%edx, %edx
	jmp	.LBB10_7
	.p2align	4
.LBB10_11:                              #   in Loop: Header=BB10_7 Depth=1
	addq	$2, %rdx
	cmpq	%rdx, %rcx
	je	.LBB10_12
.LBB10_7:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%rdx,4), %esi
	testb	$1, %sil
	jne	.LBB10_8
# %bb.9:                                #   in Loop: Header=BB10_7 Depth=1
	movl	4(%rdi,%rdx,4), %esi
	testb	$1, %sil
	je	.LBB10_11
	jmp	.LBB10_10
	.p2align	4
.LBB10_8:                               #   in Loop: Header=BB10_7 Depth=1
	movl	%esi, (%rax)
	addq	$4, %rax
	movl	4(%rdi,%rdx,4), %esi
	testb	$1, %sil
	je	.LBB10_11
.LBB10_10:                              #   in Loop: Header=BB10_7 Depth=1
	movl	%esi, (%rax)
	addq	$4, %rax
	jmp	.LBB10_11
.Lfunc_end10:
	.size	copy_if_ra_src_unbounded, .Lfunc_end10-copy_if_ra_src_unbounded
	.cfi_endproc
                                        # -- End function
	.globl	equal_generic_ptr               # -- Begin function equal_generic_ptr
	.p2align	4
	.type	equal_generic_ptr,@function
equal_generic_ptr:                      # @equal_generic_ptr
	.cfi_startproc
# %bb.0:
	cmpq	%rsi, %rdi
	sete	%al
	je	.LBB11_7
# %bb.1:
	cmpq	%rcx, %rdx
	je	.LBB11_7
# %bb.2:
	addq	$4, %rdx
	addq	$4, %rdi
	.p2align	4
.LBB11_3:                               # =>This Inner Loop Header: Depth=1
	movl	-4(%rdi), %eax
	cmpl	-4(%rdx), %eax
	jne	.LBB11_4
# %bb.5:                                #   in Loop: Header=BB11_3 Depth=1
	cmpq	%rsi, %rdi
	sete	%al
	je	.LBB11_7
# %bb.6:                                #   in Loop: Header=BB11_3 Depth=1
	addq	$4, %rdi
	cmpq	%rcx, %rdx
	leaq	4(%rdx), %rdx
	jne	.LBB11_3
.LBB11_7:
                                        # kill: def $al killed $al killed $eax
	retq
.LBB11_4:
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end11:
	.size	equal_generic_ptr, .Lfunc_end11-equal_generic_ptr
	.cfi_endproc
                                        # -- End function
	.globl	equal_both_ra                   # -- Begin function equal_both_ra
	.p2align	4
	.type	equal_both_ra,@function
equal_both_ra:                          # @equal_both_ra
	.cfi_startproc
# %bb.0:
	movq	%rsi, %rax
	subq	%rdi, %rax
	sarq	$2, %rax
	subq	%rdx, %rcx
	sarq	$2, %rcx
	cmpq	%rcx, %rax
	cmovlq	%rax, %rcx
	testq	%rcx, %rcx
	je	.LBB12_6
# %bb.1:
	leaq	(%rdi,%rcx,4), %rax
	shlq	$2, %rcx
	xorl	%r8d, %r8d
	.p2align	4
.LBB12_2:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%r8), %r9d
	cmpl	(%rdx,%r8), %r9d
	jne	.LBB12_3
# %bb.4:                                #   in Loop: Header=BB12_2 Depth=1
	addq	$4, %r8
	cmpq	%r8, %rcx
	jne	.LBB12_2
# %bb.5:
	movq	%rax, %rdi
.LBB12_6:
	cmpq	%rsi, %rdi
	sete	%al
                                        # kill: def $al killed $al killed $eax
	retq
.LBB12_3:
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end12:
	.size	equal_both_ra, .Lfunc_end12-equal_both_ra
	.cfi_endproc
                                        # -- End function
	.globl	equal_ra_a_list_b               # -- Begin function equal_ra_a_list_b
	.p2align	4
	.type	equal_ra_a_list_b,@function
equal_ra_a_list_b:                      # @equal_ra_a_list_b
	.cfi_startproc
# %bb.0:
	subq	%rdi, %rsi
	sete	%al
	je	.LBB13_7
# %bb.1:
	cmpq	%rcx, %rdx
	je	.LBB13_7
# %bb.2:
	sarq	$2, %rsi
	decq	%rsi
	xorl	%r8d, %r8d
	.p2align	4
.LBB13_3:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi,%r8,4), %eax
	cmpl	(%rdx), %eax
	jne	.LBB13_4
# %bb.5:                                #   in Loop: Header=BB13_3 Depth=1
	cmpq	%r8, %rsi
	sete	%al
	je	.LBB13_7
# %bb.6:                                #   in Loop: Header=BB13_3 Depth=1
	movq	8(%rdx), %rdx
	incq	%r8
	cmpq	%rcx, %rdx
	jne	.LBB13_3
.LBB13_7:
                                        # kill: def $al killed $al killed $eax
	retq
.LBB13_4:
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end13:
	.size	equal_ra_a_list_b, .Lfunc_end13-equal_ra_a_list_b
	.cfi_endproc
                                        # -- End function
	.globl	equal_list_a_ra_b               # -- Begin function equal_list_a_ra_b
	.p2align	4
	.type	equal_list_a_ra_b,@function
equal_list_a_ra_b:                      # @equal_list_a_ra_b
	.cfi_startproc
# %bb.0:
	subq	%rdx, %rcx
	setne	%al
	cmpq	%rsi, %rdi
	setne	%r8b
	andb	%al, %r8b
	cmpb	$1, %r8b
	jne	.LBB14_6
# %bb.1:
	sarq	$2, %rcx
	decq	%rcx
	xorl	%eax, %eax
	.p2align	4
.LBB14_2:                               # =>This Inner Loop Header: Depth=1
	movl	(%rdi), %r8d
	cmpl	(%rdx,%rax,4), %r8d
	jne	.LBB14_3
# %bb.4:                                #   in Loop: Header=BB14_2 Depth=1
	movq	8(%rdi), %rdi
	cmpq	%rax, %rcx
	je	.LBB14_6
# %bb.5:                                #   in Loop: Header=BB14_2 Depth=1
	incq	%rax
	cmpq	%rsi, %rdi
	jne	.LBB14_2
.LBB14_6:
	cmpq	%rsi, %rdi
	sete	%al
                                        # kill: def $al killed $al killed $eax
	retq
.LBB14_3:
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end14:
	.size	equal_list_a_ra_b, .Lfunc_end14-equal_list_a_ra_b
	.cfi_endproc
                                        # -- End function
	.globl	equal_generic_list_b            # -- Begin function equal_generic_list_b
	.p2align	4
	.type	equal_generic_list_b,@function
equal_generic_list_b:                   # @equal_generic_list_b
	.cfi_startproc
# %bb.0:
	cmpq	%rsi, %rdi
	sete	%al
	je	.LBB15_7
# %bb.1:
	cmpq	%rcx, %rdx
	je	.LBB15_7
# %bb.2:
	addq	$4, %rdi
	.p2align	4
.LBB15_3:                               # =>This Inner Loop Header: Depth=1
	movl	-4(%rdi), %eax
	cmpl	(%rdx), %eax
	jne	.LBB15_4
# %bb.5:                                #   in Loop: Header=BB15_3 Depth=1
	cmpq	%rsi, %rdi
	sete	%al
	je	.LBB15_7
# %bb.6:                                #   in Loop: Header=BB15_3 Depth=1
	movq	8(%rdx), %rdx
	addq	$4, %rdi
	cmpq	%rcx, %rdx
	jne	.LBB15_3
.LBB15_7:
                                        # kill: def $al killed $al killed $eax
	retq
.LBB15_4:
	xorl	%eax, %eax
                                        # kill: def $al killed $al killed $eax
	retq
.Lfunc_end15:
	.size	equal_generic_list_b, .Lfunc_end15-equal_generic_list_b
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	xorl	%eax, %eax
	retq
.Lfunc_end16:
	.size	main, .Lfunc_end16-main
	.cfi_endproc
                                        # -- End function
	.ident	"Ubuntu clang version 22.1.2 (1ubuntu1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
