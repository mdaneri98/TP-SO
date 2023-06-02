#!/bin/bash
if [[ "$1" = "gdb" ]]; then
    qemu-system-x86_64 -s -S -cpu IvyBridge -hda Image/x64BareBonesImage.qcow2 -m 512 -rtc base=localtime -d int
else
    qemu-system-x86_64 -cpu IvyBridge -hda Image/x64BareBonesImage.qcow2 -m 512 -rtc base=localtime
fi
