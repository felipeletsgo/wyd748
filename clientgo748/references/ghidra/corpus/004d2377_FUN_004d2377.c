// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d2377 | Name: FUN_004d2377


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d2377(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
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
  
  iVar1 = FUN_00494dcf();
  if (iVar1 == 30000) {
    if (*(int *)(param_1 + 0x24) == 0) {
      *(undefined4 *)(param_1 + 0x28) = 0;
      uVar2 = 0;
    }
    else {
      local_18 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x84),0,180000);
      if (local_18 == 0) {
        *(undefined4 *)(param_1 + 0x28) = 0;
        uVar2 = 0;
      }
      else {
        local_14 = *(int *)(DAT_013b71e8 + 0x1b084);
        FUN_00430f10();
        FUN_00430f10();
        puVar3 = (undefined4 *)
                 FUN_00430f50(*(undefined4 *)(local_14 + 0x48),*(undefined4 *)(local_14 + 0x50),
                              *(undefined4 *)(local_14 + 0x4c));
        local_24 = *puVar3;
        local_20 = puVar3[1];
        local_1c = puVar3[2];
        puVar3 = &local_24;
        puVar5 = local_48;
        FUN_00430f50(*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x7c),
                     *(undefined4 *)(param_1 + 0x78));
        puVar3 = (undefined4 *)FUN_004d3bc0(puVar5,puVar3);
        local_10 = *puVar3;
        local_c = puVar3[1];
        local_8 = puVar3[2];
        fVar4 = (float10)FUN_004d3c90(&local_10);
        if ((float10)*(float *)(local_18 + 0x340) < fVar4) {
          uVar2 = FUN_00430f50(*(undefined4 *)(local_14 + 0x58),*(undefined4 *)(local_14 + 0x60),
                               *(undefined4 *)(local_14 + 0x5c));
          fVar4 = (float10)FUN_004d3cf0(&local_10,uVar2);
          if (fVar4 <= (float10)_DAT_005a3534) {
            *(undefined4 *)(param_1 + 0x28) = 0;
            uVar2 = 0;
          }
          else {
            uVar2 = FUN_004d2509();
            *(undefined4 *)(param_1 + 0x28) = uVar2;
            uVar2 = *(undefined4 *)(param_1 + 0x28);
          }
        }
        else {
          *(undefined4 *)(param_1 + 0x28) = 1;
          uVar2 = 1;
        }
      }
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x28) = 1;
    uVar2 = 1;
  }
  return uVar2;
}

