// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056cb4d | Name: FUN_0056cb4d


void __thiscall FUN_0056cb4d(int param_1,uint param_2,float param_3,uint param_4)

{
  float fVar1;
  int extraout_ECX;
  undefined1 *puVar2;
  uint uVar3;
  float *pfVar4;
  float *pfVar5;
  
  if (*(int *)(param_1 + 0x1048) != 0) {
    param_4 = FUN_0056b152(param_4);
    param_1 = extraout_ECX;
  }
  puVar2 = (undefined1 *)
           (*(int *)(param_1 + 0x1050) * param_2 + *(int *)(param_1 + 0x1054) * (int)param_3 +
           *(int *)(param_1 + 0x18));
  param_2 = 0;
  if (*(int *)(param_1 + 0x1058) != 0) {
    pfVar4 = (float *)(param_4 + 8);
    do {
      uVar3 = 0;
      param_3 = 3.4028235e+38;
      param_4 = 0;
      pfVar5 = (float *)(param_1 + 0x38);
      do {
        fVar1 = (pfVar4[-2] - pfVar5[-2]) * (pfVar4[-2] - pfVar5[-2]) +
                (pfVar4[-1] - pfVar5[-1]) * (pfVar4[-1] - pfVar5[-1]) +
                (*pfVar4 - *pfVar5) * (*pfVar4 - *pfVar5) +
                (pfVar4[1] - pfVar5[1]) * (pfVar4[1] - pfVar5[1]);
        if (fVar1 < param_3) {
          uVar3 = param_4;
          param_3 = fVar1;
        }
        param_4 = param_4 + 1;
        pfVar5 = pfVar5 + 4;
      } while (param_4 < 0x100);
      *puVar2 = (char)uVar3;
      puVar2 = puVar2 + 1;
      param_2 = param_2 + 1;
      pfVar4 = pfVar4 + 4;
    } while (param_2 < *(uint *)(param_1 + 0x1058));
  }
  return;
}

