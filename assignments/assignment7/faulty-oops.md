
# Faulty Kernel Module Error

## How did the error occur
Run echo “hello_world” > /dev/faulty from the command line of your running qemu image.

## Kernel Output
Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000 <br />
Mem abort info: <br />
  ESR = 0x96000045  <br /> 
  EC = 0x25: DABT (current EL), IL = 32 bits <br />
  SET = 0, FnV = 0  <br />
  EA = 0, S1PTW = 0  <br />
  FSC = 0x05: level 1 translation fault  <br />
Data abort info:   <br />
  ISV = 0, ISS = 0x00000045   <br />
  CM = 0, WnR = 1  <br />
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042059000   <br />
[0000000000000000] pgd=0000000000000000, p4d=0000000000000000, pud=0000000000000000  <br />
Internal error: Oops: 96000045 [#1] SMP  <br />
Modules linked in: hello(O) scull(O) faulty(O)  <br />
CPU: 0 PID: 158 Comm: sh Tainted: G           O      5.15.18 #1 <br />
Hardware name: linux,dummy-virt (DT)  <br />
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--) <br />
pc : faulty_write+0x14/0x20 [faulty]  <br />
lr : vfs_write+0xa8/0x2b0 <br />  
sp : ffffffc008d23d80  <br />
x29: ffffffc008d23d80 x28: ffffff80020d0cc0 x27: 0000000000000000  <br />
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000 <br />
x23: 0000000040001000 x22: 0000000000000012 x21: 000000555c3c2a70 <br />
x20: 000000555c3c2a70 x19: ffffff8002006f00 x18: 0000000000000000  <br />
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000  <br />
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000  <br />
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000  <br />
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000  <br />
x5 : 0000000000000001 x4 : ffffffc0006f0000 x3 : ffffffc008d23df0  <br />
x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000  <br />
Call trace:  <br />
 faulty_write+0x14/0x20 [faulty]  <br />
 ksys_write+0x68/0x100  <br />
 __arm64_sys_write+0x20/0x30  <br />
 invoke_syscall+0x54/0x130  <br />
 el0_svc_common.constprop.0+0x44/0xf0  <br />
 do_el0_svc+0x40/0xa0  <br />
 el0_svc+0x20/0x60  <br />
 el0t_64_sync_handler+0xe8/0xf0   <br />
 el0t_64_sync+0x1a0/0x1a4  <br />
Code: d2800001 d2800000 d503233f d50323bf (b900003f)  <br />
---[ end trace d82bccb7eb0a8383 ]---  <br />





## Analysis: 
An oops message is usually the outcome of deferencing a NULL pointer. All addresses used by the processor are virtual addresses and
mapped to the physical addresses through a complex structure of page table. The paging mechanism fails to point to do this mapping when
we dereference an invalid pointer (NULL in this case). The error has occured in the faulty_write function which can be found in misc-modules/faulty.c

ssize_t faulty_write (struct file *filp, const char __user *buf, size_t count, loff_t *pos)  
{  
	/* make a simple fault by dereferencing a NULL pointer */  
	*(int *)0 = 0;  
	return 0;  
}  
The oops message displays the processor status at the time of the fault including contents of CPU registers. After displaying the message,
the process is then killed.

The first line Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000 tells us that we were trying to
deference a NULL pointer which is not permitted.

Internal error: Oops: 96000045 [#1] SMP
This is the error code value and each bit has its own significance. [#1] means that the oops error occured once.

CPU: 0 indicates the CPU on which the error occured.
Tainted: G indicates that a proprietary module was loaded.

pc : faulty_write+0x14/0x20 [faulty]  <br />
lr : vfs_write+0xa8/0x2b0 <br />  
sp : ffffffc008d23d80  <br />
The above lines show us the values of the CPU registers including program counter, link register and stack pointer. The values of other general
purpose registers can be seen below that.

pc : faulty_write+0x10/0x20 [faulty]
From the program counter we can see that the error occured in the faulty_write function inside the faulty.c file. 0x10 is the offset from
faulty_write and 0x20 is the length.

The call trace shows the list of functions being called just before the oops occured.

Code: d2800001 d2800000 d503233f d50323bf (b900003f) is the hexdump of the section of machine code that was being run at the time the oops occured.
