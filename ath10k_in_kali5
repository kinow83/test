
apt install linux-source
apt source linux-source

cd /usr/src/<kernel-source>
cp /lib/modules/5.18.0-kali5-amd64/build/Module.symvers .
cp -v cp /boot/config-5.18.0-kali5-amd64 .config
make menuconfig
scripts/config --disable SYSTEM_TRUSTED_KEYS
make modules -j 4
make modules -j 4 M=drivers/net/wireless/ath/ath10k


https://starrykss.tistory.com/1722


[Error#1]
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
https://hiaurea.tistory.com/32
해결 : dnf --enablerepo=powertools install dwarves
      apt install dwarves
```
