

## build kernel
ref https://starrykss.tistory.com/1722
```
1. current kernel version
# uname -r
5.18.0-kali5-amd64

2. recently kernel source download
# apt search linux-source
Sorting... Done
Full Text Search... Done
linux-source/kali-rolling,now 5.18.5-1kali6 all [installed]
  Linux kernel source (meta-package)

linux-source-5.18/kali-rolling,now 5.18.5-1kali6 all [installed,automatic]
  Linux kernel source for version 5.18 with Debian patches


# apt install linux-source
# apt source linux-source
# apt install libssl-dev
# apt install dwarves
# cd /usr/src/linux-5.18.5
# make oldconfig && make prepare
# make kernelversion  <- check build kernel version
# make modules -j 4
# make bzimage
# make modules_install
# make install
# reboot
```

## build ath10k
```
# make modules -j 4 M=drivers/net/wireless/ath/ath10k
# modprobe -r ath10k_pci
# cp -a drivers/net/wireless/ath/ath10k/*.ko /lib/modules/5.18.5/kernel/drivers/net/wireless/ath/ath10k/
# modprobe ath10k_pci
# tail -f  /var/log/syslog <- printk debug
```

## Debugging
ref: https://wireless.wiki.kernel.org/en/users/drivers/ath10k/debug
```
make menuconfig
Device Drivers -->
	Network device support -->
		Wireless LAN -->
			Atheros ath10k XXXXXX
```

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

## ath10k driver source analysis
```
1. [pic.c] static struct pci_driver ath10k_pci_driver = {
  .probe = ath10k_pci_probe,
}

2. [pic.c] static int ath10k_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *pci_dev) {
  ret = ath10k_pci_init_irq(ar);
}

3. [pic.c] static int ath10k_pci_init_irq(struct ath10k *ar) {
  ath10k_pci_init_napi(ar);
}

4. [pic.c] void ath10k_pci_init_napi(struct ath10k *ar) {
	netif_napi_add(&ar->napi_dev, &ar->napi, ath10k_pci_napi_poll, ATH10K_NAPI_BUDGET);
}

5. static int ath10k_pci_napi_poll(struct napi_struct *ctx, int budget) {
  done = ath10k_htt_txrx_compl_task(ar, budget);
}
```
