// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a3dd | Name: FUN_0059a3dd


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059a3dd(int param_1,int param_2,double param_3,double *param_4)

{
  double dVar1;
  double dVar2;
  int iVar3;
  
  dVar1 = (double)CONCAT44(param_2,param_1);
  if ((double)CONCAT44(param_2,param_1) < _DAT_005aa050) {
    dVar1 = -dVar1;
  }
  dVar2 = _DAT_005cc408;
  if (param_3._4_4_ == 0x7ff00000) {
    if (param_3._0_4_ != 0) {
LAB_0059a468:
      if (param_2 == 0x7ff00000) {
        if (param_1 != 0) {
          return 0;
        }
        if (_DAT_005aa050 < param_3) goto LAB_0059a503;
        if (param_3 < _DAT_005aa050) goto LAB_0059a49a;
      }
      else {
        if (param_2 != -0x100000) {
          return 0;
        }
        if (param_1 != 0) {
          return 0;
        }
        iVar3 = FUN_0059a50d(param_3);
        if (_DAT_005aa050 < param_3) {
          dVar2 = _DAT_005cc408;
          if (iVar3 == 1) {
            dVar2 = -_DAT_005cc408;
          }
          goto LAB_0059a503;
        }
        if (param_3 < _DAT_005aa050) {
          dVar2 = _DAT_005cc428;
          if (iVar3 != 1) {
            dVar2 = 0.0;
          }
          goto LAB_0059a503;
        }
      }
      dVar2 = 1.0;
      goto LAB_0059a503;
    }
    if (_DAT_005a4c28 < dVar1) goto LAB_0059a503;
    if (_DAT_005a4c28 <= dVar1) {
LAB_0059a42d:
      *param_4 = _DAT_005cc410;
      return 1;
    }
  }
  else {
    if ((param_3._4_4_ != -0x100000) || (param_3._0_4_ != 0)) goto LAB_0059a468;
    if (dVar1 <= _DAT_005a4c28) {
      if (_DAT_005a4c28 <= dVar1) goto LAB_0059a42d;
      goto LAB_0059a503;
    }
  }
LAB_0059a49a:
  dVar2 = 0.0;
LAB_0059a503:
  *param_4 = dVar2;
  return 0;
}

