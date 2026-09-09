// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428d3c | Name: FUN_00428d3c


undefined4 __fastcall FUN_00428d3c(int param_1,LPCSTR param_2,char param_3)

{
  char cVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  int local_244;
  WCHAR local_23c [260];
  undefined1 local_34 [36];
  int *local_10;
  int *local_c [2];
  
  if (*(char *)(param_1 + 8) == '\0') {
    uVar2 = 0xffffffff;
  }
  else {
    if (param_3 == '\x01') {
      local_244 = 0;
    }
    else {
      if (*(int *)(param_1 + 0x10) < 2) {
        return 0xffffffff;
      }
      local_244 = *(int *)(param_1 + 0xc);
    }
    piVar3 = (int *)FUN_00429320();
    if (piVar3 == (int *)0x0) {
      uVar2 = 0xffffffff;
    }
    else {
      iVar4 = FUN_005906a7(param_2,local_34);
      if (iVar4 == 0) {
        MultiByteToWideChar(0,0,param_2,-1,local_23c,0x104);
        if ((local_244 == 0) || (iVar4 = FUN_004293c0(), iVar4 == 0)) {
          cVar1 = FUN_00428412();
          if (cVar1 == '\0') {
            uVar2 = 0xffffffff;
          }
          else {
            iVar4 = (**(code **)(*piVar3 + 0x38))(piVar3,local_23c,local_23c,local_c);
            if (iVar4 == 0) {
              iVar4 = (**(code **)(*local_c[0] + 0x2c))(local_c[0],u_Output_005b483c,&local_10);
              if (iVar4 == 0) {
                iVar4 = (**(code **)(*piVar3 + 0x30))(piVar3,local_10);
                if (iVar4 == 0) {
                  if (local_10 != (int *)0x0) {
                    (**(code **)(*local_10 + 8))(local_10);
                    local_10 = (int *)0x0;
                  }
                  if (local_c[0] != (int *)0x0) {
                    (**(code **)(*local_c[0] + 8))(local_c[0]);
                    local_c[0] = (int *)0x0;
                  }
                  FUN_00429420(0,0);
                  FUN_00429390();
                  if (param_3 == '\x01') {
                    uVar2 = 0;
                  }
                  else {
                    uVar2 = *(undefined4 *)(param_1 + 0xc);
                    if (*(int *)(param_1 + 0xc) + 1 == *(int *)(param_1 + 0x10)) {
                      *(undefined4 *)(param_1 + 0xc) = 1;
                    }
                    else {
                      *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + 1;
                    }
                  }
                }
                else {
                  if (local_10 != (int *)0x0) {
                    (**(code **)(*local_10 + 8))(local_10);
                  }
                  uVar2 = 0xffffffff;
                }
              }
              else {
                uVar2 = 0xffffffff;
              }
            }
            else {
              uVar2 = 0xffffffff;
            }
          }
        }
        else {
          uVar2 = 0xffffffff;
        }
      }
      else {
        uVar2 = 0xffffffff;
      }
    }
  }
  return uVar2;
}

