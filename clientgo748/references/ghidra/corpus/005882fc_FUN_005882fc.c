// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005882fc | Name: FUN_005882fc


void FUN_005882fc(undefined4 param_1,int param_2,int param_3,int param_4)

{
  int *piVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;
  int local_c;
  int local_8;
  
  if (param_3 < param_4) {
    local_8 = param_3 * 2;
    piVar6 = (int *)(param_3 * 0x20 + 0xc + param_2);
    do {
      piVar5 = (int *)0x0;
      iVar4 = 0;
      local_c = param_3;
      if (param_4 < local_8) {
        if (0 < local_8) {
          piVar1 = (int *)(param_2 + 0x18);
          do {
            if (iVar4 < *piVar1) {
              piVar5 = piVar1 + -6;
              iVar4 = *piVar1;
            }
            piVar1 = piVar1 + 8;
            local_c = local_c + -1;
          } while (local_c != 0);
        }
      }
      else if (0 < local_8) {
        piVar1 = (int *)(param_2 + 0x1c);
        do {
          if ((iVar4 < *piVar1) && (0 < piVar1[-1])) {
            piVar5 = piVar1 + -7;
            iVar4 = *piVar1;
          }
          piVar1 = piVar1 + 8;
          local_c = local_c + -1;
        } while (local_c != 0);
      }
      if (piVar5 == (int *)0x0) {
        return;
      }
      piVar6[-2] = piVar5[1];
      *piVar6 = piVar5[3];
      piVar6[2] = piVar5[5];
      piVar6[-3] = *piVar5;
      piVar6[-1] = piVar5[2];
      piVar6[1] = piVar5[4];
      iVar2 = (piVar5[1] - *piVar5) * 0x10;
      iVar4 = (piVar5[3] - piVar5[2]) * 0xc;
      cVar3 = iVar2 <= iVar4;
      if (!(bool)cVar3) {
        iVar4 = iVar2;
      }
      if (iVar4 < (piVar5[5] - piVar5[4]) * 8) {
        cVar3 = '\x02';
      }
      if (cVar3 == '\0') {
        iVar4 = (piVar5[1] + *piVar5) / 2;
        piVar5[1] = iVar4;
        piVar6[-3] = iVar4 + 1;
      }
      else if (cVar3 == '\x01') {
        iVar4 = (piVar5[3] + piVar5[2]) / 2;
        piVar5[3] = iVar4;
        piVar6[-1] = iVar4 + 1;
      }
      else if (cVar3 == '\x02') {
        iVar4 = (piVar5[5] + piVar5[4]) / 2;
        piVar5[5] = iVar4;
        piVar6[1] = iVar4 + 1;
      }
      FUN_00587fc4();
      FUN_00587fc4();
      local_8 = local_8 + 2;
      param_3 = param_3 + 1;
      piVar6 = piVar6 + 8;
    } while (param_3 < param_4);
  }
  return;
}

