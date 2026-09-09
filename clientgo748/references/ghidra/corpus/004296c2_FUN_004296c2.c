// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004296c2 | Name: FUN_004296c2


void __thiscall FUN_004296c2(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  undefined4 local_10;
  undefined4 local_8;
  
  if (0 < param_2) {
    param_2 = 0;
  }
  if (param_2 < -10000) {
    param_2 = -10000;
  }
  *(int *)(param_1 + 4) = param_2;
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 8 + local_8 * 0x108) != 0) {
      iVar1 = FUN_0042c140();
      for (local_10 = 0; local_10 < iVar1; local_10 = local_10 + 1) {
        piVar2 = (int *)FUN_0042ac98(local_10);
        (**(code **)(*piVar2 + 0x3c))(piVar2,*(undefined4 *)(param_1 + 4));
      }
    }
  }
  return;
}

