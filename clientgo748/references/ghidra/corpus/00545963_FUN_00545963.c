// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00545963 | Name: FUN_00545963


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00545963(int param_1)

{
  float fVar1;
  short sVar2;
  short sVar3;
  void *pvVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 local_48;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a29de;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(int *)(param_1 + 0x160) = *(int *)(param_1 + 0x40) / 2;
  *(int *)(param_1 + 0x164) = *(int *)(param_1 + 0x3c) / 2;
  *(undefined4 *)(param_1 + 0x40) = 0;
  if (*(int *)(param_1 + 0x17c) == 0) {
    pvVar4 = operator_new(0x360);
    local_8 = 0;
    if (pvVar4 == (void *)0x0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_004baf10();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x17c) = local_48;
    *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x10) = 0x242;
    *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0xc) = 0x20;
    *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x298) = 3;
    *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x14) = 1;
    *(int *)(*(int *)(param_1 + 0x17c) + 0x28) =
         (*(int *)(param_1 + 0x160) + 1) * (*(int *)(param_1 + 0x164) + 1);
    *(int *)(*(int *)(param_1 + 0x17c) + 0x20) =
         *(int *)(param_1 + 0x160) * 2 * *(int *)(param_1 + 0x164);
    iVar5 = (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x5c))
                      (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),
                       *(int *)(*(int *)(param_1 + 0x17c) + 0x28) *
                       *(int *)(*(int *)(param_1 + 0x17c) + 0xc),0,
                       *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x10),1,
                       *(int *)(param_1 + 0x17c) + 0x318);
    if (iVar5 < 0) {
      if (*(int *)(param_1 + 0x17c) != 0) {
        if (*(undefined4 **)(param_1 + 0x17c) != (undefined4 *)0x0) {
          (**(code **)**(undefined4 **)(param_1 + 0x17c))(1);
        }
        *(undefined4 *)(param_1 + 0x17c) = 0;
      }
      FUN_00431159(s_Can_t_Create_Vertex_Buffer_in_Se_005bdfe4);
      uVar6 = 0;
    }
    else {
      local_14 = 0;
      (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x2c))
                (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318),0,0,&local_14,0);
      for (local_1c = 0; local_1c < *(int *)(param_1 + 0x164) + 1; local_1c = local_1c + 1) {
        for (local_24 = 0; local_24 < *(int *)(param_1 + 0x160) + 1; local_24 = local_24 + 1) {
          fVar1 = (float)(local_24 - *(int *)(param_1 + 0x160) / 2);
          *(float *)(local_14 + (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               fVar1 + fVar1;
          fVar1 = (float)(local_1c - *(int *)(param_1 + 0x164) / 2);
          *(float *)(local_14 + 8 + (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20)
               = fVar1 + fVar1;
          *(float *)(local_14 + 0x10 +
                    (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               (float)local_24 / _DAT_005a365c;
          *(float *)(local_14 + 0x14 +
                    (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               (float)local_1c / _DAT_005a365c;
          *(float *)(local_14 + 0x18 +
                    (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               (float)local_24 / _DAT_005a3e9c;
          *(float *)(local_14 + 0x1c +
                    (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) =
               (float)local_1c / _DAT_005a3e9c;
          *(undefined4 *)
           (local_14 + 4 + (local_24 + local_1c * (*(int *)(param_1 + 0x160) + 1)) * 0x20) = 0;
        }
      }
      (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 0x30))
                (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
      iVar5 = (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x60))
                        (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),
                         *(int *)(*(int *)(param_1 + 0x17c) + 0x20) * 6,0,0x65,1,
                         *(int *)(param_1 + 0x17c) + 0x31c);
      if (iVar5 < 0) {
        if (*(int *)(*(int *)(param_1 + 0x17c) + 0x318) != 0) {
          (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x318) + 8))
                    (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318));
          *(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x318) = 0;
        }
        if (*(int *)(param_1 + 0x17c) != 0) {
          if (*(undefined4 **)(param_1 + 0x17c) != (undefined4 *)0x0) {
            (**(code **)**(undefined4 **)(param_1 + 0x17c))(1);
          }
          *(undefined4 *)(param_1 + 0x17c) = 0;
        }
        FUN_00431159(s_Can_t_Create_Index_Buffer_in_Sea_005be00c);
        uVar6 = 0;
      }
      else {
        (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x31c) + 0x2c))
                  (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x31c),0,0,&local_20,0);
        local_18 = 0;
        for (local_1c = 0; local_1c < *(int *)(param_1 + 0x164); local_1c = local_1c + 1) {
          for (local_28 = 0; local_28 < *(int *)(param_1 + 0x160); local_28 = local_28 + 1) {
            sVar3 = (short)local_1c;
            sVar2 = (short)local_28;
            *(short *)(local_20 + local_18 * 2) =
                 sVar2 + sVar3 * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            *(short *)(local_20 + (local_18 + 1) * 2) =
                 sVar2 + (sVar3 + 1) * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            *(short *)(local_20 + (local_18 + 2) * 2) =
                 sVar2 + 1 + sVar3 * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            *(short *)(local_20 + (local_18 + 3) * 2) =
                 sVar2 + (sVar3 + 1) * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            *(short *)(local_20 + (local_18 + 4) * 2) =
                 sVar2 + 1 + (sVar3 + 1) * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            *(short *)(local_20 + (local_18 + 5) * 2) =
                 sVar2 + 1 + sVar3 * ((short)*(undefined4 *)(param_1 + 0x160) + 1);
            local_18 = local_18 + 6;
          }
        }
        (**(code **)(**(int **)(*(int *)(param_1 + 0x17c) + 0x31c) + 0x30))
                  (*(undefined4 *)(*(int *)(param_1 + 0x17c) + 0x31c));
        uVar6 = 1;
      }
    }
  }
  else {
    uVar6 = 0;
  }
  ExceptionList = local_10;
  return uVar6;
}

