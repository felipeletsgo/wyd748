// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042bfc1 | Name: FUN_0042bfc1


undefined4 __thiscall FUN_0042bfc1(int param_1,uint param_2,int param_3,int *param_4)

{
  undefined4 uVar1;
  MMRESULT MVar2;
  uint local_8;
  
  if (*(int *)(param_1 + 0x80) == 0) {
    if (*(int *)(param_1 + 4) == 0) {
      uVar1 = 0x800401f0;
    }
    else if ((param_4 == (int *)0x0) || (param_3 == 0)) {
      uVar1 = 0x80070057;
    }
    else {
      *param_4 = 0;
      for (local_8 = 0; local_8 < param_2; local_8 = local_8 + 1) {
        if (*(int *)(param_1 + 0x50) == *(int *)(param_1 + 0x58)) {
          *(uint *)(param_1 + 0x34) = *(uint *)(param_1 + 0x34) | 0x10000000;
          MVar2 = mmioAdvance(*(HMMIO *)(param_1 + 4),(LPMMIOINFO)(param_1 + 0x34),1);
          if (MVar2 != 0) {
            return 0x80004005;
          }
        }
        **(undefined1 **)(param_1 + 0x50) = *(undefined1 *)(param_3 + local_8);
        *(int *)(param_1 + 0x50) = *(int *)(param_1 + 0x50) + 1;
        *param_4 = *param_4 + 1;
      }
      uVar1 = 0;
    }
  }
  else {
    uVar1 = 0x80004001;
  }
  return uVar1;
}

