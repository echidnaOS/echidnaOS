qemu-system-x86_64 -net none -enable-kvm -hda echidna.img -s -S &> /dev/null < /dev/null &
gdb --eval-command="target remote localhost:1234"
