// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488879 | Name: FUN_00488879


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00488879(int param_1,int param_2)

{
  int *piVar1;
  void *pvVar2;
  int iVar3;
  int iVar4;
  int local_4c;
  undefined4 local_28;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059fb9b;
  local_10 = ExceptionList;
  iVar4 = *(int *)(param_1 + 0x2872c);
  ExceptionList = &local_10;
  *(undefined1 *)(param_2 + 0x25) = 0;
  local_1c = 0;
  do {
    if (*(int *)(iVar4 + 0x210) <= local_1c) {
LAB_00488911:
      local_28 = 0xffffffff;
      if (*(char *)(param_2 + 0xd) == '\0') {
        local_28 = 0xffaaaaff;
      }
      pvVar2 = operator_new(0xe6c);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_4c = 0;
      }
      else {
        local_4c = FUN_004077ce(param_2 + 0x16,local_28,0,0,0x42e40000,0x41a00000,
                                *(undefined2 *)(param_2 + 0x14),(int)*(char *)(param_2 + 0xc),
                                (int)*(short *)(param_2 + 0xe),(int)*(short *)(param_2 + 0x12),
                                (int)*(short *)(param_2 + 0x10));
      }
      local_8 = 0xffffffff;
      if (*(char *)(param_2 + 0xd) == '\0') {
        *(undefined4 *)(local_4c + 0xe5c) = 2;
      }
      if (local_4c != 0) {
        FUN_00408d33(local_4c);
      }
      iVar3 = (**(code **)(*DAT_013b71e8 + 0x34))(*(undefined2 *)(param_2 + 0x14));
      if (iVar3 != 0) {
        *(undefined1 *)(iVar3 + 0x248) = 1;
        FUN_004f9946(0xaaffff00);
      }
      if (DAT_005b892c == 2) {
        iVar4 = FUN_0040c0f0();
        if ((iVar4 == 0) && (*(ushort *)(param_2 + 0x14) < 1000)) {
          FUN_0044da6f();
        }
      }
      else {
        iVar4 = *(int *)(iVar4 + 0x210);
        piVar1 = *(int **)(param_1 + 0x27d8c);
        (**(code **)(*piVar1 + 0x74))(0x42e40000,(float)iVar4 * _DAT_005a4374 + _DAT_005a4370);
        (**(code **)(*piVar1 + 0x70))(0,_DAT_005a436c - (float)iVar4 * _DAT_005a4374);
        FUN_0040981b(0x42e40000,(float)iVar4 * _DAT_005a4374);
        iVar4 = FUN_0040c0f0();
        if ((iVar4 == 0) && (*(ushort *)(param_2 + 0x14) < 1000)) {
          FUN_0044da6f();
        }
      }
      ExceptionList = local_10;
      return 1;
    }
    iVar3 = *(int *)(iVar4 + 0x214 + local_1c * 4);
    if (*(uint *)(iVar3 + 0xe50) == (uint)*(ushort *)(param_2 + 0x14)) {
      FUN_00409078(iVar3);
      goto LAB_00488911;
    }
    local_1c = local_1c + 1;
  } while( true );
}

