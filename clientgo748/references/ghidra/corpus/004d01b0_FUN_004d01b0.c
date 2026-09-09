// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d01b0 | Name: FUN_004d01b0


undefined4 * __thiscall
FUN_004d01b0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_64 [76];
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a092c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0054a9e0(0);
  local_8 = 0;
  FUN_00421910(param_1 + 0xb,0x18,4,FUN_004cb8e0);
  FUN_00421910(param_1 + 0x23,0x40,100,CPreviewView::PAGE_INFO::PAGE_INFO);
  FUN_00421910(param_1 + 0x665,0xc,100,CSimpleArray<>);
  *param_1 = &PTR_FUN_005a4928;
  local_18 = (uint)local_18._2_2_ << 0x10;
  while ((local_18 & 0xffff) < 100) {
    puVar1 = (undefined4 *)FUN_00430f50(0,0x40a00000,0);
    uVar2 = FUN_004d04b3(local_64,(local_18 & 0xffff) % 3,0x41200000,param_2,param_3,param_4,*puVar1
                         ,puVar1[1],puVar1[2]);
    FUN_004d18b0(uVar2);
    local_18 = CONCAT22(local_18._2_2_,(short)local_18 + 1);
  }
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  param_1[0x663] = uVar2;
  param_1[0x664] = 0;
  param_1[9] = param_5;
  uVar2 = FUN_00430f50(0xbf000000,0xbf000000,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0x3f000000,0xbf000000,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0x3f000000,0x3f000000,0);
  FUN_004310a0(uVar2);
  uVar2 = FUN_00430f50(0xbf000000,0x3f000000,0);
  FUN_004310a0(uVar2);
  param_1[0xf] = 0x3ca3d70a;
  param_1[0x10] = 0x3f7ae148;
  param_1[0x15] = 0x3f7ae148;
  param_1[0x16] = 0x3f7ae148;
  param_1[0x1b] = 0x3f7ae148;
  param_1[0x1c] = 0x3ca3d70a;
  param_1[0x21] = 0x3ca3d70a;
  param_1[0x22] = 0x3ca3d70a;
  local_14 = 0x13b;
  if (DAT_005ccf98 != 0) {
    iVar3 = FUN_00429a6d(0x13b);
    if (iVar3 != 0) {
      uVar4 = 0;
      uVar2 = 0;
      FUN_00429a6d(local_14);
      FUN_0042ad2b(uVar2,uVar4);
    }
  }
  _memset(param_1 + 0x792,0,100);
  ExceptionList = local_10;
  return param_1;
}

