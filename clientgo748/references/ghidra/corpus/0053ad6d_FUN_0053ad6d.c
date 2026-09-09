// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053ad6d | Name: FUN_0053ad6d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_0053ad6d(int *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 *puVar5;
  undefined1 local_48 [36];
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_18 = FUN_004b14fd(param_1[9],0,180000);
  if (local_18 == 0) {
    param_1[0x12] = 0;
    iVar1 = 0;
  }
  else {
    local_14 = *(int *)(DAT_013b71e8 + 0x1b084);
    FUN_00430f10();
    FUN_00430f10();
    puVar2 = (undefined4 *)
             FUN_00430f50(*(undefined4 *)(local_14 + 0x48),*(undefined4 *)(local_14 + 0x50),
                          *(undefined4 *)(local_14 + 0x4c));
    local_24 = *puVar2;
    local_20 = puVar2[1];
    local_1c = puVar2[2];
    puVar2 = &local_24;
    puVar5 = local_48;
    FUN_00430f50(param_1[10],param_1[0xb],param_1[0xc]);
    puVar2 = (undefined4 *)FUN_004d3bc0(puVar5,puVar2);
    local_10 = *puVar2;
    local_c = puVar2[1];
    local_8 = puVar2[2];
    fVar4 = (float10)FUN_004d3c90(&local_10);
    if ((float10)*(float *)(local_18 + 0x340) < fVar4) {
      uVar3 = FUN_00430f50(*(undefined4 *)(local_14 + 0x58),*(undefined4 *)(local_14 + 0x60),
                           *(undefined4 *)(local_14 + 0x5c));
      fVar4 = (float10)FUN_004d3cf0(&local_10,uVar3);
      if (fVar4 <= (float10)_DAT_005a3534) {
        param_1[0x12] = 0;
        iVar1 = 0;
      }
      else {
        iVar1 = (**(code **)(*param_1 + 0x60))();
        param_1[0x12] = iVar1;
        iVar1 = param_1[0x12];
      }
    }
    else {
      param_1[0x12] = 1;
      iVar1 = 1;
    }
  }
  return iVar1;
}

