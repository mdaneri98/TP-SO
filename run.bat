@echo off
qemu-system-x86_64.exe -hda Image/x64BareBonesImage.qcow2 -m 512 -rtc base=localtime -soundhw pcspk
