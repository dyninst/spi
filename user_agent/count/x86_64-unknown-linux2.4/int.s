	.file	"int.i"
	.local	inline_target
	.comm	inline_target,8,8
	.local	inline_fp_rtx
	.comm	inline_fp_rtx,8,8
	.local	parm_map
	.comm	parm_map,8,8
	.local	fp_addr_p
	.comm	fp_addr_p,8,8
	.local	fp_delta
	.comm	fp_delta,4,4
	.local	orig_asm_operands_vector
	.comm	orig_asm_operands_vector,8,8
	.local	copy_asm_operands_vector
	.comm	copy_asm_operands_vector,8,8
	.section	.rodata
	.align 8
.LC0:
	.string	"varargs function cannot be inline"
	.align 8
.LC1:
	.string	"function too large to be inline"
	.align 8
.LC2:
	.string	"function with large aggregate parameter cannot be inline"
	.align 8
.LC3:
	.string	"no prototype, and parameter address used; cannot be inline"
	.align 8
.LC4:
	.string	"address of an aggregate parameter is used; cannot be inline"
	.text
.globl function_cannot_inline_p
	.type	function_cannot_inline_p, @function
function_cannot_inline_p:
.LFB0:
	pushq	%rbp
.LCFI0:
	movq	%rsp, %rbp
.LCFI1:
	pushq	%r12
.LCFI2:
	pushq	%rbx
.LCFI3:
	subq	$16, %rsp
.LCFI4:
	movq	%rdi, %rbx
	movq	16(%rbx), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	tree_last
	movq	%rax, -24(%rbp)
	movq	96(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	list_length
	leal	8(%rax), %edx
	movzbl	26(%rbx), %eax
	shrb	$6, %al
	andl	$1, %eax
	movzbl	%al, %eax
	sall	$4, %eax
	leal	(%rdx,%rax), %eax
	sall	$3, %eax
	movl	%eax, -28(%rbp)
	movl	$0, %r12d
	cmpq	$0, -24(%rbp)
	je	.L2
	movq	-24(%rbp), %rax
	movq	40(%rax), %rdx
	movq	void_type_node(%rip), %rax
	cmpq	%rax, %rdx
	je	.L2
	movl	$.LC0, %eax
	jmp	.L3
.L2:
	movl	$0, %eax
	call	get_max_uid
	movl	-28(%rbp), %edx
	addl	%edx, %edx
	cmpl	%edx, %eax
	jle	.L4
	movl	$.LC1, %eax
	jmp	.L3
.L4:
	movq	96(%rbx), %rbx
	jmp	.L5
.L10:
	movq	16(%rbx), %rax
	movzbl	56(%rax), %eax
	cmpb	$26, %al
	jne	.L6
	movl	$.LC2, %eax
	jmp	.L3
.L6:
	cmpq	$0, -24(%rbp)
	jne	.L7
	movzbl	26(%rbx), %eax
	andl	$2, %eax
	testb	%al, %al
	je	.L7
	movl	$.LC3, %eax
	jmp	.L3
.L7:
	movq	16(%rbx), %rax
	movzbl	24(%rax), %eax
	cmpb	$19, %al
	je	.L8
	movq	16(%rbx), %rax
	movzbl	24(%rax), %eax
	cmpb	$20, %al
	jne	.L9
.L8:
	movq	120(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$37, %ax
	jne	.L9
	movl	$.LC4, %eax
	jmp	.L3
.L9:
	movq	8(%rbx), %rbx
.L5:
	testq	%rbx, %rbx
	jne	.L10
	movl	$0, %eax
	call	get_max_uid
	cmpl	-28(%rbp), %eax
	jle	.L11
	movl	$0, %r12d
	movl	$0, %eax
	call	get_first_nonparm_insn
	movq	%rax, %rbx
	jmp	.L12
.L16:
	movzwl	(%rbx), %eax
	cmpw	$13, %ax
	je	.L13
	movzwl	(%rbx), %eax
	cmpw	$14, %ax
	je	.L13
	movzwl	(%rbx), %eax
	cmpw	$15, %ax
	jne	.L14
.L13:
	addl	$1, %r12d
.L14:
	movq	24(%rbx), %rbx
.L12:
	testq	%rbx, %rbx
	je	.L15
	cmpl	-28(%rbp), %r12d
	jl	.L16
.L15:
	cmpl	-28(%rbp), %r12d
	jl	.L11
	movl	$.LC1, %eax
	jmp	.L3
.L11:
	movl	$0, %eax
.L3:
	addq	$16, %rsp
	popq	%rbx
	popq	%r12
	leave
.LCFI5:
	ret
.LFE0:
	.size	function_cannot_inline_p, .-function_cannot_inline_p
	.local	reg_map
	.comm	reg_map,8,8
	.local	label_map
	.comm	label_map,8,8
	.local	insn_map
	.comm	insn_map,8,8
	.local	parmdecl_map
	.comm	parmdecl_map,8,8
	.local	max_parm_reg
	.comm	max_parm_reg,4,4
	.local	first_parm_offset
	.comm	first_parm_offset,4,4
.globl save_for_inline
	.type	save_for_inline, @function
save_for_inline:
.LFB1:
	pushq	%rbp
.LCFI6:
	movq	%rsp, %rbp
.LCFI7:
	pushq	%rbx
.LCFI8:
	subq	$168, %rsp
.LCFI9:
	movq	%rdi, -152(%rbp)
	movq	return_label(%rip), %rax
	testq	%rax, %rax
	jne	.L18
	movl	$0, %eax
	call	gen_label_rtx
	movq	%rax, return_label(%rip)
	movq	return_label(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_label
.L18:
	movl	$0, %eax
	call	max_label_num
	movl	%eax, -56(%rbp)
	movl	$0, %eax
	call	get_first_label_num
	movl	%eax, -60(%rbp)
	movl	$0, %eax
	call	max_parm_reg_num
	movl	%eax, max_parm_reg(%rip)
	movl	$0, %eax
	call	max_reg_num
	movl	%eax, -64(%rbp)
	movl	max_parm_reg(%rip), %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	leaq	8(%rsp), %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, parmdecl_map(%rip)
	movl	max_parm_reg(%rip), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	parmdecl_map(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset
	movq	-152(%rbp), %rax
	movq	96(%rax), %rax
	movq	%rax, -48(%rbp)
	jmp	.L19
.L22:
	movq	-48(%rbp), %rax
	movq	120(%rax), %rax
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L20
	movq	parmdecl_map(%rip), %rdx
	movq	-72(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movq	-48(%rbp), %rax
	movq	%rax, (%rdx)
	movq	-48(%rbp), %rax
	movzbl	25(%rax), %edx
	andl	$-9, %edx
	movb	%dl, 25(%rax)
	jmp	.L21
.L20:
	movq	-48(%rbp), %rax
	movzbl	25(%rax), %edx
	orl	$8, %edx
	movb	%dl, 25(%rax)
.L21:
	movq	-48(%rbp), %rax
	movzbl	25(%rax), %edx
	orl	$32, %edx
	movb	%dl, 25(%rax)
	movq	-48(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -48(%rbp)
.L19:
	cmpq	$0, -48(%rbp)
	jne	.L22
	movl	current_function_args_size(%rip), %ecx
	movl	max_parm_reg(%rip), %ebx
	movl	-64(%rbp), %esi
	movl	-56(%rbp), %edx
	movl	-60(%rbp), %eax
	movl	%ecx, (%rsp)
	movl	%esi, %r9d
	movl	%ebx, %r8d
	movl	%edx, %ecx
	movl	%eax, %edx
	movl	$0, %esi
	movl	$0, %edi
	movl	$0, %eax
	call	gen_inline_header_rtx
	movq	%rax, -80(%rbp)
	movq	-80(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -84(%rbp)
	movl	$0, %eax
	call	preserve_data
	movl	$0, %eax
	call	get_insns
	movq	%rax, -32(%rbp)
	movq	-32(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$18, %ax
	je	.L23
	call	abort
.L23:
	movl	$18, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, -96(%rbp)
	movq	-32(%rbp), %rax
	movq	32(%rax), %rdx
	movq	-96(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-32(%rbp), %rax
	movl	40(%rax), %edx
	movq	-96(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-32(%rbp), %rax
	movl	8(%rax), %edx
	movq	-96(%rbp), %rax
	movl	%edx, 8(%rax)
	movq	-96(%rbp), %rax
	movq	$0, 16(%rax)
	movq	-96(%rbp), %rax
	movq	$0, 24(%rax)
	movq	-96(%rbp), %rax
	movq	%rax, -24(%rbp)
	movl	-64(%rbp), %eax
	addl	$1, %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	leaq	8(%rsp), %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, reg_map(%rip)
	movl	rtx_length+136(%rip), %eax
	subl	$1, %eax
	cltq
	addq	$2, %rax
	sall	$3, %eax
	movl	%eax, -100(%rbp)
	movl	-64(%rbp), %eax
	subl	$1, %eax
	movl	%eax, -52(%rbp)
	jmp	.L24
.L29:
	movq	reg_map(%rip), %rax
	movl	-52(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movq	$maybepermanent_obstack, -112(%rbp)
	movq	-112(%rbp), %rax
	movq	%rax, -120(%rbp)
	movl	-100(%rbp), %eax
	movl	%eax, -124(%rbp)
	movq	-120(%rbp), %rax
	movq	24(%rax), %rdx
	movl	-124(%rbp), %eax
	cltq
	addq	%rax, %rdx
	movq	-120(%rbp), %rax
	movq	32(%rax), %rax
	cmpq	%rax, %rdx
	jbe	.L26
	movl	-124(%rbp), %edx
	movq	-120(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	_obstack_newchunk
.L26:
	movl	-124(%rbp), %eax
	movslq	%eax, %rdx
	movq	regno_reg_rtx(%rip), %rax
	movl	-52(%rbp), %ecx
	movslq	%ecx, %rcx
	salq	$3, %rcx
	addq	%rcx, %rax
	movq	(%rax), %rcx
	movq	-120(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy
	movq	-120(%rbp), %rax
	movq	24(%rax), %rdx
	movl	-124(%rbp), %eax
	cltq
	addq	%rax, %rdx
	movq	-120(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-112(%rbp), %rax
	movq	%rax, -136(%rbp)
	movq	-136(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -144(%rbp)
	movq	-136(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rdx
	movq	-136(%rbp), %rax
	movl	44(%rax), %eax
	cltq
	addq	%rax, %rdx
	movq	-136(%rbp), %rax
	movl	44(%rax), %eax
	notl	%eax
	cltq
	andq	%rdx, %rax
	movq	%rax, %rdx
	movq	-136(%rbp), %rax
	movq	%rdx, 24(%rax)
	movq	-136(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, %rdx
	movq	-136(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rdx, %rcx
	subq	%rax, %rcx
	movq	-136(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdx
	movq	-136(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rdx, %rsi
	subq	%rax, %rsi
	movq	%rsi, %rax
	cmpq	%rax, %rcx
	jle	.L28
	movq	-136(%rbp), %rax
	movq	32(%rax), %rdx
	movq	-136(%rbp), %rax
	movq	%rdx, 24(%rax)
.L28:
	movq	-136(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-136(%rbp), %rax
	movq	%rdx, 16(%rax)
	movq	-144(%rbp), %rax
	movq	%rax, (%rbx)
	subl	$1, -52(%rbp)
.L24:
	cmpl	$55, -52(%rbp)
	jg	.L29
	movl	-64(%rbp), %eax
	subl	$56, %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	reg_map(%rip), %rax
	leaq	448(%rax), %rcx
	movq	regno_reg_rtx(%rip), %rax
	addq	$448, %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy
	movl	-60(%rbp), %eax
	movl	-56(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	leaq	8(%rsp), %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, label_map(%rip)
	movq	label_map(%rip), %rax
	movl	-60(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	negq	%rdx
	addq	%rdx, %rax
	movq	%rax, label_map(%rip)
	movl	-60(%rbp), %eax
	movl	%eax, -52(%rbp)
	jmp	.L30
.L31:
	movq	label_map(%rip), %rax
	movl	-52(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movl	$0, %eax
	call	gen_label_rtx
	movq	%rax, (%rbx)
	addl	$1, -52(%rbp)
.L30:
	movl	-52(%rbp), %eax
	cmpl	-56(%rbp), %eax
	jl	.L31
	movl	-84(%rbp), %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	leaq	8(%rsp), %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, insn_map(%rip)
	movl	-84(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	insn_map(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset
	movq	-32(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -32(%rbp)
	jmp	.L32
.L42:
	movq	$0, orig_asm_operands_vector(%rip)
	movq	$0, copy_asm_operands_vector(%rip)
	movq	-32(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	subl	$13, %eax
	cmpl	$5, %eax
	ja	.L33
	mov	%eax, %eax
	movq	.L38(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L38:
	.quad	.L34
	.quad	.L34
	.quad	.L34
	.quad	.L35
	.quad	.L36
	.quad	.L37
	.text
.L37:
	movq	-32(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$-6, %eax
	je	.L43
.L39:
	movl	$18, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, -40(%rbp)
	movq	-32(%rbp), %rax
	movq	32(%rax), %rdx
	movq	-40(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-32(%rbp), %rax
	movl	40(%rax), %edx
	movq	-40(%rbp), %rax
	movl	%edx, 40(%rax)
	jmp	.L41
.L34:
	movq	-32(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	movl	%eax, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, -40(%rbp)
	movq	-32(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_for_inline
	movq	-40(%rbp), %rdx
	movq	%rax, 32(%rdx)
	movq	-40(%rbp), %rax
	movl	$-1, 40(%rax)
	movq	-40(%rbp), %rax
	movq	$0, 48(%rax)
	movq	-32(%rbp), %rax
	movq	56(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_for_inline
	movq	-40(%rbp), %rdx
	movq	%rax, 56(%rdx)
	jmp	.L41
.L36:
	movq	label_map(%rip), %rdx
	movq	-32(%rbp), %rax
	movl	32(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movq	%rax, -40(%rbp)
	jmp	.L41
.L35:
	movl	$16, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, -40(%rbp)
	jmp	.L41
.L33:
	call	abort
.L41:
	movq	-32(%rbp), %rax
	movl	8(%rax), %edx
	movq	-40(%rbp), %rax
	movl	%edx, 8(%rax)
	movq	insn_map(%rip), %rdx
	movq	-32(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movq	-40(%rbp), %rax
	movq	%rax, (%rdx)
	movq	-24(%rbp), %rax
	movq	-40(%rbp), %rdx
	movq	%rdx, 24(%rax)
	movq	-40(%rbp), %rax
	movq	-24(%rbp), %rdx
	movq	%rdx, 16(%rax)
	movq	-40(%rbp), %rax
	movq	%rax, -24(%rbp)
	jmp	.L40
.L43:
	nop
.L40:
	movq	-32(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -32(%rbp)
.L32:
	cmpq	$0, -32(%rbp)
	jne	.L42
	movq	-24(%rbp), %rax
	movq	$0, 24(%rax)
	movl	$0, %eax
	call	get_first_nonparm_insn
	movq	-80(%rbp), %rdx
	movq	%rax, 24(%rdx)
	movl	$0, %eax
	call	get_insns
	movq	-80(%rbp), %rdx
	movq	%rax, 32(%rdx)
	movq	-152(%rbp), %rax
	movq	-80(%rbp), %rdx
	movq	%rdx, 136(%rax)
	movl	$0, %eax
	call	get_frame_size
	movq	-152(%rbp), %rdx
	movl	%eax, 128(%rdx)
	movq	-152(%rbp), %rax
	movzbl	26(%rax), %edx
	orl	$64, %edx
	movb	%dl, 26(%rax)
	movq	$0, parmdecl_map(%rip)
	movq	$0, label_map(%rip)
	movq	$0, reg_map(%rip)
	movq	$0, return_label(%rip)
	movq	-24(%rbp), %rdx
	movq	-96(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	set_new_first_and_last_insn
	movq	-8(%rbp), %rbx
	leave
.LCFI10:
	ret
.LFE1:
	.size	save_for_inline, .-save_for_inline
	.type	copy_for_inline, @function
copy_for_inline:
.LFB2:
	pushq	%rbp
.LCFI11:
	movq	%rsp, %rbp
.LCFI12:
	pushq	%r15
.LCFI13:
	pushq	%r14
.LCFI14:
	pushq	%r13
.LCFI15:
	pushq	%r12
.LCFI16:
	pushq	%rbx
.LCFI17:
	subq	$40, %rsp
.LCFI18:
	movq	%rdi, -72(%rbp)
	movq	-72(%rbp), %rbx
	testq	%rbx, %rbx
	jne	.L45
	movq	%rbx, %rax
	jmp	.L46
.L45:
	movzwl	(%rbx), %eax
	movzwl	%ax, %r13d
	leal	-22(%r13), %eax
	cmpl	$19, %eax
	ja	.L47
	mov	%eax, %eax
	movq	.L54(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L54:
	.quad	.L48
	.quad	.L47
	.quad	.L47
	.quad	.L49
	.quad	.L47
	.quad	.L47
	.quad	.L47
	.quad	.L47
	.quad	.L50
	.quad	.L50
	.quad	.L47
	.quad	.L50
	.quad	.L51
	.quad	.L47
	.quad	.L47
	.quad	.L52
	.quad	.L53
	.quad	.L50
	.quad	.L50
	.quad	.L50
	.text
.L50:
	movq	%rbx, %rax
	jmp	.L46
.L48:
	movq	-72(%rbp), %rax
	movq	32(%rax), %rdx
	movq	orig_asm_operands_vector(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L75
	movl	$22, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %rbx
	movq	-72(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, 8(%rbx)
	movq	-72(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, 16(%rbx)
	movq	-72(%rbp), %rax
	movl	24(%rax), %eax
	movl	%eax, 24(%rbx)
	movq	copy_asm_operands_vector(%rip), %rax
	movq	%rax, 32(%rbx)
	movq	-72(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, 40(%rbx)
	movq	%rbx, %rax
	jmp	.L46
.L52:
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L56
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L56
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L56
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L57
.L56:
	movq	%rbx, %rax
	jmp	.L46
.L57:
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$44, %ax
	jne	.L76
	movq	8(%rbx), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L77
	movq	8(%rbx), %rax
	movq	8(%rax), %rax
	movl	8(%rax), %eax
	cmpl	$14, %eax
	je	.L59
	movq	8(%rbx), %rax
	movq	8(%rax), %rax
	movl	8(%rax), %eax
	cmpl	$14, %eax
	jne	.L78
.L59:
	movq	8(%rbx), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L60
	movq	8(%rbx), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L60
	movq	8(%rbx), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L60
	movq	8(%rbx), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L79
.L60:
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L80
	movq	8(%rbx), %rax
	movl	8(%rax), %eax
	cmpl	$14, %eax
	je	.L61
	movq	8(%rbx), %rax
	movl	8(%rax), %eax
	cmpl	$14, %eax
	jne	.L81
.L61:
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L62
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L62
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L62
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L82
.L62:
	movq	%rbx, %rax
	jmp	.L46
.L53:
	movq	label_map(%rip), %rdx
	movq	-72(%rbp), %rax
	movq	8(%rax), %rax
	movl	32(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rdx
	movq	-72(%rbp), %rax
	movzbl	2(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, %esi
	movl	$38, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L46
.L51:
	movl	8(%rbx), %eax
	cmpl	$55, %eax
	jle	.L63
	movq	reg_map(%rip), %rax
	movl	8(%rbx), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	jmp	.L46
.L63:
	movq	%rbx, %rax
	jmp	.L46
.L49:
	movq	8(%rbx), %rax
	movq	%rax, -56(%rbp)
	movq	-56(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L83
	movq	-56(%rbp), %rax
	movl	8(%rax), %edx
	movl	max_parm_reg(%rip), %eax
	cmpl	%eax, %edx
	jge	.L84
	movq	-56(%rbp), %rax
	movl	8(%rax), %eax
	cmpl	$55, %eax
	jle	.L85
	movq	parmdecl_map(%rip), %rdx
	movq	-56(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L86
	movq	parmdecl_map(%rip), %rdx
	movq	-56(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movzbl	25(%rax), %edx
	andl	$-33, %edx
	movb	%dl, 25(%rax)
	jmp	.L47
.L75:
	nop
	jmp	.L47
.L76:
	nop
	jmp	.L47
.L77:
	nop
	jmp	.L47
.L78:
	nop
	jmp	.L47
.L79:
	nop
	jmp	.L47
.L80:
	nop
	jmp	.L47
.L81:
	nop
	jmp	.L47
.L82:
	nop
	jmp	.L47
.L83:
	nop
	jmp	.L47
.L84:
	nop
	jmp	.L47
.L85:
	nop
	jmp	.L47
.L86:
	nop
.L47:
	movl	%r13d, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %rbx
	movl	%r13d, %eax
	cltq
	movl	rtx_length(,%rax,4), %eax
	addl	$1, %eax
	cltq
	leaq	0(,%rax,4), %rdx
	movq	-72(%rbp), %rax
	movq	%rax, %rsi
	movq	%rbx, %rdi
	call	memcpy
	movl	%r13d, %eax
	cltq
	movq	rtx_format(,%rax,8), %r15
	movl	$0, %r12d
	jmp	.L65
.L73:
	movzbl	(%r15), %eax
	movsbl	%al, %eax
	addq	$1, %r15
	cmpl	$101, %eax
	je	.L68
	cmpl	$117, %eax
	je	.L69
	cmpl	$69, %eax
	je	.L67
	jmp	.L66
.L68:
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_for_inline
	movslq	%r12d, %rdx
	movq	%rax, 8(%rbx,%rdx,8)
	jmp	.L66
.L69:
	movq	insn_map(%rip), %rdx
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	jmp	.L46
.L67:
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	testq	%rax, %rax
	je	.L87
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	je	.L88
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	leaq	8(%rax), %rdx
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	movl	$0, %eax
	call	gen_rtvec_v
	movslq	%r12d, %rdx
	movq	%rax, 8(%rbx,%rdx,8)
	movl	$0, %r14d
	jmp	.L71
.L72:
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movq	%rax, -80(%rbp)
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movslq	%r14d, %rdx
	movq	8(%rax,%rdx,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_for_inline
	movslq	%r14d, %rdx
	movq	-80(%rbp), %rcx
	movq	%rax, 8(%rcx,%rdx,8)
	addl	$1, %r14d
.L71:
	movl	%r14d, %edx
	movslq	%r12d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	cmpl	%eax, %edx
	jb	.L72
	jmp	.L66
.L87:
	nop
	jmp	.L66
.L88:
	nop
.L66:
	addl	$1, %r12d
.L65:
	movl	%r13d, %eax
	cltq
	movl	rtx_length(,%rax,4), %eax
	cmpl	%r12d, %eax
	jg	.L73
	cmpl	$22, %r13d
	jne	.L74
	movq	orig_asm_operands_vector(%rip), %rax
	testq	%rax, %rax
	jne	.L74
	movq	-72(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, orig_asm_operands_vector(%rip)
	movq	32(%rbx), %rax
	movq	%rax, copy_asm_operands_vector(%rip)
.L74:
	movq	%rbx, %rax
.L46:
	addq	$40, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	leave
.LCFI19:
	ret
.LFE2:
	.size	copy_for_inline, .-copy_for_inline
.globl expand_inline_function
	.type	expand_inline_function, @function
expand_inline_function:
.LFB3:
	pushq	%rbp
.LCFI20:
	movq	%rsp, %rbp
.LCFI21:
	pushq	%r12
.LCFI22:
	pushq	%rbx
.LCFI23:
	subq	$256, %rsp
.LCFI24:
	movq	%rdi, -232(%rbp)
	movq	%rsi, -240(%rbp)
	movq	%rdx, -248(%rbp)
	movl	%ecx, -252(%rbp)
	movq	%r8, -264(%rbp)
	movq	%r9, -272(%rbp)
	movq	-232(%rbp), %rax
	movq	136(%rax), %rax
	movq	%rax, -104(%rbp)
	movq	-104(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -112(%rbp)
	movq	-104(%rbp), %rax
	movl	64(%rax), %eax
	addl	$1, %eax
	movl	%eax, -116(%rbp)
	movq	-104(%rbp), %rax
	movl	40(%rax), %eax
	movl	%eax, -120(%rbp)
	movq	-104(%rbp), %rax
	movl	48(%rax), %eax
	movl	%eax, -124(%rbp)
	movq	$0, -48(%rbp)
	movq	$0, -56(%rbp)
	movq	$0, -64(%rbp)
	cmpl	$55, -116(%rbp)
	jg	.L90
	call	abort
.L90:
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	list_length
	movl	%eax, -128(%rbp)
	movl	$8, first_parm_offset(%rip)
	movq	-240(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	list_length
	cmpl	-128(%rbp), %eax
	je	.L91
	movq	$-1, %rax
	jmp	.L92
.L91:
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	movq	%rax, -24(%rbp)
	movq	-240(%rbp), %rax
	movq	%rax, -32(%rbp)
	jmp	.L93
.L96:
	movq	-32(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, -136(%rbp)
	movq	-24(%rbp), %rax
	movq	96(%rax), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -140(%rbp)
	movq	-136(%rbp), %rax
	movq	16(%rax), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	cmpl	-140(%rbp), %eax
	je	.L94
	movq	$-1, %rax
	jmp	.L92
.L94:
	cmpl	$26, -140(%rbp)
	jne	.L95
	movq	-136(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	cmpq	%rax, %rdx
	je	.L95
	movq	$-1, %rax
	jmp	.L92
.L95:
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -24(%rbp)
	movq	-32(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -32(%rbp)
.L93:
	cmpq	$0, -24(%rbp)
	jne	.L96
	movl	$0, %edi
	movl	$0, %eax
	call	pushlevel
	movl	$0, %edi
	movl	$0, %eax
	call	expand_start_bindings
	movl	-128(%rbp), %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, -152(%rbp)
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	movq	%rax, -24(%rbp)
	movq	-240(%rbp), %rax
	movq	%rax, -32(%rbp)
	movl	$0, %ebx
	jmp	.L97
.L104:
	movq	-32(%rbp), %rax
	movq	40(%rax), %rax
	movq	%rax, -160(%rbp)
	movq	-24(%rbp), %rax
	movq	96(%rax), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -164(%rbp)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -168(%rbp)
	movq	-24(%rbp), %rax
	movl	40(%rax), %edx
	movq	-24(%rbp), %rax
	movq	32(%rax), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_note
	movq	-24(%rbp), %rax
	movzbl	26(%rax), %eax
	andl	$2, %eax
	testb	%al, %al
	je	.L98
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	int_size_in_bytes
	movl	%eax, -172(%rbp)
	movl	-172(%rbp), %edx
	movl	-164(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	movl	$0, %eax
	call	assign_stack_local
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rax
	movq	8(%rax), %rdx
	movq	-24(%rbp), %rax
	movl	56(%rax), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	movl	$0, %eax
	call	memory_address_p
	testl	%eax, %eax
	jne	.L99
	movq	-72(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx
	movq	%rax, %rdx
	movq	-72(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	change_address
	movq	%rax, -72(%rbp)
.L99:
	movq	-72(%rbp), %rcx
	movq	-160(%rbp), %rax
	movl	$0, %edx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	store_expr
	jmp	.L100
.L98:
	movq	-24(%rbp), %rax
	movzbl	25(%rax), %eax
	andl	$32, %eax
	testb	%al, %al
	je	.L101
	movq	-24(%rbp), %rax
	movzbl	25(%rax), %eax
	andl	$8, %eax
	testb	%al, %al
	je	.L102
.L101:
	movl	-164(%rbp), %eax
	movl	%eax, %edi
	movl	$0, %eax
	call	gen_reg_rtx
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rcx
	movq	-160(%rbp), %rax
	movl	$0, %edx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	store_expr
	jmp	.L100
.L102:
	movl	-164(%rbp), %edx
	movq	-160(%rbp), %rax
	movl	$0, %ecx
	movl	$0, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	expand_expr
	movq	%rax, -72(%rbp)
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	je	.L100
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L100
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L100
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L100
	movq	-72(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	je	.L100
	movq	-72(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_to_reg
	movq	%rax, -72(%rbp)
.L100:
	movl	-164(%rbp), %eax
	cmpl	-168(%rbp), %eax
	je	.L103
	movq	-72(%rbp), %rdx
	movl	-168(%rbp), %eax
	movq	%rdx, %rsi
	movl	%eax, %edi
	movl	$0, %eax
	call	convert_to_mode
	movq	%rax, -72(%rbp)
.L103:
	movslq	%ebx, %rax
	salq	$3, %rax
	addq	-152(%rbp), %rax
	movq	-72(%rbp), %rdx
	movq	%rdx, (%rax)
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -24(%rbp)
	movq	-32(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -32(%rbp)
	addl	$1, %ebx
.L97:
	cmpq	$0, -24(%rbp)
	jne	.L104
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	movq	-152(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_parm_decls
	movl	$0, %eax
	call	emit_queue
	movl	$0, %eax
	call	do_pending_stack_adjust
	cmpq	$0, -272(%rbp)
	je	.L105
	movq	struct_value_rtx(%rip), %rax
	movzwl	(%rax), %eax
	cmpw	$37, %ax
	jne	.L106
	movq	-272(%rbp), %rax
	movq	%rax, %rsi
	movl	$4, %edi
	movl	$0, %eax
	call	force_reg
	movq	%rax, -64(%rbp)
	jmp	.L105
.L106:
	movq	struct_value_rtx(%rip), %rax
	movq	%rax, -64(%rbp)
	movq	-272(%rbp), %rdx
	movq	-64(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_move_insn
.L105:
	movl	-116(%rbp), %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, reg_map(%rip)
	movl	-116(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	reg_map(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	testq	%rax, %rax
	je	.L107
	movq	-232(%rbp), %rax
	movq	96(%rax), %rax
	movq	%rax, -184(%rbp)
	movq	-104(%rbp), %rax
	movl	72(%rax), %eax
	movl	%eax, -188(%rbp)
	movl	-188(%rbp), %eax
	leal	3(%rax), %edx
	testl	%eax, %eax
	cmovs	%edx, %eax
	sarl	$2, %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, parm_map(%rip)
	movl	-188(%rbp), %eax
	leal	3(%rax), %edx
	testl	%eax, %eax
	cmovs	%edx, %eax
	sarl	$2, %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	parm_map(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset
	movq	parm_map(%rip), %rdx
	movl	first_parm_offset(%rip), %eax
	leal	3(%rax), %ecx
	testl	%eax, %eax
	cmovs	%ecx, %eax
	sarl	$2, %eax
	cltq
	salq	$3, %rax
	negq	%rax
	leaq	(%rdx,%rax), %rax
	movq	%rax, parm_map(%rip)
	movq	-184(%rbp), %rax
	movq	%rax, -24(%rbp)
	movl	$0, %ebx
	jmp	.L108
.L116:
	movq	-24(%rbp), %rax
	movl	80(%rax), %eax
	testl	%eax, %eax
	js	.L109
	movq	parm_map(%rip), %rdx
	movq	-24(%rbp), %rax
	movl	80(%rax), %eax
	leal	31(%rax), %ecx
	testl	%eax, %eax
	cmovs	%ecx, %eax
	sarl	$5, %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movslq	%ebx, %rax
	salq	$3, %rax
	addq	-152(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, (%rdx)
	jmp	.L110
.L109:
	movq	-24(%rbp), %rax
	movq	120(%rax), %rax
	movq	%rax, -200(%rbp)
	movq	$0, -80(%rbp)
	movq	-200(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$37, %ax
	jne	.L111
	movq	-200(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -200(%rbp)
	movq	-200(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$44, %ax
	jne	.L112
	movq	-200(%rbp), %rax
	movq	8(%rax), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L113
	movq	-200(%rbp), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L113
	movq	-200(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -80(%rbp)
	jmp	.L112
.L113:
	movq	-200(%rbp), %rax
	movq	16(%rax), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L112
	movq	-200(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L112
	movq	-200(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -80(%rbp)
.L112:
	cmpq	$0, -80(%rbp)
	je	.L114
	movq	parm_map(%rip), %rdx
	movq	-80(%rbp), %rax
	movl	8(%rax), %eax
	leal	3(%rax), %ecx
	testl	%eax, %eax
	cmovs	%ecx, %eax
	sarl	$2, %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movslq	%ebx, %rax
	salq	$3, %rax
	addq	-152(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, (%rdx)
	jmp	.L110
.L114:
	call	abort
.L111:
	movq	-200(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	je	.L110
	call	abort
.L110:
	movq	-24(%rbp), %rax
	movq	120(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L115
	movq	reg_map(%rip), %rdx
	movq	-24(%rbp), %rax
	movq	120(%rax), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movslq	%ebx, %rax
	salq	$3, %rax
	addq	-152(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, (%rdx)
.L115:
	movq	-24(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -24(%rbp)
	addl	$1, %ebx
.L108:
	cmpq	$0, -24(%rbp)
	jne	.L116
	cmpq	$0, -64(%rbp)
	je	.L156
	movq	struct_value_incoming_rtx(%rip), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	je	.L157
	movq	struct_value_incoming_rtx(%rip), %rax
	movzwl	(%rax), %eax
	cmpw	$37, %ax
	jne	.L118
	movq	struct_value_incoming_rtx(%rip), %rax
	movq	8(%rax), %rax
	movq	8(%rax), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L118
	movq	struct_value_incoming_rtx(%rip), %rax
	movq	8(%rax), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L118
	movq	parm_map(%rip), %rdx
	movq	struct_value_incoming_rtx(%rip), %rax
	movq	8(%rax), %rax
	movq	16(%rax), %rax
	movl	8(%rax), %eax
	leal	3(%rax), %ecx
	testl	%eax, %eax
	cmovs	%ecx, %eax
	sarl	$2, %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movq	-64(%rbp), %rax
	movq	%rax, (%rdx)
	jmp	.L119
.L118:
	call	abort
.L107:
	movq	$0, parm_map(%rip)
	jmp	.L119
.L156:
	nop
	jmp	.L119
.L157:
	nop
.L119:
	movl	-120(%rbp), %eax
	movl	-124(%rbp), %edx
	movl	%edx, %ecx
	subl	%eax, %ecx
	movl	%ecx, %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, label_map(%rip)
	movq	label_map(%rip), %rax
	movl	-120(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	negq	%rdx
	addq	%rdx, %rax
	movq	%rax, label_map(%rip)
	movl	-120(%rbp), %ebx
	jmp	.L120
.L121:
	movq	label_map(%rip), %rax
	movslq	%ebx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %r12
	movl	$0, %eax
	call	gen_label_rtx
	movq	%rax, (%r12)
	addl	$1, %ebx
.L120:
	cmpl	-124(%rbp), %ebx
	jl	.L121
	movq	-104(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	addq	$15, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	subq	%rax, %rsp
	movq	%rsp, %rax
	addq	$15, %rax
	shrq	$4, %rax
	salq	$4, %rax
	movq	%rax, insn_map(%rip)
	movq	-104(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	insn_map(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset
	cmpq	$0, -272(%rbp)
	jne	.L122
	movq	-264(%rbp), %rax
	movzbl	56(%rax), %eax
	testb	%al, %al
	jne	.L123
.L122:
	movq	$0, inline_target(%rip)
	jmp	.L124
.L123:
	movq	-264(%rbp), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -204(%rbp)
	movq	-232(%rbp), %rax
	movq	88(%rax), %rax
	movzbl	56(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -208(%rbp)
	cmpq	$0, -248(%rbp)
	je	.L125
	movq	-248(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L125
	movq	-248(%rbp), %rax
	movzbl	2(%rax), %eax
	movzbl	%al, %eax
	cmpl	-204(%rbp), %eax
	jne	.L125
	movq	-248(%rbp), %rax
	movq	%rax, inline_target(%rip)
	jmp	.L126
.L125:
	movl	-204(%rbp), %eax
	movl	%eax, %edi
	movl	$0, %eax
	call	gen_reg_rtx
	movq	%rax, -248(%rbp)
	movq	-248(%rbp), %rax
	movq	%rax, inline_target(%rip)
.L126:
	movl	-208(%rbp), %eax
	cmpl	-204(%rbp), %eax
	je	.L124
	movq	-248(%rbp), %rdx
	movl	-208(%rbp), %eax
	movl	$0, %ecx
	movl	%eax, %esi
	movl	$35, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, inline_target(%rip)
.L124:
	movl	$0, %eax
	call	get_frame_size
	movl	%eax, fp_delta(%rip)
	movl	fp_delta(%rip), %eax
	negl	%eax
	movl	%eax, fp_delta(%rip)
	movl	fp_delta(%rip), %eax
	movl	%eax, fp_delta(%rip)
	movl	fp_delta(%rip), %edx
	movq	frame_pointer_rtx(%rip), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %rsi
	movl	$4, %edi
	movl	$0, %eax
	call	copy_to_mode_reg
	movq	%rax, inline_fp_rtx(%rip)
	movq	-232(%rbp), %rax
	movl	128(%rax), %eax
	movl	%eax, %esi
	movl	$0, %edi
	movl	$0, %eax
	call	assign_stack_local
	movq	-112(%rbp), %rax
	movq	%rax, -40(%rbp)
	jmp	.L127
.L150:
	movq	$0, -96(%rbp)
	movq	$0, orig_asm_operands_vector(%rip)
	movq	$0, copy_asm_operands_vector(%rip)
	movq	-40(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	subl	$13, %eax
	cmpl	$5, %eax
	ja	.L128
	mov	%eax, %eax
	movq	.L135(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L135:
	.quad	.L129
	.quad	.L130
	.quad	.L131
	.quad	.L132
	.quad	.L133
	.quad	.L134
	.text
.L129:
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, -216(%rbp)
	cmpq	$0, -56(%rbp)
	je	.L136
	movq	-216(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$25, %ax
	jne	.L137
	movq	-216(%rbp), %rax
	movq	8(%rax), %rdx
	movq	stack_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	je	.L136
.L137:
	movq	-216(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$25, %ax
	jne	.L138
	movq	-216(%rbp), %rax
	movq	16(%rax), %rax
	movq	-56(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	rtx_equal_p
	testl	%eax, %eax
	je	.L138
	movq	-216(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	-56(%rbp), %rdx
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movl	$0, %esi
	movl	$25, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_insn
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movzbl	3(%rax), %edx
	orl	$64, %edx
	movb	%dl, 3(%rax)
	movq	$0, -56(%rbp)
	jmp	.L139
.L138:
	movq	-216(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$26, %ax
	jne	.L140
	movq	-216(%rbp), %rax
	movq	8(%rax), %rax
	movq	-56(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	rtx_equal_p
	testl	%eax, %eax
	je	.L140
	movq	inline_target(%rip), %rax
	movq	-56(%rbp), %rdx
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movl	$0, %esi
	movl	$25, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_insn
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movzbl	3(%rax), %edx
	orl	$64, %edx
	movb	%dl, 3(%rax)
	movq	$0, -56(%rbp)
	jmp	.L139
.L140:
	movq	$0, -56(%rbp)
.L136:
	movq	$0, -88(%rbp)
	movq	-216(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$26, %ax
	jne	.L141
	movq	-216(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L141
	movq	-216(%rbp), %rax
	movq	8(%rax), %rax
	movzbl	3(%rax), %eax
	andl	$64, %eax
	testb	%al, %al
	jne	.L158
.L141:
	movq	-216(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$25, %ax
	jne	.L142
	movq	-216(%rbp), %rax
	movq	8(%rax), %rdx
	movq	cc0_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L142
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	try_fold_cc0
	movq	%rax, -96(%rbp)
.L142:
	cmpq	$0, -96(%rbp)
	je	.L143
	movq	-96(%rbp), %rax
	movq	%rax, -40(%rbp)
	jmp	.L139
.L143:
	movq	-216(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_insn
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movzbl	3(%rax), %edx
	orl	$64, %edx
	movb	%dl, 3(%rax)
	jmp	.L139
.L130:
	movq	$0, -56(%rbp)
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$29, %ax
	jne	.L145
	cmpq	$0, -48(%rbp)
	jne	.L146
	movl	$0, %eax
	call	gen_label_rtx
	movq	%rax, -48(%rbp)
.L146:
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_jump
	jmp	.L139
.L145:
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_jump_insn
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movzbl	3(%rax), %edx
	orl	$64, %edx
	movb	%dl, 3(%rax)
	jmp	.L139
.L131:
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_call_insn
	movq	%rax, -88(%rbp)
	movq	-88(%rbp), %rax
	movzbl	3(%rax), %edx
	orl	$64, %edx
	movb	%dl, 3(%rax)
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$25, %ax
	jne	.L159
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movq	8(%rax), %rax
	movq	%rax, -56(%rbp)
	jmp	.L139
.L133:
	movq	label_map(%rip), %rdx
	movq	-40(%rbp), %rax
	movl	32(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_label
	movq	%rax, -88(%rbp)
	movq	$0, -56(%rbp)
	jmp	.L139
.L132:
	movl	$0, %eax
	call	emit_barrier
	movq	%rax, -88(%rbp)
	jmp	.L139
.L134:
	movq	-40(%rbp), %rax
	movl	40(%rax), %eax
	cmpl	$-6, %eax
	je	.L148
	movq	-40(%rbp), %rax
	movl	40(%rax), %edx
	movq	-40(%rbp), %rax
	movq	32(%rax), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_note
	movq	%rax, -88(%rbp)
	jmp	.L139
.L148:
	movq	$0, -88(%rbp)
	jmp	.L139
.L128:
	call	abort
.L158:
	nop
	jmp	.L139
.L159:
	nop
.L139:
	movq	insn_map(%rip), %rdx
	movq	-40(%rbp), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	addq	%rax, %rdx
	movq	-88(%rbp), %rax
	movq	%rax, (%rdx)
	movq	-40(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -40(%rbp)
.L127:
	cmpq	$0, -40(%rbp)
	jne	.L150
	cmpq	$0, -48(%rbp)
	je	.L151
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_label
.L151:
	movq	-232(%rbp), %rax
	movq	112(%rax), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_decl_tree
	movl	$0, %eax
	call	getdecls
	movl	$1, %edx
	movl	$1, %esi
	movl	%eax, %edi
	movl	$0, %eax
	call	expand_end_bindings
	movl	$0, %edx
	movl	$1, %esi
	movl	$1, %edi
	movl	$0, %eax
	call	poplevel
	movq	$0, reg_map(%rip)
	movq	$0, label_map(%rip)
	cmpl	$0, -252(%rbp)
	jne	.L152
	movq	-264(%rbp), %rax
	movzbl	56(%rax), %eax
	testb	%al, %al
	jne	.L153
.L152:
	movl	$0, %eax
	jmp	.L92
.L153:
	cmpq	$0, -272(%rbp)
	je	.L154
	cmpq	$0, -248(%rbp)
	je	.L155
	movq	-248(%rbp), %rax
	jmp	.L92
.L155:
	movq	-272(%rbp), %rax
	movq	%rax, %rsi
	movl	$26, %edi
	movl	$0, %eax
	call	memory_address
	movq	%rax, %rdx
	movl	$26, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L92
.L154:
	movq	-248(%rbp), %rax
.L92:
	leaq	-16(%rbp), %rsp
	addq	$0, %rsp
	popq	%rbx
	popq	%r12
	leave
.LCFI25:
	ret
.LFE3:
	.size	expand_inline_function, .-expand_inline_function
	.type	copy_parm_decls, @function
copy_parm_decls:
.LFB4:
	pushq	%rbp
.LCFI26:
	movq	%rsp, %rbp
.LCFI27:
	pushq	%r13
.LCFI28:
	pushq	%r12
.LCFI29:
	pushq	%rbx
.LCFI30:
	subq	$24, %rsp
.LCFI31:
	movq	%rdi, -40(%rbp)
	movq	%rsi, -48(%rbp)
	movq	-40(%rbp), %rbx
	movl	$0, %r13d
	jmp	.L161
.L162:
	movq	16(%rbx), %rdx
	movq	64(%rbx), %rax
	movq	%rax, %rsi
	movl	$43, %edi
	movl	$0, %eax
	call	build_decl
	movq	%rax, %rdi
	movl	$0, %eax
	call	pushdecl
	movq	%rax, %r12
	movzbl	26(%r12), %eax
	orl	$-128, %eax
	movb	%al, 26(%r12)
	movslq	%r13d, %rax
	salq	$3, %rax
	addq	-48(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, 120(%r12)
	movq	8(%rbx), %rbx
	addl	$1, %r13d
.L161:
	testq	%rbx, %rbx
	jne	.L162
	addq	$24, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	leave
.LCFI32:
	ret
.LFE4:
	.size	copy_parm_decls, .-copy_parm_decls
	.type	copy_decl_tree, @function
copy_decl_tree:
.LFB5:
	pushq	%rbp
.LCFI33:
	movq	%rsp, %rbp
.LCFI34:
	subq	$32, %rsp
.LCFI35:
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movl	$0, %edi
	movl	$0, %eax
	call	pushlevel
	movq	-24(%rbp), %rax
	movq	56(%rax), %rax
	movq	%rax, -8(%rbp)
	jmp	.L164
.L168:
	movq	-8(%rbp), %rax
	movq	16(%rax), %rdx
	movq	-8(%rbp), %rax
	movq	64(%rax), %rcx
	movq	-8(%rbp), %rax
	movzbl	24(%rax), %eax
	movzbl	%al, %eax
	movq	%rcx, %rsi
	movl	%eax, %edi
	movl	$0, %eax
	call	build_decl
	movq	%rax, -16(%rbp)
	movq	-8(%rbp), %rax
	movl	40(%rax), %edx
	movq	-16(%rbp), %rax
	movl	%edx, 40(%rax)
	movq	-8(%rbp), %rax
	movq	32(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 32(%rax)
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	testq	%rax, %rax
	je	.L165
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$37, %ax
	jne	.L166
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L167
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L167
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L167
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L166
.L167:
	movq	-8(%rbp), %rax
	movq	120(%rax), %rdx
	movq	-16(%rbp), %rax
	movq	%rdx, 120(%rax)
	jmp	.L165
.L166:
	movq	-8(%rbp), %rax
	movq	120(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	-16(%rbp), %rdx
	movq	%rax, 120(%rdx)
.L165:
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	movl	%eax, %edx
	andl	$1, %edx
	movq	-16(%rbp), %rax
	movl	%edx, %ecx
	andl	$1, %ecx
	movzbl	25(%rax), %edx
	andl	$-2, %edx
	orl	%ecx, %edx
	movb	%dl, 25(%rax)
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	shrb	$2, %al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	leal	0(,%rax,4), %ecx
	movzbl	25(%rdx), %eax
	andl	$-5, %eax
	orl	%ecx, %eax
	movb	%al, 25(%rdx)
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	shrb	%al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	leal	(%rax,%rax), %ecx
	movzbl	25(%rdx), %eax
	andl	$-3, %eax
	orl	%ecx, %eax
	movb	%al, 25(%rdx)
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	shrb	$6, %al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	movl	%eax, %ecx
	sall	$6, %ecx
	movzbl	25(%rdx), %eax
	andl	$-65, %eax
	orl	%ecx, %eax
	movb	%al, 25(%rdx)
	movq	-8(%rbp), %rax
	movzbl	26(%rax), %eax
	shrb	%al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	leal	(%rax,%rax), %ecx
	movzbl	26(%rdx), %eax
	andl	$-3, %eax
	orl	%ecx, %eax
	movb	%al, 26(%rdx)
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	shrb	$5, %al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	movl	%eax, %ecx
	sall	$5, %ecx
	movzbl	25(%rdx), %eax
	andl	$-33, %eax
	orl	%ecx, %eax
	movb	%al, 25(%rdx)
	movq	-8(%rbp), %rax
	movzbl	25(%rax), %eax
	shrb	$3, %al
	andl	$1, %eax
	movq	-16(%rbp), %rdx
	andl	$1, %eax
	leal	0(,%rax,8), %ecx
	movzbl	25(%rdx), %eax
	andl	$-9, %eax
	orl	%ecx, %eax
	movb	%al, 25(%rdx)
	movq	-16(%rbp), %rax
	movzbl	26(%rax), %edx
	orl	$-128, %edx
	movb	%dl, 26(%rax)
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	pushdecl
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
.L164:
	cmpq	$0, -8(%rbp)
	jne	.L168
	movq	-24(%rbp), %rax
	movq	48(%rax), %rax
	movq	%rax, -8(%rbp)
	jmp	.L169
.L170:
	movl	-28(%rbp), %eax
	leal	1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_decl_tree
	movq	-8(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -8(%rbp)
.L169:
	cmpq	$0, -8(%rbp)
	jne	.L170
	cmpl	$0, -28(%rbp)
	setg	%al
	movzbl	%al, %eax
	movl	$0, %edx
	movl	$0, %esi
	movl	%eax, %edi
	movl	$0, %eax
	call	poplevel
	leave
.LCFI36:
	ret
.LFE5:
	.size	copy_decl_tree, .-copy_decl_tree
	.type	copy_rtx_and_substitute, @function
copy_rtx_and_substitute:
.LFB6:
	pushq	%rbp
.LCFI37:
	movq	%rsp, %rbp
.LCFI38:
	pushq	%r15
.LCFI39:
	pushq	%r14
.LCFI40:
	pushq	%r13
.LCFI41:
	pushq	%r12
.LCFI42:
	pushq	%rbx
.LCFI43:
	subq	$72, %rsp
.LCFI44:
	movq	%rdi, %r12
	testq	%r12, %r12
	jne	.L172
	movl	$0, %eax
	jmp	.L173
.L172:
	movzwl	(%r12), %eax
	movzwl	%ax, %r15d
	movzbl	2(%r12), %eax
	movzbl	%al, %r14d
	leal	-17(%r15), %eax
	cmpl	$27, %eax
	ja	.L174
	mov	%eax, %eax
	movq	.L184(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L184:
	.quad	.L175
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L176
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L177
	.quad	.L178
	.quad	.L179
	.quad	.L179
	.quad	.L174
	.quad	.L179
	.quad	.L180
	.quad	.L174
	.quad	.L174
	.quad	.L181
	.quad	.L182
	.quad	.L179
	.quad	.L179
	.quad	.L174
	.quad	.L174
	.quad	.L174
	.quad	.L183
	.text
.L180:
	movl	8(%r12), %eax
	movl	%eax, -68(%rbp)
	cmpl	$55, -68(%rbp)
	jg	.L185
	movzbl	3(%r12), %eax
	andl	$64, %eax
	testb	%al, %al
	je	.L186
	movq	inline_target(%rip), %rax
	testq	%rax, %rax
	jne	.L187
	call	abort
.L187:
	movq	inline_target(%rip), %rax
	movzbl	2(%rax), %eax
	movzbl	%al, %eax
	cmpl	%r14d, %eax
	jne	.L188
	movq	inline_target(%rip), %rax
	jmp	.L173
.L188:
	movq	inline_target(%rip), %rax
	movl	$0, %ecx
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$35, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L186:
	cmpl	$14, -68(%rbp)
	jne	.L189
	movl	fp_delta(%rip), %eax
	movl	%eax, %esi
	movq	%r12, %rdi
	movl	$0, %eax
	call	plus_constant
	jmp	.L173
.L189:
	movq	%r12, %rax
	jmp	.L173
.L185:
	movq	reg_map(%rip), %rax
	movl	-68(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	jne	.L190
	movq	reg_map(%rip), %rax
	movl	-68(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	leaq	(%rax,%rdx), %rbx
	movl	%r14d, %edi
	movl	$0, %eax
	call	gen_reg_rtx
	movq	%rax, (%rbx)
.L190:
	movq	reg_map(%rip), %rax
	movl	-68(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	jmp	.L173
.L175:
	movq	label_map(%rip), %rax
	movl	32(%r12), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	jmp	.L173
.L182:
	movl	$38, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %rbx
	movl	%r14d, %eax
	movb	%al, 2(%rbx)
	movq	label_map(%rip), %rdx
	movq	8(%r12), %rax
	movl	32(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rax
	movq	%rax, 8(%rbx)
	movq	%rbx, %rax
	jmp	.L173
.L179:
	movq	%r12, %rax
	jmp	.L173
.L176:
	movq	32(%r12), %rdx
	movq	orig_asm_operands_vector(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L251
	movl	$22, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movq	%rax, 8(%rbx)
	movq	16(%r12), %rax
	movq	%rax, 16(%rbx)
	movl	24(%r12), %eax
	movl	%eax, 24(%rbx)
	movq	copy_asm_operands_vector(%rip), %rax
	movq	%rax, 32(%rbx)
	movq	40(%r12), %rax
	movq	%rax, 40(%rbx)
	movq	%rbx, %rax
	jmp	.L173
.L177:
	movl	optimize(%rip), %eax
	testl	%eax, %eax
	je	.L192
	movl	flag_no_function_cse(%rip), %eax
	testl	%eax, %eax
	je	.L252
.L192:
	movq	16(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movq	8(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movq	8(%r12), %rax
	movzbl	2(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rdx
	movzbl	2(%r12), %eax
	movzbl	%al, %eax
	movq	%rbx, %rcx
	movl	%eax, %esi
	movl	$28, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L183:
	movq	8(%r12), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	je	.L194
	movq	16(%r12), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L195
.L194:
	movq	8(%r12), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	je	.L196
	movq	8(%r12), %rdx
	movq	arg_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L197
.L196:
	movq	16(%r12), %rbx
	jmp	.L198
.L197:
	movq	8(%r12), %rbx
.L198:
	movzwl	(%rbx), %eax
	cmpw	$30, %ax
	jne	.L199
	movl	8(%rbx), %eax
	movl	%eax, -72(%rbp)
	cmpl	$0, -72(%rbp)
	jle	.L200
	movq	parm_map(%rip), %rdx
	movl	-72(%rbp), %eax
	leal	3(%rax), %ecx
	testl	%eax, %eax
	cmovs	%ecx, %eax
	sarl	$2, %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rbx
	movq	8(%rbx), %rax
	jmp	.L173
.L200:
	movl	fp_delta(%rip), %eax
	addl	-72(%rbp), %eax
	movl	%eax, %edx
	movl	$4, %esi
	movl	$30, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rdx
	movq	frame_pointer_rtx(%rip), %rax
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L199:
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	frame_pointer_rtx(%rip), %rax
	movq	%rbx, %rcx
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rsi
	movl	%r14d, %edi
	movl	$0, %eax
	call	force_reg
	movq	%rax, %r13
	movl	fp_delta(%rip), %eax
	movl	%eax, %esi
	movq	%r13, %rdi
	movl	$0, %eax
	call	plus_constant
	jmp	.L173
.L195:
	movq	frame_pointer_rtx(%rip), %rax
	movq	%r12, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	reg_mentioned_p
	testl	%eax, %eax
	je	.L201
	movq	%r12, %rsi
	movl	%r14d, %edi
	movl	$0, %eax
	call	memory_address_p
	testl	%eax, %eax
	je	.L202
	movq	8(%r12), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L203
	movq	16(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movl	8(%rax), %eax
	movl	%eax, %esi
	movq	%rbx, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L204
.L203:
	movq	16(%r12), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L205
	movq	8(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	16(%r12), %rax
	movl	8(%rax), %eax
	movl	%eax, %esi
	movq	%rbx, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L204
.L205:
	movq	16(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movzbl	2(%r12), %eax
	movzbl	%al, %eax
	movq	%rbx, %rcx
	movl	%eax, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
.L204:
	movq	%r13, %rsi
	movl	%r14d, %edi
	movl	$0, %eax
	call	memory_address
	movq	%rax, %r13
	jmp	.L207
.L202:
	movq	16(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movzbl	2(%r12), %eax
	movzbl	%al, %eax
	movq	%rbx, %rcx
	movl	%eax, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
	jmp	.L207
.L201:
	movq	16(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	8(%r12), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movzbl	2(%r12), %eax
	movzbl	%al, %eax
	movq	%rbx, %rcx
	movl	%eax, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
.L207:
	movq	%r13, %rax
	jmp	.L173
.L181:
	movq	8(%r12), %rbx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rbx
	je	.L208
	movq	arg_pointer_rtx(%rip), %rax
	cmpq	%rax, %rbx
	jne	.L209
.L208:
	movl	fp_delta(%rip), %edx
	movq	frame_pointer_rtx(%rip), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L209:
	movzwl	(%rbx), %eax
	cmpw	$63, %ax
	jne	.L210
	movq	8(%rbx), %rdx
	movq	stack_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L210
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L210:
	movq	%rbx, %rsi
	movl	%r14d, %edi
	movl	$0, %eax
	call	memory_address_p
	testl	%eax, %eax
	jne	.L211
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_address
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L211:
	movzwl	(%rbx), %eax
	cmpw	$44, %ax
	jne	.L212
	movq	8(%rbx), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	je	.L213
	movq	16(%rbx), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L214
.L213:
	movq	8(%rbx), %rdx
	movq	frame_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	je	.L215
	movq	8(%rbx), %rdx
	movq	arg_pointer_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L216
.L215:
	movq	8(%rbx), %rax
	movq	%rax, -56(%rbp)
	movq	16(%rbx), %rbx
	jmp	.L217
.L216:
	movq	16(%rbx), %rax
	movq	%rax, -56(%rbp)
	movq	8(%rbx), %rbx
.L217:
	movzwl	(%rbx), %eax
	cmpw	$30, %ax
	jne	.L218
	movl	8(%rbx), %eax
	movl	%eax, -92(%rbp)
	movq	arg_pointer_rtx(%rip), %rax
	cmpq	%rax, -56(%rbp)
	jne	.L219
	movl	first_parm_offset(%rip), %eax
	cmpl	%eax, -92(%rbp)
	jl	.L219
	movl	-92(%rbp), %eax
	leal	3(%rax), %edx
	testl	%eax, %eax
	cmovs	%edx, %eax
	sarl	$2, %eax
	movl	%eax, -60(%rbp)
	movl	-92(%rbp), %eax
	movl	%eax, %edx
	sarl	$31, %edx
	shrl	$30, %edx
	addl	%edx, %eax
	andl	$3, %eax
	subl	%edx, %eax
	movl	%eax, -64(%rbp)
	jmp	.L220
.L222:
	subl	$1, -60(%rbp)
	movl	first_parm_offset(%rip), %eax
	leal	3(%rax), %edx
	testl	%eax, %eax
	cmovs	%edx, %eax
	sarl	$2, %eax
	cmpl	-60(%rbp), %eax
	jle	.L221
	call	abort
.L221:
	addl	$4, -64(%rbp)
.L220:
	movq	parm_map(%rip), %rax
	movl	-60(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rax
	testq	%rax, %rax
	je	.L222
	movq	parm_map(%rip), %rax
	movl	-60(%rbp), %edx
	movslq	%edx, %rdx
	salq	$3, %rdx
	addq	%rdx, %rax
	movq	(%rax), %rbx
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	cltq
	movl	mode_size(,%rax,4), %eax
	cmpl	$3, %eax
	jg	.L223
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	cltq
	movl	mode_size(,%rax,4), %eax
	subl	$4, %eax
	addl	%eax, -64(%rbp)
.L223:
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	cmpl	%r14d, %eax
	je	.L224
	movzbl	2(%rbx), %eax
	testb	%al, %al
	je	.L224
	movzwl	(%rbx), %eax
	cmpw	$37, %ax
	jne	.L225
	movq	8(%rbx), %rax
	movl	-64(%rbp), %edx
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %rdx
	movl	%r14d, %esi
	movq	%rbx, %rdi
	movl	$0, %eax
	call	change_address
	jmp	.L173
.L225:
	movzwl	(%rbx), %eax
	cmpw	$34, %ax
	jne	.L226
	movl	%r14d, %eax
	cltq
	movl	mode_size(,%rax,4), %eax
	movl	%eax, %edx
	addl	-64(%rbp), %edx
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	cltq
	movl	mode_size(,%rax,4), %eax
	cmpl	%eax, %edx
	je	.L227
	call	abort
.L227:
	movl	$0, %ecx
	movq	%rbx, %rdx
	movl	%r14d, %esi
	movl	$35, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L226:
	call	abort
.L224:
	movq	%rbx, %rax
	jmp	.L173
.L219:
	movl	fp_delta(%rip), %eax
	addl	-92(%rbp), %eax
	movl	%eax, %edx
	movl	$4, %esi
	movl	$30, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %rdx
	movq	frame_pointer_rtx(%rip), %rax
	movq	%rdx, %rcx
	movq	%rax, %rdx
	movl	$4, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
	movq	%r13, %rsi
	movl	$4, %edi
	movl	$0, %eax
	call	memory_address_p
	testl	%eax, %eax
	jne	.L218
	movq	inline_fp_rtx(%rip), %rax
	movl	-92(%rbp), %edx
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %rdx
	movl	%r14d, %esi
	movl	$37, %edi
	movl	$0, %eax
	call	gen_rtx
	jmp	.L173
.L218:
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rbx
	movq	frame_pointer_rtx(%rip), %rax
	movq	%rbx, %rcx
	movq	%rax, %rdx
	movl	$4, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
	movl	fp_delta(%rip), %eax
	movl	%eax, %esi
	movq	%r13, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	movq	%r13, %rsi
	movl	$4, %edi
	movl	$0, %eax
	call	memory_address
	movq	%rax, %r13
	jmp	.L235
.L214:
	movq	frame_pointer_rtx(%rip), %rax
	movq	%rbx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	reg_mentioned_p
	testl	%eax, %eax
	je	.L229
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L230
	movq	16(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %r13
	movq	8(%rbx), %rax
	movl	8(%rax), %eax
	movl	%eax, %esi
	movq	%r13, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L235
.L230:
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L232
	movq	8(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %r13
	movq	16(%rbx), %rax
	movl	8(%rax), %eax
	movl	%eax, %esi
	movq	%r13, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L235
.L232:
	movq	16(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %r13
	movq	8(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdx
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	movq	%r13, %rcx
	movl	%eax, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
	jmp	.L235
.L229:
	movq	16(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L233
	movq	16(%rbx), %rax
	movl	8(%rax), %r13d
	movq	8(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movl	%r13d, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L235
.L233:
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	jne	.L234
	movq	8(%rbx), %rax
	movl	8(%rax), %r13d
	movq	16(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movl	%r13d, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	movq	%rax, %r13
	jmp	.L235
.L234:
	movq	8(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, -80(%rbp)
	movq	16(%rbx), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, -88(%rbp)
	movzbl	2(%rbx), %eax
	movzbl	%al, %eax
	movq	-88(%rbp), %rcx
	movq	-80(%rbp), %rdx
	movl	%eax, %esi
	movl	$44, %edi
	movl	$0, %eax
	call	gen_rtx
	movq	%rax, %r13
	jmp	.L235
.L212:
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %r13
.L235:
	movq	%r13, %rdx
	movl	%r14d, %esi
	movq	%r12, %rdi
	movl	$0, %eax
	call	change_address
	jmp	.L173
.L178:
	call	abort
.L251:
	nop
	jmp	.L174
.L252:
	nop
.L174:
	movl	%r15d, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %rbx
	movl	%r14d, %eax
	movb	%al, 2(%rbx)
	movzbl	3(%r12), %eax
	shrb	$4, %al
	andl	$1, %eax
	andl	$1, %eax
	movl	%eax, %edx
	sall	$4, %edx
	movzbl	3(%rbx), %eax
	andl	$-17, %eax
	orl	%edx, %eax
	movb	%al, 3(%rbx)
	movzbl	3(%r12), %eax
	shrb	$3, %al
	andl	$1, %eax
	andl	$1, %eax
	leal	0(,%rax,8), %edx
	movzbl	3(%rbx), %eax
	andl	$-9, %eax
	orl	%edx, %eax
	movb	%al, 3(%rbx)
	movzbl	3(%r12), %eax
	shrb	$2, %al
	andl	$1, %eax
	andl	$1, %eax
	leal	0(,%rax,4), %edx
	movzbl	3(%rbx), %eax
	andl	$-5, %eax
	orl	%edx, %eax
	movb	%al, 3(%rbx)
	movzwl	(%rbx), %eax
	movzwl	%ax, %eax
	cltq
	movq	rtx_format(,%rax,8), %rax
	movq	%rax, -104(%rbp)
	movl	$0, %r13d
	jmp	.L236
.L249:
	movq	-104(%rbp), %rdx
	movzbl	(%rdx), %eax
	movsbl	%al, %eax
	addq	$1, -104(%rbp)
	cmpl	$101, %eax
	je	.L240
	cmpl	$101, %eax
	jg	.L244
	cmpl	$48, %eax
	je	.L253
	cmpl	$69, %eax
	je	.L239
	jmp	.L237
.L244:
	cmpl	$115, %eax
	je	.L242
	cmpl	$117, %eax
	je	.L243
	cmpl	$105, %eax
	je	.L241
	jmp	.L237
.L240:
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movslq	%r13d, %rdx
	movq	%rax, 8(%rbx,%rdx,8)
	jmp	.L245
.L243:
	movq	insn_map(%rip), %rdx
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%rbx,%rax,8)
	jmp	.L245
.L239:
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%rbx,%rax,8)
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	testq	%rax, %rax
	je	.L254
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	je	.L255
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movl	(%rax), %eax
	movl	%eax, %edi
	movl	$0, %eax
	call	rtvec_alloc
	movslq	%r13d, %rdx
	movq	%rax, 8(%rbx,%rdx,8)
	movl	$0, %r14d
	jmp	.L247
.L248:
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movq	%rax, -112(%rbp)
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movslq	%r14d, %rdx
	movq	8(%rax,%rdx,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movslq	%r14d, %rdx
	movq	-112(%rbp), %rcx
	movq	%rax, 8(%rcx,%rdx,8)
	addl	$1, %r14d
.L247:
	movl	%r14d, %edx
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	cmpl	%eax, %edx
	jb	.L248
	jmp	.L245
.L241:
	movslq	%r13d, %rax
	movl	8(%r12,%rax,8), %edx
	movslq	%r13d, %rax
	movl	%edx, 8(%rbx,%rax,8)
	jmp	.L245
.L242:
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%rbx,%rax,8)
	jmp	.L245
.L237:
	call	abort
.L253:
	nop
	jmp	.L245
.L254:
	nop
	jmp	.L245
.L255:
	nop
.L245:
	addl	$1, %r13d
.L236:
	movzwl	(%rbx), %eax
	movzwl	%ax, %eax
	cltq
	movl	rtx_length(,%rax,4), %eax
	cmpl	%r13d, %eax
	jg	.L249
	cmpl	$22, %r15d
	jne	.L250
	movq	orig_asm_operands_vector(%rip), %rax
	testq	%rax, %rax
	jne	.L250
	movq	32(%r12), %rax
	movq	%rax, orig_asm_operands_vector(%rip)
	movq	32(%rbx), %rax
	movq	%rax, copy_asm_operands_vector(%rip)
.L250:
	movq	%rbx, %rax
.L173:
	addq	$72, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	leave
.LCFI45:
	ret
.LFE6:
	.size	copy_rtx_and_substitute, .-copy_rtx_and_substitute
	.type	copy_address, @function
copy_address:
.LFB7:
	pushq	%rbp
.LCFI46:
	movq	%rsp, %rbp
.LCFI47:
	pushq	%r15
.LCFI48:
	pushq	%r14
.LCFI49:
	pushq	%r13
.LCFI50:
	pushq	%r12
.LCFI51:
	pushq	%rbx
.LCFI52:
	subq	$24, %rsp
.LCFI53:
	movq	%rdi, %rbx
	testq	%rbx, %rbx
	jne	.L257
	movl	$0, %eax
	jmp	.L258
.L257:
	movzwl	(%rbx), %eax
	movzwl	%ax, %r12d
	movzbl	2(%rbx), %eax
	movzbl	%al, %r13d
	leal	-17(%r12), %eax
	cmpl	$27, %eax
	ja	.L259
	mov	%eax, %eax
	movq	.L265(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L265:
	.quad	.L260
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L261
	.quad	.L261
	.quad	.L259
	.quad	.L261
	.quad	.L262
	.quad	.L259
	.quad	.L259
	.quad	.L263
	.quad	.L260
	.quad	.L261
	.quad	.L261
	.quad	.L259
	.quad	.L259
	.quad	.L259
	.quad	.L264
	.text
.L262:
	movl	8(%rbx), %eax
	cmpl	$14, %eax
	je	.L266
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L258
.L266:
	movl	fp_delta(%rip), %edx
	movq	frame_pointer_rtx(%rip), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	plus_constant
	jmp	.L258
.L264:
	movq	8(%rbx), %rax
	movzwl	(%rax), %eax
	cmpw	$34, %ax
	jne	.L282
	movq	8(%rbx), %rax
	movl	8(%rax), %eax
	cmpl	$14, %eax
	jne	.L283
	movl	fp_delta(%rip), %eax
	movl	%eax, %esi
	movq	%rbx, %rdi
	movl	$0, %eax
	call	plus_constant
	jmp	.L258
.L263:
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_to_reg
	jmp	.L258
.L260:
	movq	%rbx, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L258
.L261:
	movq	%rbx, %rax
	jmp	.L258
.L282:
	nop
	jmp	.L259
.L283:
	nop
.L259:
	movl	%r12d, %edi
	movl	$0, %eax
	call	rtx_alloc
	movq	%rax, %r12
	movl	%r13d, %eax
	movb	%al, 2(%r12)
	movzbl	3(%rbx), %eax
	shrb	$4, %al
	andl	$1, %eax
	andl	$1, %eax
	movl	%eax, %edx
	sall	$4, %edx
	movzbl	3(%r12), %eax
	andl	$-17, %eax
	orl	%edx, %eax
	movb	%al, 3(%r12)
	movzbl	3(%rbx), %eax
	shrb	$3, %al
	andl	$1, %eax
	andl	$1, %eax
	leal	0(,%rax,8), %edx
	movzbl	3(%r12), %eax
	andl	$-9, %eax
	orl	%edx, %eax
	movb	%al, 3(%r12)
	movzbl	3(%rbx), %eax
	shrb	$2, %al
	andl	$1, %eax
	andl	$1, %eax
	leal	0(,%rax,4), %edx
	movzbl	3(%r12), %eax
	andl	$-5, %eax
	orl	%edx, %eax
	movb	%al, 3(%r12)
	movzwl	(%r12), %eax
	movzwl	%ax, %eax
	cltq
	movq	rtx_format(,%rax,8), %r15
	movl	$0, %r13d
	jmp	.L268
.L281:
	movzbl	(%r15), %eax
	movsbl	%al, %eax
	addq	$1, %r15
	cmpl	$101, %eax
	je	.L272
	cmpl	$101, %eax
	jg	.L276
	cmpl	$48, %eax
	je	.L284
	cmpl	$69, %eax
	je	.L271
	jmp	.L269
.L276:
	cmpl	$115, %eax
	je	.L274
	cmpl	$117, %eax
	je	.L275
	cmpl	$105, %eax
	je	.L273
	jmp	.L269
.L272:
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movslq	%r13d, %rdx
	movq	%rax, 8(%r12,%rdx,8)
	jmp	.L277
.L275:
	movq	insn_map(%rip), %rdx
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	8(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%r12,%rax,8)
	jmp	.L277
.L271:
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%r12,%rax,8)
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	testq	%rax, %rax
	je	.L285
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	je	.L286
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movl	(%rax), %eax
	movl	%eax, %edi
	movl	$0, %eax
	call	rtvec_alloc
	movslq	%r13d, %rdx
	movq	%rax, 8(%r12,%rdx,8)
	movl	$0, %r14d
	jmp	.L279
.L280:
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movq	%rax, -56(%rbp)
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rax
	movslq	%r14d, %rdx
	movq	8(%rax,%rdx,8), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movslq	%r14d, %rdx
	movq	-56(%rbp), %rcx
	movq	%rax, 8(%rcx,%rdx,8)
	addl	$1, %r14d
.L279:
	movl	%r14d, %edx
	movslq	%r13d, %rax
	movq	8(%r12,%rax,8), %rax
	movl	(%rax), %eax
	cmpl	%eax, %edx
	jb	.L280
	jmp	.L277
.L273:
	movslq	%r13d, %rax
	movl	8(%rbx,%rax,8), %edx
	movslq	%r13d, %rax
	movl	%edx, 8(%r12,%rax,8)
	jmp	.L277
.L274:
	movslq	%r13d, %rax
	movq	8(%rbx,%rax,8), %rdx
	movslq	%r13d, %rax
	movq	%rdx, 8(%r12,%rax,8)
	jmp	.L277
.L269:
	call	abort
.L284:
	nop
	jmp	.L277
.L285:
	nop
	jmp	.L277
.L286:
	nop
.L277:
	addl	$1, %r13d
.L268:
	movzwl	(%r12), %eax
	movzwl	%ax, %eax
	cltq
	movl	rtx_length(,%rax,4), %eax
	cmpl	%r13d, %eax
	jg	.L281
	movq	%r12, %rax
.L258:
	addq	$24, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	leave
.LCFI54:
	ret
.LFE7:
	.size	copy_address, .-copy_address
	.type	try_fold_cc0, @function
try_fold_cc0:
.LFB8:
	pushq	%rbp
.LCFI55:
	movq	%rsp, %rbp
.LCFI56:
	pushq	%rbx
.LCFI57:
	subq	$72, %rsp
.LCFI58:
	movq	%rdi, -72(%rbp)
	movq	-72(%rbp), %rax
	movq	32(%rax), %rax
	movq	16(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	movq	%rax, -40(%rbp)
	movq	-40(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L288
	movq	-40(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L288
	movq	-40(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L288
	movq	-40(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L289
.L288:
	movq	-72(%rbp), %rax
	movq	24(%rax), %rax
	testq	%rax, %rax
	je	.L289
	movq	-72(%rbp), %rax
	movq	24(%rax), %rax
	movq	32(%rax), %rax
	movq	%rax, -48(%rbp)
	movq	-48(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$25, %ax
	jne	.L289
	movq	-48(%rbp), %rax
	movq	8(%rax), %rdx
	movq	pc_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L289
	movq	-48(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -48(%rbp)
	movq	-48(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$42, %ax
	jne	.L289
	movq	-48(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	cltq
	movl	rtx_length(,%rax,4), %eax
	cmpl	$2, %eax
	jne	.L289
	movq	-48(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -56(%rbp)
	movq	-56(%rbp), %rax
	movq	8(%rax), %rdx
	movq	cc0_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L290
	movq	-56(%rbp), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L291
	movq	-56(%rbp), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L291
	movq	-56(%rbp), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L291
	movq	-56(%rbp), %rax
	movq	16(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L290
.L291:
	movq	-56(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -24(%rbp)
	cmpq	$0, -24(%rbp)
	jne	.L292
.L290:
	movq	-56(%rbp), %rax
	movq	16(%rax), %rdx
	movq	cc0_rtx(%rip), %rax
	cmpq	%rax, %rdx
	jne	.L289
	movq	-56(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	je	.L293
	movq	-56(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$39, %ax
	je	.L293
	movq	-56(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	je	.L293
	movq	-56(%rbp), %rax
	movq	8(%rax), %rax
	movzwl	(%rax), %eax
	cmpw	$32, %ax
	jne	.L289
.L293:
	movq	-56(%rbp), %rax
	movq	8(%rax), %rax
	movq	%rax, -24(%rbp)
	cmpq	$0, -24(%rbp)
	je	.L289
.L292:
	movq	-48(%rbp), %rax
	movq	24(%rax), %rdx
	movq	-48(%rbp), %rax
	movq	16(%rax), %rbx
	movq	-24(%rbp), %rsi
	movq	-40(%rbp), %rcx
	movq	-56(%rbp), %rax
	movq	%rsi, %r8
	movq	%rbx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	fold_out_const_cc0
	movq	%rax, -64(%rbp)
	cmpq	$0, -64(%rbp)
	je	.L289
	movq	-64(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$38, %ax
	jne	.L294
	movq	-72(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -32(%rbp)
	jmp	.L295
.L297:
	movq	-32(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -32(%rbp)
.L295:
	cmpq	$0, -32(%rbp)
	je	.L296
	movq	-32(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$17, %ax
	jne	.L297
.L296:
	cmpq	$0, -32(%rbp)
	jne	.L298
	call	abort
.L298:
	movq	label_map(%rip), %rdx
	movq	-32(%rbp), %rax
	movl	32(%rax), %eax
	cltq
	salq	$3, %rax
	leaq	(%rdx,%rax), %rax
	movq	(%rax), %rdx
	movq	-64(%rbp), %rax
	movq	8(%rax), %rax
	cmpq	%rax, %rdx
	jne	.L299
	movq	-32(%rbp), %rax
	movq	16(%rax), %rax
	jmp	.L300
.L299:
	movq	-64(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	emit_jump
	movq	-72(%rbp), %rax
	movq	24(%rax), %rax
	jmp	.L300
.L294:
	movq	pc_rtx(%rip), %rax
	cmpq	%rax, -64(%rbp)
	jne	.L301
	movq	-72(%rbp), %rax
	movq	24(%rax), %rax
	jmp	.L300
.L301:
	call	abort
.L289:
	movl	$0, %eax
.L300:
	addq	$72, %rsp
	popq	%rbx
	leave
.LCFI59:
	ret
.LFE8:
	.size	try_fold_cc0, .-try_fold_cc0
	.type	fold_out_const_cc0, @function
fold_out_const_cc0:
.LFB9:
	pushq	%rbp
.LCFI60:
	movq	%rsp, %rbp
.LCFI61:
	subq	$64, %rsp
.LCFI62:
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	%rdx, -40(%rbp)
	movq	%rcx, -48(%rbp)
	movq	%r8, -56(%rbp)
	movq	-48(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	sete	%al
	movzbl	%al, %eax
	movl	%eax, -12(%rbp)
	movq	-56(%rbp), %rax
	movzwl	(%rax), %eax
	cmpw	$30, %ax
	sete	%al
	movzbl	%al, %eax
	movl	%eax, -16(%rbp)
	cmpl	$0, -12(%rbp)
	je	.L303
	movq	-48(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -4(%rbp)
	jmp	.L304
.L303:
	movl	$1, -4(%rbp)
.L304:
	cmpl	$0, -16(%rbp)
	je	.L305
	movq	-56(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -8(%rbp)
	jmp	.L306
.L305:
	movl	$1, -8(%rbp)
.L306:
	movq	-24(%rbp), %rax
	movzwl	(%rax), %eax
	movzwl	%ax, %eax
	subl	$67, %eax
	cmpl	$9, %eax
	ja	.L307
	mov	%eax, %eax
	movq	.L318(,%rax,8), %rax
	jmp	*%rax
	.section	.rodata
	.align 8
	.align 4
.L318:
	.quad	.L308
	.quad	.L309
	.quad	.L310
	.quad	.L311
	.quad	.L312
	.quad	.L313
	.quad	.L314
	.quad	.L315
	.quad	.L316
	.quad	.L317
	.text
.L308:
	cmpl	$0, -12(%rbp)
	je	.L319
	cmpl	$0, -16(%rbp)
	je	.L319
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	je	.L320
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L320:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L319:
	cmpl	$0, -4(%rbp)
	je	.L322
	cmpl	$0, -8(%rbp)
	jne	.L323
.L322:
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L323:
	cmpl	$0, -12(%rbp)
	jne	.L362
	cmpl	$0, -16(%rbp)
	jne	.L363
	movq	-56(%rbp), %rdx
	movq	-48(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	rtx_equal_p
	testl	%eax, %eax
	je	.L364
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L309:
	cmpl	$0, -12(%rbp)
	je	.L325
	cmpl	$0, -16(%rbp)
	je	.L325
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jne	.L326
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L326:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L325:
	cmpl	$0, -4(%rbp)
	je	.L327
	cmpl	$0, -8(%rbp)
	jne	.L328
.L327:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L328:
	cmpl	$0, -12(%rbp)
	jne	.L365
	cmpl	$0, -16(%rbp)
	jne	.L366
	movq	-56(%rbp), %rdx
	movq	-48(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	rtx_equal_p
	testl	%eax, %eax
	je	.L367
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L310:
	cmpl	$0, -12(%rbp)
	je	.L330
	cmpl	$0, -16(%rbp)
	je	.L330
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jl	.L331
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L331:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L330:
	cmpl	$0, -4(%rbp)
	jne	.L332
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L332:
	cmpl	$0, -8(%rbp)
	jne	.L368
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L311:
	cmpl	$0, -12(%rbp)
	je	.L334
	cmpl	$0, -16(%rbp)
	je	.L334
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jle	.L335
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L335:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L334:
	cmpl	$0, -4(%rbp)
	jne	.L336
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L336:
	cmpl	$0, -8(%rbp)
	jne	.L369
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L312:
	cmpl	$0, -12(%rbp)
	je	.L338
	cmpl	$0, -16(%rbp)
	je	.L338
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jg	.L339
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L339:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L338:
	cmpl	$0, -4(%rbp)
	jne	.L340
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L340:
	cmpl	$0, -8(%rbp)
	jne	.L370
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L313:
	cmpl	$0, -12(%rbp)
	je	.L342
	cmpl	$0, -16(%rbp)
	je	.L342
	movl	-4(%rbp), %eax
	cmpl	-8(%rbp), %eax
	jge	.L343
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L343:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L342:
	cmpl	$0, -4(%rbp)
	jne	.L344
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L344:
	cmpl	$0, -8(%rbp)
	jne	.L371
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L314:
	cmpl	$0, -12(%rbp)
	je	.L346
	cmpl	$0, -16(%rbp)
	je	.L346
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	cmpl	%eax, %edx
	jb	.L347
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L347:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L346:
	cmpl	$0, -4(%rbp)
	jne	.L348
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L348:
	cmpl	$0, -8(%rbp)
	jne	.L372
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L315:
	cmpl	$0, -12(%rbp)
	je	.L350
	cmpl	$0, -16(%rbp)
	je	.L350
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	cmpl	%eax, %edx
	jbe	.L351
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L351:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L350:
	cmpl	$0, -4(%rbp)
	jne	.L352
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L352:
	cmpl	$0, -8(%rbp)
	jne	.L373
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L316:
	cmpl	$0, -12(%rbp)
	je	.L354
	cmpl	$0, -16(%rbp)
	je	.L354
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	cmpl	%eax, %edx
	ja	.L355
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L355:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L354:
	cmpl	$0, -4(%rbp)
	jne	.L356
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L356:
	cmpl	$0, -8(%rbp)
	jne	.L374
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L317:
	cmpl	$0, -12(%rbp)
	je	.L358
	cmpl	$0, -16(%rbp)
	je	.L358
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %eax
	cmpl	%eax, %edx
	jae	.L359
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L359:
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L358:
	cmpl	$0, -4(%rbp)
	jne	.L360
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L360:
	cmpl	$0, -8(%rbp)
	jne	.L375
	movq	-40(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	copy_rtx_and_substitute
	jmp	.L321
.L362:
	nop
	jmp	.L307
.L363:
	nop
	jmp	.L307
.L364:
	nop
	jmp	.L307
.L365:
	nop
	jmp	.L307
.L366:
	nop
	jmp	.L307
.L367:
	nop
	jmp	.L307
.L368:
	nop
	jmp	.L307
.L369:
	nop
	jmp	.L307
.L370:
	nop
	jmp	.L307
.L371:
	nop
	jmp	.L307
.L372:
	nop
	jmp	.L307
.L373:
	nop
	jmp	.L307
.L374:
	nop
	jmp	.L307
.L375:
	nop
.L307:
	movl	$0, %eax
.L321:
	leave
.LCFI63:
	ret
.LFE9:
	.size	fold_out_const_cc0, .-fold_out_const_cc0
.globl output_inline_function
	.type	output_inline_function, @function
output_inline_function:
.LFB10:
	pushq	%rbp
.LCFI64:
	movq	%rsp, %rbp
.LCFI65:
	subq	$32, %rsp
.LCFI66:
	movq	%rdi, -24(%rbp)
	movq	-24(%rbp), %rax
	movq	136(%rax), %rax
	movq	%rax, -16(%rbp)
	movl	$0, %eax
	call	temporary_allocation
	movq	-24(%rbp), %rax
	movq	%rax, current_function_decl(%rip)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	expand_function_start
	movq	-24(%rbp), %rax
	movl	128(%rax), %eax
	movl	%eax, %esi
	movl	$26, %edi
	movl	$0, %eax
	call	assign_stack_local
	movq	-16(%rbp), %rax
	movq	32(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	restore_reg_data
	movq	-24(%rbp), %rax
	movl	40(%rax), %edx
	movq	-24(%rbp), %rax
	movq	32(%rax), %rax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	expand_function_end
	movq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	jmp	.L377
.L378:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	movq	%rax, -8(%rbp)
.L377:
	movq	-8(%rbp), %rax
	movq	24(%rax), %rax
	testq	%rax, %rax
	jne	.L378
	movq	-16(%rbp), %rax
	movq	32(%rax), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	set_new_first_and_last_insn
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	rest_of_compilation
	movq	$0, current_function_decl(%rip)
	movl	$0, %eax
	call	permanent_allocation
	leave
.LCFI67:
	ret
.LFE10:
	.size	output_inline_function, .-output_inline_function
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	"zR"
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.uleb128 0x1
	.byte	0x3
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB0
	.long	.LFE0-.LFB0
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI0-.LFB0
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI4-.LCFI1
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI5-.LCFI4
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.long	.LFB1
	.long	.LFE1-.LFB1
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI6-.LFB1
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI7-.LCFI6
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI9-.LCFI7
	.byte	0x83
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI10-.LCFI9
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE3:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.long	.LFB2
	.long	.LFE2-.LFB2
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI11-.LFB2
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI12-.LCFI11
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI18-.LCFI12
	.byte	0x83
	.uleb128 0x7
	.byte	0x8c
	.uleb128 0x6
	.byte	0x8d
	.uleb128 0x5
	.byte	0x8e
	.uleb128 0x4
	.byte	0x8f
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI19-.LCFI18
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE5:
.LSFDE7:
	.long	.LEFDE7-.LASFDE7
.LASFDE7:
	.long	.LASFDE7-.Lframe1
	.long	.LFB3
	.long	.LFE3-.LFB3
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI20-.LFB3
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI21-.LCFI20
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI24-.LCFI21
	.byte	0x83
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI25-.LCFI24
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE7:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.long	.LFB4
	.long	.LFE4-.LFB4
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI26-.LFB4
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI27-.LCFI26
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI31-.LCFI27
	.byte	0x83
	.uleb128 0x5
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8d
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI32-.LCFI31
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE9:
.LSFDE11:
	.long	.LEFDE11-.LASFDE11
.LASFDE11:
	.long	.LASFDE11-.Lframe1
	.long	.LFB5
	.long	.LFE5-.LFB5
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI33-.LFB5
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI34-.LCFI33
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI36-.LCFI34
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE11:
.LSFDE13:
	.long	.LEFDE13-.LASFDE13
.LASFDE13:
	.long	.LASFDE13-.Lframe1
	.long	.LFB6
	.long	.LFE6-.LFB6
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI37-.LFB6
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI38-.LCFI37
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI44-.LCFI38
	.byte	0x83
	.uleb128 0x7
	.byte	0x8c
	.uleb128 0x6
	.byte	0x8d
	.uleb128 0x5
	.byte	0x8e
	.uleb128 0x4
	.byte	0x8f
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI45-.LCFI44
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE13:
.LSFDE15:
	.long	.LEFDE15-.LASFDE15
.LASFDE15:
	.long	.LASFDE15-.Lframe1
	.long	.LFB7
	.long	.LFE7-.LFB7
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI46-.LFB7
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI47-.LCFI46
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI53-.LCFI47
	.byte	0x83
	.uleb128 0x7
	.byte	0x8c
	.uleb128 0x6
	.byte	0x8d
	.uleb128 0x5
	.byte	0x8e
	.uleb128 0x4
	.byte	0x8f
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI54-.LCFI53
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE15:
.LSFDE17:
	.long	.LEFDE17-.LASFDE17
.LASFDE17:
	.long	.LASFDE17-.Lframe1
	.long	.LFB8
	.long	.LFE8-.LFB8
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI55-.LFB8
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI56-.LCFI55
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI58-.LCFI56
	.byte	0x83
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI59-.LCFI58
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE17:
.LSFDE19:
	.long	.LEFDE19-.LASFDE19
.LASFDE19:
	.long	.LASFDE19-.Lframe1
	.long	.LFB9
	.long	.LFE9-.LFB9
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI60-.LFB9
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI61-.LCFI60
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI63-.LCFI61
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE19:
.LSFDE21:
	.long	.LEFDE21-.LASFDE21
.LASFDE21:
	.long	.LASFDE21-.Lframe1
	.long	.LFB10
	.long	.LFE10-.LFB10
	.uleb128 0x0
	.byte	0x4
	.long	.LCFI64-.LFB10
	.byte	0xe
	.uleb128 0x10
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI65-.LCFI64
	.byte	0xd
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI67-.LCFI65
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.align 8
.LEFDE21:
	.ident	"GCC: (GNU) 4.5.2"
	.section	.note.GNU-stack,"",@progbits
