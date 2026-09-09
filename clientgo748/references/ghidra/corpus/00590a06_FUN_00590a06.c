// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00590a06 | Name: FUN_00590a06


char * FUN_00590a06(char *param_1,int param_2,undefined4 *param_3)

{
  int *piVar1;
  uint uVar2;
  char *pcVar3;
  
  if (param_2 < 1) {
    param_1 = (char *)0x0;
  }
  else {
    FUN_00591bb3(param_3);
    pcVar3 = param_1;
    do {
      param_2 = param_2 + -1;
      if (param_2 == 0) break;
      piVar1 = param_3 + 1;
      *piVar1 = *piVar1 + -1;
      if (*piVar1 < 0) {
        uVar2 = FUN_00597b76(param_3);
      }
      else {
        uVar2 = (uint)*(byte *)*param_3;
        *param_3 = (byte *)*param_3 + 1;
      }
      if (uVar2 == 0xffffffff) {
        if (pcVar3 == param_1) {
          param_1 = (char *)0x0;
          goto LAB_00590a5a;
        }
        break;
      }
      *pcVar3 = (char)uVar2;
      pcVar3 = pcVar3 + 1;
    } while ((char)uVar2 != '\n');
    *pcVar3 = '\0';
LAB_00590a5a:
    FUN_00591c05(param_3);
  }
  return param_1;
}

