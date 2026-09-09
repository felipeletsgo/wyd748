// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00528325 | Name: FUN_00528325


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00528325(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  *(undefined1 *)(param_1 + 0x774) = 1;
  uVar2 = __ftol();
  *(undefined4 *)(param_1 + 0x474) = uVar2;
  uVar2 = __ftol();
  *(undefined4 *)(param_1 + 0x478) = uVar2;
  *(uint *)(param_1 + 0x47c) = (uint)*(ushort *)(param_2 + 0x18);
  *(uint *)(param_1 + 0x480) = (uint)*(ushort *)(param_2 + 0x1a);
  for (local_8 = 0; iVar1 = DAT_0067cf38, local_8 < 0x30; local_8 = local_8 + 1) {
    *(float *)(param_1 + 0x290 + local_8 * 8) = (float)*(ushort *)(param_2 + 0x18) + _DAT_005a34a0;
    *(float *)(param_1 + 0x294 + local_8 * 8) = (float)*(ushort *)(param_2 + 0x1a) + _DAT_005a34a0;
  }
  if (param_1 == *(int *)(DAT_0067cf38 + 0x4c)) {
    if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
      *(uint *)(DAT_0067cf38 + 0x275b0) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar1 + 0x275b4) = (uint)*(ushort *)(param_2 + 0x1a);
      *(uint *)(iVar1 + 0x28794) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(iVar1 + 0x28798) = (uint)*(ushort *)(param_2 + 0x1a);
    }
    *(uint *)(param_1 + 0x484) = (uint)*(ushort *)(param_2 + 0x18);
    *(uint *)(param_1 + 0x488) = (uint)*(ushort *)(param_2 + 0x1a);
  }
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x5f4) = uVar2;
  return 1;
}

