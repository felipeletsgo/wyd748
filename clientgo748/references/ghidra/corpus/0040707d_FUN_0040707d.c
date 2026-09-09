// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040707d | Name: FUN_0040707d


void __thiscall
FUN_0040707d(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6)

{
  size_t sVar1;
  size_t sVar2;
  int iVar3;
  
  sVar1 = _strlen((char *)(param_1 + 0xe4c));
  sVar2 = _strlen((char *)(param_1 + 0xf4c));
  iVar3 = sVar1 + sVar2;
  if (*(int *)(param_1 + 0x30) == 1) {
    *(int *)(param_1 + 0x1054) = *(int *)(param_1 + 0x1054) + 1;
    *(int *)(param_1 + 0x1054) = *(int *)(param_1 + 0x1054) % 0x14;
    if (*(int *)(param_1 + 0x1054) == 10) {
      *(undefined1 *)(param_1 + iVar3 + 0x76a) = 0x7c;
      *(undefined1 *)(param_1 + iVar3 + 0x76b) = 0;
      FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),0);
    }
    else if (*(int *)(param_1 + 0x1054) == 0) {
      *(undefined1 *)(param_1 + iVar3 + 0x76a) = 0;
      FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),0);
    }
  }
  else if (*(char *)(param_1 + iVar3 + 0x76a) == '|') {
    *(undefined1 *)(param_1 + iVar3 + 0x76a) = 0;
    FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),0);
  }
  else {
    *(undefined1 *)(param_1 + iVar3 + 0x76a) = 0;
  }
  FUN_0040268c(param_2,param_3,param_4,param_5,param_6);
  return;
}

