// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d04b3 | Name: FUN_004d04b3


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * FUN_004d04b3(undefined4 *param_1,undefined2 param_2,float param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  PAGE_INFO *pPVar4;
  undefined4 *puVar5;
  float fVar6;
  float fVar7;
  undefined1 local_74 [24];
  PAGE_INFO local_5c [48];
  float local_2c;
  undefined4 local_28;
  undefined2 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined1 local_10 [12];
  
  CPreviewView::PAGE_INFO::PAGE_INFO(local_5c);
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  FUN_004310a0(&stack0x0000001c);
  FUN_004310a0(&stack0x00000010);
  iVar2 = _rand();
  iVar3 = _rand();
  fVar7 = ((float)iVar2 - (float)iVar3) / _DAT_005a4960;
  iVar2 = _rand();
  iVar3 = _rand();
  fVar6 = ((float)iVar2 - (float)iVar3) / _DAT_005a4960;
  iVar2 = _rand();
  iVar3 = _rand();
  FUN_00430f50(((float)iVar2 - (float)iVar3) / _DAT_005a4960,fVar6,fVar7);
  thunk_FUN_005611dd(&local_1c,&local_1c);
  FUN_00430f50(local_1c,local_18,local_14);
  FUN_004d1960(0x41700000);
  uVar1 = FUN_00493d50(local_74,local_10);
  FUN_004310a0(uVar1);
  iVar2 = _rand();
  iVar3 = _rand();
  local_2c = ((((float)iVar2 - (float)iVar3) / _DAT_005a4960) / _DAT_005a3834 + _DAT_005a3660) *
             param_3;
  local_28 = 0;
  local_20 = 0x3dcccccd;
  local_24 = param_2;
  pPVar4 = local_5c;
  puVar5 = param_1;
  for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar5 = *(undefined4 *)pPVar4;
    pPVar4 = pPVar4 + 4;
    puVar5 = puVar5 + 1;
  }
  return param_1;
}

