// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042b9e9 | Name: FUN_0042b9e9


undefined4 __fastcall FUN_0042b9e9(int param_1)

{
  LONG LVar1;
  MMRESULT MVar2;
  
  if (*(int *)(param_1 + 0x80) == 0) {
    if (*(int *)(param_1 + 4) == 0) {
      return 0x800401f0;
    }
    if (*(int *)(param_1 + 0x7c) == 1) {
      LVar1 = mmioSeek(*(HMMIO *)(param_1 + 4),*(int *)(param_1 + 0x28) + 4,0);
      if (LVar1 == -1) {
        return 0x80004005;
      }
      *(undefined4 *)(param_1 + 8) = 0x61746164;
      MVar2 = mmioDescend(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),
                          (MMCKINFO *)(param_1 + 0x1c),0x10);
      if (MVar2 != 0) {
        return 0x80004005;
      }
    }
    else {
      *(undefined4 *)(param_1 + 8) = 0x61746164;
      *(undefined4 *)(param_1 + 0xc) = 0;
      MVar2 = mmioCreateChunk(*(HMMIO *)(param_1 + 4),(LPMMCKINFO)(param_1 + 8),0);
      if (MVar2 != 0) {
        return 0x80004005;
      }
      MVar2 = mmioGetInfo(*(HMMIO *)(param_1 + 4),(LPMMIOINFO)(param_1 + 0x34),0);
      if (MVar2 != 0) {
        return 0x80004005;
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x88) = *(undefined4 *)(param_1 + 0x84);
  }
  return 0;
}

