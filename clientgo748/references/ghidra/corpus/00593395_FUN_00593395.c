// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593395 | Name: FUN_00593395


void FUN_00593395(undefined8 *param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  char *pcVar2;
  char *pcVar3;
  undefined1 local_2c [24];
  int local_14;
  int local_10;
  
  FUN_0059ab97(*param_1,&local_14,local_2c);
  iVar1 = local_10 + -1;
  pcVar2 = (char *)((uint)(local_14 == 0x2d) + param_2);
  FUN_0059ab20(pcVar2,param_3,&local_14);
  local_10 = local_10 + -1;
  if ((local_10 < -4) || (param_3 <= local_10)) {
    FUN_005931d7(param_2,param_3,param_4,&local_14,1);
  }
  else {
    if (iVar1 < local_10) {
      do {
        pcVar3 = pcVar2;
        pcVar2 = pcVar3 + 1;
      } while (*pcVar3 != '\0');
      pcVar3[-1] = '\0';
    }
    FUN_005932ee(param_2,param_3,&local_14,1);
  }
  return;
}

