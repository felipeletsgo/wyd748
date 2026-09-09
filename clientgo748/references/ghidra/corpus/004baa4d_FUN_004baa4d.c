// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004baa4d | Name: FUN_004baa4d


undefined4 __thiscall FUN_004baa4d(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 < 0) || (0x40 < param_2)) {
    uVar1 = 0;
  }
  else if (param_3 == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 4 + param_2 * 0xc) == 0) {
    iVar2 = FUN_00560eab(*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),param_3,0x278,0x10,0xc,0,0,
                         *(undefined4 *)(DAT_005ccf9c + 0x2a514),1,1,1,0,0,0,
                         param_1 + 4 + param_2 * 0xc);
    if (iVar2 < 0) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

