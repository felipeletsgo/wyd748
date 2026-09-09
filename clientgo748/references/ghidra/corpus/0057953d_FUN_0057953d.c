// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057953d | Name: FUN_0057953d


undefined4 FUN_0057953d(int param_1,int param_2,char *param_3,int param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  if (((param_3 == (char *)0x0) || (*param_3 != '1')) || (param_4 != 0x38)) {
    uVar2 = 0xfffffffa;
  }
  else if (param_1 == 0) {
    uVar2 = 0xfffffffe;
  }
  else {
    *(undefined4 *)(param_1 + 0x18) = 0;
    if (*(int *)(param_1 + 0x20) == 0) {
      *(code **)(param_1 + 0x20) = FUN_0058a351;
      *(undefined4 *)(param_1 + 0x28) = 0;
    }
    if (*(int *)(param_1 + 0x24) == 0) {
      *(code **)(param_1 + 0x24) = FUN_0058a361;
    }
    iVar1 = (**(code **)(param_1 + 0x20))(*(undefined4 *)(param_1 + 0x28),1,0x18);
    *(int *)(param_1 + 0x1c) = iVar1;
    if (iVar1 == 0) {
      uVar2 = 0xfffffffc;
    }
    else {
      *(undefined4 *)(iVar1 + 0x14) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0xc) = 0;
      if (param_2 < 0) {
        param_2 = -param_2;
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0xc) = 1;
      }
      if ((param_2 < 8) || (0xf < param_2)) {
        uVar2 = 0xfffffffe;
      }
      else {
        *(int *)(*(int *)(param_1 + 0x1c) + 0x10) = param_2;
        uVar2 = FUN_005899dc(param_1,~-(uint)(*(int *)(*(int *)(param_1 + 0x1c) + 0xc) != 0) &
                                     0x58a238,1 << ((byte)param_2 & 0x1f));
        *(undefined4 *)(*(int *)(param_1 + 0x1c) + 0x14) = uVar2;
        if (*(int *)(*(int *)(param_1 + 0x1c) + 0x14) != 0) {
          FUN_005794c0(param_1);
          return 0;
        }
        uVar2 = 0xfffffffc;
      }
      FUN_005794ff(param_1);
    }
  }
  return uVar2;
}

