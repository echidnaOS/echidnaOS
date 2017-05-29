; The BPB starts here

bpbOEM						db 'ECHIDNA '
bpbBytesPerSector			dw 512
bpbSectorsPerCluster		db 4
bpbReservedSectors			dw 8
bpbNumberOfFATs				db 2
bpbRootEntries				dw 512
bpbTotalSectors				dw 0
bpbMedia					db 0xF8
bpbSectorsPerFAT			dw 128
bpbSectorsPerTrack			dw 32
bpbHeadsPerCylinder			dw 64
bpbHiddenSectors			dd 0
bpbTotalSectorsBig			dd 131040
bsDriveNumber				db 0x80
bsUnused					db 0x00
bsExtBootSignature			db 0x29
bsSerialNumber				dd 0x12345678
bsVolumeLabel				db 'ECHIDNA OS '
bsFileSystem				db 'FAT16   '
