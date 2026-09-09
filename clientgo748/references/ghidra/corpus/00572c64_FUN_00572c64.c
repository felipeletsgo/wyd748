// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00572c64 | Name: FUN_00572c64


undefined4 FUN_00572c64(int *param_1)

{
  byte bVar1;
  undefined4 *puVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  byte *pbVar6;
  
  puVar2 = (undefined4 *)param_1[5];
  iVar5 = puVar2[1];
  pbVar6 = (byte *)*puVar2;
  do {
    if (iVar5 == 0) {
      cVar3 = (*(code *)puVar2[3])(param_1);
      if (cVar3 == '\0') {
        return 0;
      }
      pbVar6 = (byte *)*puVar2;
      iVar5 = puVar2[1];
    }
    bVar1 = *pbVar6;
    iVar5 = iVar5 + -1;
    pbVar6 = pbVar6 + 1;
    if (bVar1 == 0xff) {
      do {
        if (iVar5 == 0) {
          cVar3 = (*(code *)puVar2[3])(param_1);
          if (cVar3 == '\0') {
            return 0;
          }
          pbVar6 = (byte *)*puVar2;
          iVar5 = puVar2[1];
        }
        uVar4 = (uint)*pbVar6;
        iVar5 = iVar5 + -1;
        pbVar6 = pbVar6 + 1;
      } while (uVar4 == 0xff);
      if (uVar4 != 0) {
        if (*(int *)(param_1[100] + 0x58) != 0) {
          *(undefined4 *)(*param_1 + 0x14) = 0x70;
          *(undefined4 *)(*param_1 + 0x18) = *(undefined4 *)(param_1[100] + 0x58);
          *(uint *)(*param_1 + 0x1c) = uVar4;
          (**(code **)(*param_1 + 4))(param_1,0xffffffff);
          *(undefined4 *)(param_1[100] + 0x58) = 0;
        }
        param_1[0x5e] = uVar4;
        *puVar2 = pbVar6;
        puVar2[1] = iVar5;
        return 1;
      }
      *(int *)(param_1[100] + 0x58) = *(int *)(param_1[100] + 0x58) + 2;
    }
    else {
      *(int *)(param_1[100] + 0x58) = *(int *)(param_1[100] + 0x58) + 1;
    }
    *puVar2 = pbVar6;
    puVar2[1] = iVar5;
  } while( true );
}

