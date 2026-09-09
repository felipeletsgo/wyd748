// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00585510 | Name: FUN_00585510


undefined4 FUN_00585510(undefined4 *param_1,uint param_2,int param_3,int param_4)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  byte *pbVar4;
  
  iVar3 = param_1[1];
  pbVar4 = (byte *)*param_1;
  for (; param_3 < 0x19; param_3 = param_3 + 8) {
    if (param_1[2] == 0) {
      if (iVar3 == 0) {
        cVar1 = (**(code **)(*(int *)(param_1[7] + 0x14) + 0xc))(param_1[7]);
        if (cVar1 == '\0') {
          return 0;
        }
        pbVar4 = (byte *)**(undefined4 **)(param_1[7] + 0x14);
        iVar3 = (*(undefined4 **)(param_1[7] + 0x14))[1];
      }
      uVar2 = (uint)*pbVar4;
      iVar3 = iVar3 + -1;
      pbVar4 = pbVar4 + 1;
      if (uVar2 == 0xff) {
        do {
          if (iVar3 == 0) {
            cVar1 = (**(code **)(*(int *)(param_1[7] + 0x14) + 0xc))(param_1[7]);
            if (cVar1 == '\0') {
              return 0;
            }
            pbVar4 = (byte *)**(undefined4 **)(param_1[7] + 0x14);
            iVar3 = (*(undefined4 **)(param_1[7] + 0x14))[1];
          }
          uVar2 = (uint)*pbVar4;
          iVar3 = iVar3 + -1;
          pbVar4 = pbVar4 + 1;
        } while (uVar2 == 0xff);
        if (uVar2 != 0) {
          param_1[2] = uVar2;
          goto LAB_00585594;
        }
        uVar2 = 0xff;
      }
    }
    else {
LAB_00585594:
      if (param_4 <= param_3) break;
      if (*(char *)param_1[8] == '\0') {
        *(undefined4 *)(*(int *)param_1[7] + 0x14) = 0x71;
        (**(code **)(*(int *)param_1[7] + 4))((int *)param_1[7],0xffffffff);
        *(undefined1 *)param_1[8] = 1;
      }
      uVar2 = 0;
    }
    param_2 = param_2 << 8 | uVar2;
  }
  param_1[3] = param_2;
  param_1[6] = param_3;
  *param_1 = pbVar4;
  param_1[1] = iVar3;
  return 1;
}

