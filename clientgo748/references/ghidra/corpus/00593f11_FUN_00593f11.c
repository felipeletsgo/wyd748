// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00593f11 | Name: FUN_00593f11


uint FUN_00593f11(uint param_1,int *param_2)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  undefined *puVar6;
  
  piVar4 = param_2;
  uVar1 = param_2[3];
  uVar2 = param_2[4];
  if (((uVar1 & 0x82) == 0) || ((uVar1 & 0x40) != 0)) {
LAB_0059401d:
    param_2[3] = uVar1 | 0x20;
  }
  else {
    if ((uVar1 & 1) != 0) {
      param_2[1] = 0;
      if ((uVar1 & 0x10) == 0) goto LAB_0059401d;
      *param_2 = param_2[2];
      param_2[3] = uVar1 & 0xfffffffe;
    }
    uVar1 = param_2[3];
    param_2[1] = 0;
    param_2 = (int *)0x0;
    piVar4[3] = uVar1 & 0xffffffef | 2;
    if (((uVar1 & 0x10c) == 0) &&
       (((piVar4 != (int *)&DAT_005c9998 && (piVar4 != (int *)&DAT_005c99b8)) ||
        (iVar5 = FUN_0059adc1(uVar2), iVar5 == 0)))) {
      FUN_0059ad7d(piVar4);
    }
    if ((*(ushort *)(piVar4 + 3) & 0x108) == 0) {
      iVar5 = 1;
      param_2 = (int *)FUN_00590b78(uVar2,&param_1,1);
    }
    else {
      iVar3 = piVar4[2];
      iVar5 = *piVar4 - iVar3;
      *piVar4 = iVar3 + 1;
      piVar4[1] = piVar4[6] + -1;
      if (iVar5 < 1) {
        if (uVar2 == 0xffffffff) {
          puVar6 = &DAT_005c9ed8;
        }
        else {
          puVar6 = (undefined *)((&DAT_013b8620)[(int)uVar2 >> 5] + (uVar2 & 0x1f) * 0x24);
        }
        if ((puVar6[4] & 0x20) != 0) {
          FUN_005950cc(uVar2,0,2);
        }
      }
      else {
        param_2 = (int *)FUN_00590b78(uVar2,iVar3,iVar5);
      }
      *(undefined1 *)piVar4[2] = (undefined1)param_1;
    }
    if (param_2 == (int *)iVar5) {
      return param_1 & 0xff;
    }
    piVar4[3] = piVar4[3] | 0x20;
  }
  return 0xffffffff;
}

