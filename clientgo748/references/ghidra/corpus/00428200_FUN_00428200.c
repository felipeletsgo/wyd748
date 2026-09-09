// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428200 | Name: FUN_00428200


LPVOID * __fastcall FUN_00428200(LPVOID *param_1)

{
  HRESULT HVar1;
  int iVar2;
  undefined4 uVar3;
  
  param_1[3] = (LPVOID)0x0;
  param_1[2] = (LPVOID)0x0;
  param_1[1] = (LPVOID)0x0;
  *param_1 = (LPVOID)0x0;
  CoInitialize((LPVOID)0x0);
  HVar1 = CoCreateInstance((IID *)&DAT_005a9f7c,(LPUNKNOWN)0x0,3,(IID *)&DAT_005a9f0c,param_1);
  if (HVar1 == 0) {
    iVar2 = (*(code *)**(undefined4 **)*param_1)(*param_1,&DAT_005a9f6c,param_1 + 1);
    if (iVar2 == 0) {
      iVar2 = (*(code *)**(undefined4 **)*param_1)(*param_1,&DAT_005a9f1c,param_1 + 2);
      if (iVar2 == 0) {
        iVar2 = (*(code *)**(undefined4 **)*param_1)(*param_1,&DAT_005a9f4c,param_1 + 3);
        if (iVar2 == 0) {
          iVar2 = (*(code *)**(undefined4 **)*param_1)(*param_1,&DAT_005a9f5c,param_1 + 4);
          if (iVar2 == 0) {
            uVar3 = FUN_0055b288(0x465,0);
            iVar2 = (**(code **)(*(int *)param_1[4] + 0x34))(param_1[4],uVar3);
            if (iVar2 < 0) {
              *(undefined1 *)(param_1 + 5) = 0;
            }
            else {
              *(undefined1 *)(param_1 + 5) = 1;
            }
          }
          else {
            *(undefined1 *)(param_1 + 5) = 0;
          }
        }
        else {
          *(undefined1 *)(param_1 + 5) = 0;
        }
      }
      else {
        *(undefined1 *)(param_1 + 5) = 0;
      }
    }
    else {
      *(undefined1 *)(param_1 + 5) = 0;
    }
  }
  else {
    *(undefined1 *)(param_1 + 5) = 0;
  }
  return param_1;
}

