// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00429a6d | Name: FUN_00429a6d


undefined4 __thiscall FUN_00429a6d(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int *piVar3;
  int local_c;
  
  if (param_2 < 1) {
    uVar1 = 0;
  }
  else if (param_2 < 0x200) {
    if (*(int *)(param_1 + 4) == -10000) {
      uVar1 = 0;
    }
    else {
      if (*(int *)(param_1 + 8 + param_2 * 0x108) == 0) {
        iVar2 = FUN_00429e77(param_1 + 8 + param_2 * 0x108,param_1 + 0xc + param_2 * 0x108,0x80,
                             DAT_005a6128,DAT_005a612c,DAT_005a6130,DAT_005a6134,
                             *(undefined4 *)(param_1 + 0x10c + param_2 * 0x108));
        if (iVar2 < 0) {
          FUN_00431159(s_Load_Sound_Error__d____s_005b48c0,param_2,param_1 + 0xc + param_2 * 0x108);
          return 0;
        }
        iVar2 = FUN_0042c140();
        for (local_c = 0; local_c < iVar2; local_c = local_c + 1) {
          piVar3 = (int *)FUN_0042ac98(local_c);
          (**(code **)(*piVar3 + 0x3c))(piVar3,*(undefined4 *)(param_1 + 4));
        }
      }
      uVar1 = *(undefined4 *)(param_1 + 8 + param_2 * 0x108);
    }
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

