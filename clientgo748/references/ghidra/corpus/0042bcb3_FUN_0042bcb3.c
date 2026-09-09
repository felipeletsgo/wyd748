// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042bcb3 | Name: FUN_0042bcb3


undefined4 __fastcall FUN_0042bcb3(int param_1)

{
  MMRESULT MVar1;
  char local_8 [4];
  
  if (*(int *)(param_1 + 0x7c) == 1) {
    mmioClose(*(HMMIO *)(param_1 + 4),0);
    *(undefined4 *)(param_1 + 4) = 0;
  }
  else {
    *(uint *)(param_1 + 0x34) = *(uint *)(param_1 + 0x34) | 0x10000000;
    if (*(int *)(param_1 + 4) == 0) {
      return 0x800401f0;
    }
    MVar1 = mmioSetInfo(*(HMMIO *)(param_1 + 4),(LPCMMIOINFO)(param_1 + 0x34),0);
    if (MVar1 != 0) {
      return 0x80004005;
    }
    MVar1 = mmioAscend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),0);
    if (MVar1 != 0) {
      return 0x80004005;
    }
    MVar1 = mmioAscend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 0x1c),0);
    if (MVar1 != 0) {
      return 0x80004005;
    }
    mmioSeek(*(HMMIO *)(param_1 + 4),0,0);
    MVar1 = mmioDescend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 0x1c),(MMCKINFO *)0x0,0);
    if (MVar1 != 0) {
      return 0x80004005;
    }
    *(undefined4 *)(param_1 + 8) = 0x74636166;
    MVar1 = mmioDescend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),
                        (MMCKINFO *)(param_1 + 0x1c),0x10);
    if (MVar1 == 0) {
      local_8[0] = '\0';
      local_8[1] = '\0';
      local_8[2] = '\0';
      local_8[3] = '\0';
      mmioWrite(*(HMMIO *)(param_1 + 4),local_8,4);
      mmioAscend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),0);
    }
    MVar1 = mmioAscend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 0x1c),0);
    if (MVar1 != 0) {
      return 0x80004005;
    }
    mmioClose(*(HMMIO *)(param_1 + 4),0);
    *(undefined4 *)(param_1 + 4) = 0;
  }
  return 0;
}

