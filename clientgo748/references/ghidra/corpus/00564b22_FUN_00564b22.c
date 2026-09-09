// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00564b22 | Name: FUN_00564b22


undefined4 __thiscall FUN_00564b22(int *param_1,int *param_2)

{
  uint *puVar1;
  int iVar2;
  uint *puVar3;
  uint *puVar4;
  
  if ((param_1[1] != 0) && (param_1[0xe] != 0)) {
    FUN_0058ea3a(param_1[1]);
  }
  if ((param_1[2] != 0) && (param_1[0xf] != 0)) {
    FUN_0058ea3a(param_1[2]);
  }
  *param_1 = param_2[1];
  param_1[1] = *param_2;
  param_1[2] = param_2[0x12];
  puVar1 = (uint *)(param_1 + 6);
  puVar3 = (uint *)(param_2 + 10);
  puVar4 = puVar1;
  for (iVar2 = 6; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar4 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  }
  param_1[0xc] = param_2[2];
  param_1[0xd] = param_2[3];
  iVar2 = *param_1;
  if ((iVar2 == 0x31545844) || (iVar2 == 0x32545844)) {
LAB_00564bb6:
    *puVar1 = *puVar1 & 0xfffffffc;
    param_1[7] = param_1[7] & 0xfffffffc;
  }
  else {
    if (iVar2 != 0x32595559) {
      if (((iVar2 == 0x33545844) || (iVar2 == 0x34545844)) || (iVar2 == 0x35545844))
      goto LAB_00564bb6;
      if (iVar2 != 0x59565955) goto LAB_00564bbd;
    }
    *puVar1 = *puVar1 & 0xfffffffe;
  }
LAB_00564bbd:
  param_1[0xe] = 0;
  param_1[0xf] = 0;
  param_1[3] = param_1[8] - *puVar1;
  param_1[4] = param_1[9] - param_1[7];
  param_1[5] = param_1[0xb] - param_1[10];
  return 0;
}

