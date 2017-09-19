/*
 * syscall.c
 *
 *  Created on: 2016. 10. 5.
 *      Author: root
 */



#include <linux/kernel.h> /* We're doing kernel work */
#include <linux/module.h> /* Specifically, a module, */
#include <linux/moduleparam.h> /* which will have params */
#include <linux/unistd.h> /* The list of system calls */
#include <linux/sched.h>
#include <linux/uaccess.h>

#define USER_PAGE_PROTECTED

MODULE_LICENSE("GPL");

/*************************************************************************
 * cat /boot/System.map-$(uname -r) |grep -e "R sys_call_table"
 *************************************************************************/

static void **syscall_table = (void*) 0xffffffff8164e400;
static int uid;
module_param(uid, int, 0644);


asmlinkage int (*original_open) (const char*, int, int);
asmlinkage int (*original_write)(unsigned int, const char __user *, size_t);

/*************************************************************************************
 ****************************** System.map ********************************
 *************************************************************************************/
/*
System.map 파일
	리눅스 커널에 들어 있는 심벌에 대한 정보를 담고 있음.
	이 파일은 커널 부팅과정에서 사용되지 않고, 부팅 이후 디버깅을 하는 프로그램 등에 의해 사용됨.
	아래 글은 Wikipedia 참조 : http://en.wikipedia.org/wiki/System.map
	In Linux, the System.map file is a symbol table used by the kernel.
	리눅스에서 System.map 파일은 커널에서 사용되는 심볼 테이블이다.
	A symbol table is a look-up between symbol names and their addresses in memory.
	심볼 테이블은 메모리에서 심볼 이름과 주소 사이를 검색한다.
	A symbol name may be the name of a variable or the name of a function.
	심볼 이름은 변수의 이름이거나 함수의 이름이다.
	The System.map is required when the address of a symbol name, or the symbol name of an address, is needed.
	System.map은 심볼 이름의 주소 또는 주소의 심볼 이름이 필요할 때 참고할 수 있다.
	It is especially useful for debugging kernel panics and kernel oopses.
	이는 커널 패닉과 커널 에러 메시지를 디버깅하는데 유용하다.
	Because addresses may change from one build to the next, a new System.map is generated for each build of the kernel.
	주소는 한 빌드에서 다음까지 변화될 수 있기 때문에, System.map은 커널의 각 빌드에 대해 생성된다.

Type 설명(Wikipedia 참조 : http://en.wikipedia.org/wiki/System.map)
	The character between the address and the symbol (separated by spaces) is the 'type' of the symbol.
	주소와 심볼 사이에 있는 문자는 심볼에 대한 Type이다.
	The nm utility program on Unix systems list the symbols from object files.
	Unix 시스템의 nm 유틸리티 프로그램은 오브젝트 파일의 심볼을 나열한다.
	A : absolute.
	B or b : uninitialized data section (called BSS). - 초기화되지 않은 섹션(BSS)
	D or d : initialized data section. - 초기화 된 섹션
	G or g : initialized data section for small objects (global). - small objects를 위한 초기화 된 된 데이터 섹션
	i : sections specific to DLLs. - DLLs에 대한 특정 섹션
	N : debugging symbol. - 디버깅 심볼
	p : stack unwind section. - 스택 해제 섹션
	R or r : read only data section.
	S or s : uninitialzed data section for small objects. - small objects를 위한 초기화 되지 않은 섹션
	T or t : text (code) section. - 코드 섹션
	U : undefined.
	V or v : weak object.
	W or w : weak objects which have not been tagged so.
	- : stabs symbol in an a.out object file.
	? : 'symbol type unknown.'
 */

static void disable_page_protection(void)
{
	unsigned long value;
	asm volatile ("mov %%cr0, %0" : "=r" (value));
	if (value & 0x00010000) {
		value &= ~0x00010000;
		asm volatile ("mov %0, %%cr0" : : "r" (value));
	}
}

static void enable_page_protection(void)
{
	unsigned long value;
	asm volatile ("mov %%cr0, %0" : "=r" (value));
	if (!(value & 0x00010000)) {
		value |= 0x00010000;
		asm volatile ("mov %0, %%cr0" : : "r" (value));
	}
}

asmlinkage int my_sys_open(const char *filename, int flags, int mode)
{
	int i=0;
	char ch;
	kuid_t kuid;

	kuid = current_uid();

	printk(KERN_INFO "current = 0x%p\n", current);
	printk(KERN_INFO "kuid = %d\n", kuid.val);
	printk(KERN_INFO "current->comm = %s\n", current->comm);

	if (uid == kuid.val) {
		printk("Opened file by %d: ", uid);
		do {
			get_user(ch, filename+i);
			if (ch == 0) {
				break;
			}
			i++;
			printk("%c", ch);
		} while (1);
		printk("\n");
	}


	return original_open(filename, flags, mode);

}

static int __init syscall_init(void)
{

	printk(KERN_ALERT "I'm dangerous. I hope you did a ");
	printk(KERN_ALERT "sync before you insmod'ed me.\n");
	printk(KERN_ALERT "My counterpart, cleanup_module(), is even");
	printk(KERN_ALERT "more dangerous. If\n");
	printk(KERN_ALERT "you value your file system, it will ");
	printk(KERN_ALERT "be \"sync; rmmod\" \n");
	printk(KERN_ALERT "when you remove this module.\n");

#ifdef USER_PAGE_PROTECTED
	enable_page_protection();
#else
	write_cr0 (read_cr0 () & (~ 0x10000));
#endif
	{
		printk(KERN_INFO "__NR_open                ==> %d\n", __NR_open);
		printk(KERN_INFO "syscall_table[__NR_open] ==> %p\n", syscall_table[__NR_open]);
		printk(KERN_INFO "my_sys_open              ==> %p\n", my_sys_open);

		original_open = syscall_table[__NR_open];
		syscall_table[__NR_open] = my_sys_open;
		printk(KERN_INFO "Spying on UID:%d\n", uid);
	}
#ifdef USER_PAGE_PROTECTED
	disable_page_protection();
#else
	write_cr0 (read_cr0 () | 0x10000);
#endif

	return 0;
}

static void __exit syscall_exit(void)
{
#ifdef USER_PAGE_PROTECTED
	enable_page_protection();
#else
	write_cr0 (read_cr0 () & (~ 0x10000));
#endif
	if (syscall_table[__NR_open] != my_sys_open) {
		printk(KERN_ALERT "Somebody else also played with the ");
		printk(KERN_ALERT "open system call\n");
		printk(KERN_ALERT "The system may be left in ");
		printk(KERN_ALERT "an unstable state.\n");
	}

	syscall_table[__NR_open] = original_open;
#ifdef USER_PAGE_PROTECTED
	disable_page_protection();
#else
	write_cr0 (read_cr0 () | 0x10000);
#endif
}

module_init(syscall_init);
module_exit(syscall_exit);
