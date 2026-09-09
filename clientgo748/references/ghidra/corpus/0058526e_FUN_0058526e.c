// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058526e | Name: FUN_0058526e


void FUN_0058526e(int param_1,char param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  int iVar8;
  
  iVar8 = param_1;
  puVar3 = (undefined4 *)(*(code *)**(undefined4 **)(param_1 + 4))(param_1,1,0x74);
  *(undefined4 **)(param_1 + 0x184) = puVar3;
  *puVar3 = &LAB_00584741;
  puVar3[2] = FUN_00585233;
  puVar3[0x1c] = 0;
  if (param_2 == '\0') {
    iVar5 = (**(code **)(*(int *)(param_1 + 4) + 4))(param_1,1,0x500);
    piVar7 = puVar3 + 8;
    iVar8 = 10;
    do {
      *piVar7 = iVar5;
      piVar7 = piVar7 + 1;
      iVar5 = iVar5 + 0x80;
      iVar8 = iVar8 + -1;
    } while (iVar8 != 0);
    puVar3[1] = &LAB_0058492b;
    puVar3[3] = FUN_00584751;
    puVar3[4] = 0;
  }
  else {
    piVar7 = (int *)(param_1 + 0x20);
    piVar1 = (int *)(param_1 + 0xc4);
    param_1 = 0;
    if (0 < *piVar7) {
      piVar7 = (int *)(*piVar1 + 0xc);
      _param_2 = puVar3 + 0x12;
      do {
        iVar5 = *piVar7;
        iVar6 = iVar5;
        if (*(char *)(iVar8 + 200) != '\0') {
          iVar6 = iVar5 * 3;
        }
        iVar2 = *(int *)(iVar8 + 4);
        uVar4 = FUN_00577e41(piVar7[5],iVar5,iVar6);
        uVar4 = FUN_00577e41(piVar7[4],piVar7[-1],uVar4);
        uVar4 = (**(code **)(iVar2 + 0x14))(iVar8,1,1,uVar4);
        param_1 = param_1 + 1;
        *_param_2 = uVar4;
        piVar7 = piVar7 + 0x15;
        _param_2 = _param_2 + 1;
      } while (param_1 < *(int *)(iVar8 + 0x20));
    }
    puVar3[1] = FUN_0058492e;
    puVar3[3] = FUN_00584acd;
    puVar3[4] = puVar3 + 0x12;
  }
  return;
}

