// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058c74e | Name: FUN_0058c74e


int FUN_0058c74e(uint param_1,undefined4 param_2,int param_3,int *param_4,int *param_5,
                undefined4 param_6,undefined4 param_7,undefined4 param_8,int param_9)

{
  int iVar1;
  int iVar2;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = (**(code **)(param_9 + 0x20))(*(undefined4 *)(param_9 + 0x28),0x120,4);
  if (iVar1 == 0) {
    return -4;
  }
  iVar2 = FUN_0058c34a(param_3,param_1,0x101,&DAT_005a9868,&DAT_005a98e8,param_6,param_8,&local_8,
                       iVar1);
  if (iVar2 == 0) {
    if (*param_4 == 0) goto LAB_0058c833;
    iVar2 = FUN_0058c34a(param_3 + param_1 * 4,param_2,0,&DAT_005a9968,&DAT_005a99e0,param_7,param_8
                         ,&local_8,iVar1);
    if (iVar2 == 0) {
      if ((*param_5 != 0) || (param_1 < 0x102)) {
        iVar2 = 0;
        goto LAB_0058c83f;
      }
LAB_0058c817:
      *(char **)(param_9 + 0x18) = "empty distance tree with lengths";
    }
    else {
      if (iVar2 == -3) {
        *(char **)(param_9 + 0x18) = "oversubscribed distance tree";
        goto LAB_0058c83f;
      }
      if (iVar2 != -5) {
        if (iVar2 == -4) goto LAB_0058c83f;
        goto LAB_0058c817;
      }
      *(char **)(param_9 + 0x18) = "incomplete distance tree";
    }
  }
  else {
    if (iVar2 == -3) {
      *(char **)(param_9 + 0x18) = "oversubscribed literal/length tree";
      goto LAB_0058c83f;
    }
    if (iVar2 == -4) goto LAB_0058c83f;
LAB_0058c833:
    *(char **)(param_9 + 0x18) = "incomplete literal/length tree";
  }
  iVar2 = -3;
LAB_0058c83f:
  (**(code **)(param_9 + 0x24))(*(undefined4 *)(param_9 + 0x28),iVar1);
  return iVar2;
}

