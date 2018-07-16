;bomb/bomb:	file format ELF64-x86-64

phase_6:
  4010f4:	pushq	%r14
  4010f6:	pushq	%r13
  4010f8:	pushq	%r12
  4010fa:	pushq	%rbp
  4010fb:	pushq	%rbx
  4010fc:	subq	$80, %rsp
  401100:	movq	%rsp, %r13
  401103:	movq	%rsp, %rsi
  401106:	callq	<read_six_numbers>
  40110b:	movq	%rsp, %r14
  40110e:	movl	$0, %r12d
  401114:	movq	%r13, %rbp
  401117:	movl	(%r13), %eax
  40111b:	subl	$1, %eax
  40111e:	cmpl	$5, %eax
  401121:	jbe	    <phase_6+0x34>
  401123:	callq	786 <explode_bomb>
  401128:	addl	$1, %r12d
  40112c:	cmpl	$6, %r12d
  401130:	je	    <phase_6+0x5f>
  401132:	movl	%r12d, %ebx
  401135:	movslq	%ebx, %rax
  401138:	movl	(%rsp,%rax,4), %eax
  40113b:	cmpl	%eax, (%rbp)
  40113e:	jne	    <phase_6+0x51>
  401140:	callq	<explode_bomb>     ;no repeat with first element
  401145:	addl	$1, %ebx
  401148:	cmpl	$5, %ebx
  40114b:	jle	    <phase_6+0x41>
  40114d:	addq	$4, %r13
  401151:	jmp	<phase_6+0x20>
  401153:	leaq	24(%rsp), %rsi
  401158:	movq	%r14, %rax
  40115b:	movl	$7, %ecx
  401160:	movl	%ecx, %edx
  401162:	subl	(%rax), %edx        ;7 minus inputs
  401164:	movl	%edx, (%rax)
  401166:	addq	$4, %rax
  40116a:	cmpq	%rsi, %rax
  40116d:	jne	    <phase_6+0x6c>
  40116f:	movl	$0, %esi            ;array has been reverted
  401174:	jmp	    <phase_6+0xa3>    ;jump to 401197

  401176:	movq	8(%rdx), %rdx
  40117a:	addl	$1, %eax
  40117d:	cmpl	%ecx, %eax
  40117f:	jne	    <phase_6+0x82>    ;not equal, jump back to the loop 401176 
  401181:	jmp	    <phase_6+0x94>    ;finished, jump to 401188

  401183:	movl	$6304464, %edx      ;store the array

  401188:	movq	%rdx, 32(%rsp,%rsi,2)
  40118d:	addq	$4, %rsi
  401191:	cmpq	$24, %rsi
  401195:	je	    <phase_6+0xb7>    ;jump to 4011ab

  ;kind of an entry
  401197:	movl	(%rsp,%rsi), %ecx
  40119a:	cmpl	$1, %ecx
  40119d:	jle	    <phase_6+0x8f>    ;<= jump to 401183
  40119f:	movl	$1, %eax
  4011a4:	movl	$6304464, %edx      ;store the array
  4011a9:	jmp	    <phase_6+0x82>    ;> jump to 401176

  4011ab:	movq	32(%rsp), %rbx      ;1st element
  4011b0:	leaq	40(%rsp), %rax      ;2nd element's address
  4011b5:	leaq	80(%rsp), %rsi      ;bottom
  4011ba:	movq	%rbx, %rcx

  4011bd:	movq	(%rax), %rdx
  4011c0:	movq	%rdx, 8(%rcx)       ;node5->next = node6 (build a linked list)
  4011c4:	addq	$8, %rax            ;get next node's address
  4011c8:	cmpq	%rsi, %rax
  4011cb:	je	  <phase_6+0xde>      ;bottom, jump to 4011d2
  4011cd:	movq	%rdx, %rcx
  4011d0:	jmp	  <phase_6+0xc9>      ;or jump back to the loop 4011bd

  ;rdx refers to node1, rbx refers to node6

  ;|address           | value | node
  ;|0x6032d0 <node1>: |	332	  |  1
  ;|0x6032e0 <node2>:	| 168	  |  2
  ;|0x6032f0 <node3>:	| 924	  |  3
  ;|0x603300 <node4>:	| 691	  |  4
  ;|0x603310 <node5>:	| 477	  |  5
  ;|0x603320 <node6>:	| 443	  |  6
  ;
  ; !!! arranged by value:
  ;924->691->447->443->332->168
  ;
  ; 3->4->5->6->1->2
  ; 7 minus:
  ; 4->3->2->1->6->5

  4011d2:	movq	$0, 8(%rdx)
  4011da:	movl	$5, %ebp
  4011df:	movq	8(%rbx), %rax
  4011e3:	movl	(%rax), %eax
  4011e5:	cmpl	%eax, (%rbx)
  4011e7:	jge	  <phase_6+0xfa>       ;jump to 4011ee
  4011e9:	callq	<explode_bomb>
  4011ee:	movq	8(%rbx), %rbx
  4011f2:	subl	$1, %ebp
  4011f5:	jne	  <phase_6+0xeb>       ;back to 4011df 
  4011f7:	addq	$80, %rsp
  4011fb:	popq	%rbx
  4011fc:	popq	%rbp
  4011fd:	popq	%r12
  4011ff:	popq	%r13
  401201:	popq	%r14
  401203:	retq