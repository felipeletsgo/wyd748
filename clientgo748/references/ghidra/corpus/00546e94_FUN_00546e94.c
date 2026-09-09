// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00546e94 | Name: FUN_00546e94


undefined4 * __thiscall FUN_00546e94(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined4 uVar5;
  undefined1 local_8c [12];
  undefined1 local_80 [12];
  undefined1 local_74 [12];
  undefined1 local_68 [12];
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  undefined1 local_2c [12];
  int local_20;
  undefined1 local_1c [12];
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_00430f50(0,0xc61c4000,0);
  if (*(int *)(param_1 + 0x17c) != 0) {
    FUN_00430f10();
    FUN_00430f10();
    (**(code **)(*DAT_005ccf9c + 0x50))(local_1c,local_2c);
    local_20 = 0;
    (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x2c))
              (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318),0,0,&local_20,0);
    for (local_30 = 0; local_30 < *(int *)(param_1 + 0x164); local_30 = local_30 + 1) {
      for (local_34 = 0; local_34 < *(int *)(param_1 + 0x160); local_34 = local_34 + 1) {
        local_38 = local_20 + (local_34 + local_30 * (*(int *)(param_1 + 0x160) + 1)) * 0x20;
        local_3c = local_20 + (local_34 + 1 + local_30 * (*(int *)(param_1 + 0x160) + 1)) * 0x20;
        local_40 = local_20 + (local_34 + (local_30 + 1) * (*(int *)(param_1 + 0x160) + 1)) * 0x20;
        local_44 = local_20 +
                   (local_34 + 1 + (local_30 + 1) * (*(int *)(param_1 + 0x160) + 1)) * 0x20;
        iVar1 = FUN_005649c7(local_38,local_40,local_3c,local_1c,local_2c,0,0,0);
        if (iVar1 == 1) {
          uVar5 = 0x40400000;
          puVar4 = local_68;
          puVar3 = local_5c;
          iVar1 = local_40;
          FUN_004b3a50(local_50,local_3c);
          FUN_004b3a50(puVar3,iVar1);
          puVar2 = (undefined4 *)FUN_00547240(puVar4,uVar5);
          local_10 = *puVar2;
          local_c = puVar2[1];
          local_8 = puVar2[2];
          *(int *)(param_1 + 0x1c4) = local_34;
          *(int *)(param_1 + 0x1c8) = local_30;
          (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x30))
                    (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
          *param_2 = local_10;
          param_2[1] = local_c;
          param_2[2] = local_8;
          return param_2;
        }
        iVar1 = FUN_005649c7(local_40,local_44,local_3c,local_1c,local_2c,0,0,0);
        if (iVar1 == 1) {
          uVar5 = 0x40400000;
          puVar3 = local_8c;
          puVar4 = local_80;
          iVar1 = local_44;
          FUN_004b3a50(local_74,local_3c);
          FUN_004b3a50(puVar4,iVar1);
          puVar2 = (undefined4 *)FUN_00547240(puVar3,uVar5);
          local_10 = *puVar2;
          local_c = puVar2[1];
          local_8 = puVar2[2];
          *(int *)(param_1 + 0x1c4) = local_34;
          *(int *)(param_1 + 0x1c8) = local_30;
          (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x30))
                    (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
          *param_2 = local_10;
          param_2[1] = local_c;
          param_2[2] = local_8;
          return param_2;
        }
      }
    }
    (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x30))
              (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
  }
  *param_2 = local_10;
  param_2[1] = local_c;
  param_2[2] = local_8;
  return param_2;
}

