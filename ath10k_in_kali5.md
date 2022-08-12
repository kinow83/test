
apt install linux-source
apt source linux-source

https://starrykss.tistory.com/1722


## Errors
Error#1
```

  GEN     .version
  CHK     include/generated/compile.h
  LD      vmlinux.o
ld: warning: arch/x86/power/hibernate_asm_64.o: missing .note.GNU-stack section implies executable stack
ld: NOTE: This behaviour is deprecated and will be removed in a future version of the linker
  MODPOST vmlinux.symvers
  MODINFO modules.builtin.modinfo
  GEN     modules.builtin
BTF: .tmp_vmlinux.btf: pahole (pahole) is not available
Failed to generate BTF for vmlinux
Try to disable CONFIG_DEBUG_INFO_BTF
make: *** [Makefile:1169: vmlinux] Error 1
```
Resolve#1
```
https://hiaurea.tistory.com/32
해결 : dnf --enablerepo=powertools install dwarves
      apt install dwarves
```
