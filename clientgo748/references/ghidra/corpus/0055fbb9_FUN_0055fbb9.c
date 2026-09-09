// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fbb9 | Name: FUN_0055fbb9


int FUN_0055fbb9(int *param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined1 local_18 [12];
  undefined4 local_c;
  int *local_8;
  
  local_8 = (int *)0x0;
  FUN_00564a04(1);
  if (param_1 != (int *)0x0) {
    (**(code **)(*param_1 + 0x18))(param_1,&local_8);
    (**(code **)(*param_1 + 0x1c))(param_1,&local_ec);
    (**(code **)(*param_1 + 0x20))(param_1,local_18);
  }
  uVar3 = 0xffffffff;
  piVar4 = &DAT_005a6160;
  piVar6 = &DAT_005a6138;
  piVar5 = &DAT_005a6138;
  if (&DAT_005a6160 < PTR_DAT_005c5084) {
    do {
      piVar6 = piVar5;
      if ((*piVar4 != 0) &&
         ((param_1 == (int *)0x0 ||
          (iVar1 = (**(code **)(*local_8 + 0x28))
                             (local_8,local_e8,local_ec,local_c,param_2,param_3,*piVar4), -1 < iVar1
          )))) {
        piVar6 = piVar4;
        if (*param_4 == *piVar4) break;
        piVar6 = piVar5;
        if ((((piVar4[8] != 0) && (uVar2 = FUN_0055fad6(), uVar2 != 0xffffffff)) && (uVar2 <= uVar3)
            ) && ((uVar2 != uVar3 || ((uint)piVar4[2] < (uint)piVar5[2])))) {
          uVar3 = uVar2;
          piVar6 = piVar4;
        }
      }
      piVar4 = piVar4 + 9;
      piVar5 = piVar6;
    } while (piVar4 < PTR_DAT_005c5084);
  }
  if (local_8 != (int *)0x0) {
    (**(code **)(*local_8 + 8))(local_8);
    local_8 = (int *)0x0;
  }
  FUN_00564a04(0);
  return *piVar6;
}

