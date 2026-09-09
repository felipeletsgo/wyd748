// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056fd19 | Name: FUN_0056fd19


/* WARNING (jumptable): Unable to track spacebase fully for stack */

int * FUN_0056fd19(void)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  int unaff_EBP;
  int *piVar4;
  
  FUN_00592900();
  iVar1 = *(int *)(unaff_EBP + 8);
  iVar2 = *(int *)(iVar1 + 4);
  piVar4 = (int *)0x0;
  if (iVar2 < 0x31545845) {
    if (iVar2 == 0x31545844) {
      pvVar3 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x20;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fc07(iVar1);
    }
    else if (iVar2 < 0x29) {
      if (iVar2 == 0x28) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0xd;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f660(iVar1);
      }
      else {
        switch(iVar2) {
        case 0x14:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 0;
          if (pvVar3 == (void *)0x0) {
LAB_00570410:
            piVar4 = (int *)0x0;
          }
          else {
            piVar4 = (int *)FUN_0056ed2d(iVar1);
          }
          break;
        case 0x15:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 1;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056ef35(iVar1);
          break;
        case 0x16:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 2;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056ef51(iVar1);
          break;
        case 0x17:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 3;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056ef6d(iVar1);
          break;
        case 0x18:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 4;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f276(iVar1);
          break;
        case 0x19:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 5;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f292(iVar1);
          break;
        case 0x1a:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 6;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f2ae(iVar1);
          break;
        case 0x1b:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 7;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f2ca(iVar1);
          break;
        case 0x1c:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 8;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f2e6(iVar1);
          break;
        case 0x1d:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 9;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f38f(iVar1);
          break;
        case 0x1e:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 10;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f60c(iVar1);
          break;
        case 0x1f:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 0xb;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f628(iVar1);
          break;
        default:
          goto switchD_0056fd5b_caseD_20;
        case 0x22:
          pvVar3 = operator_new(0x106c);
          *(void **)(unaff_EBP + 8) = pvVar3;
          *(undefined4 *)(unaff_EBP + -4) = 0xc;
          if (pvVar3 == (void *)0x0) goto LAB_00570410;
          piVar4 = (int *)FUN_0056f644(iVar1);
        }
      }
    }
    else if (iVar2 < 0x3f) {
      if (iVar2 == 0x3e) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0x14;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f846(iVar1);
      }
      else if (iVar2 == 0x29) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0xe;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f698(iVar1);
      }
      else if (iVar2 == 0x32) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0xf;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f6b4(iVar1);
      }
      else if (iVar2 == 0x33) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0x10;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f7d1(iVar1);
      }
      else if (iVar2 == 0x34) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0x11;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f7f2(iVar1);
      }
      else if (iVar2 == 0x3c) {
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0x12;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f80e(iVar1);
      }
      else {
        if (iVar2 != 0x3d) goto switchD_0056fd5b_caseD_20;
        pvVar3 = operator_new(0x106c);
        *(void **)(unaff_EBP + 8) = pvVar3;
        *(undefined4 *)(unaff_EBP + -4) = 0x13;
        if (pvVar3 == (void *)0x0) goto LAB_00570410;
        piVar4 = (int *)FUN_0056f82a(iVar1);
      }
    }
    else if (iVar2 == 0x3f) {
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x15;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056f862(iVar1);
    }
    else if (iVar2 == 0x40) {
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x16;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056f87e(iVar1);
    }
    else if (iVar2 == 0x41) {
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x17;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056f89a(iVar1);
    }
    else if (iVar2 == 0x43) {
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x18;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fb5f(iVar1);
    }
    else {
      if (iVar2 != 0x46) goto switchD_0056fd5b_caseD_20;
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x19;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fb7b(iVar1);
    }
  }
  else if (iVar2 < 0x36314c21) {
    if (iVar2 == 0x36314c20) {
      pvVar3 = operator_new(0x106c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x1a;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fb97(iVar1);
    }
    else if (iVar2 == 0x32545844) {
      pvVar3 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x21;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fc24(iVar1);
    }
    else if (iVar2 == 0x32595559) {
      pvVar3 = operator_new(0x109c);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x1f;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fd01(iVar1);
    }
    else if (iVar2 == 0x33545844) {
      pvVar3 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x22;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fc3c(iVar1);
    }
    else if (iVar2 == 0x34545844) {
      pvVar3 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x23;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fc54(iVar1);
    }
    else {
      if (iVar2 != 0x35545844) goto switchD_0056fd5b_caseD_20;
      pvVar3 = operator_new(0x10c4);
      *(void **)(unaff_EBP + 8) = pvVar3;
      *(undefined4 *)(unaff_EBP + -4) = 0x24;
      if (pvVar3 == (void *)0x0) goto LAB_00570410;
      piVar4 = (int *)FUN_0056fc6c(iVar1);
    }
  }
  else if (iVar2 == 0x36314c41) {
    pvVar3 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0x1b;
    if (pvVar3 == (void *)0x0) goto LAB_00570410;
    piVar4 = (int *)FUN_0056fbb3(iVar1);
  }
  else if (iVar2 == 0x36315220) {
    pvVar3 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0x1c;
    if (pvVar3 == (void *)0x0) goto LAB_00570410;
    piVar4 = (int *)FUN_0056fbcf(iVar1);
  }
  else if (iVar2 == 0x36315241) {
    pvVar3 = operator_new(0x106c);
    *(void **)(unaff_EBP + 8) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0x1d;
    if (pvVar3 == (void *)0x0) goto LAB_00570410;
    piVar4 = (int *)FUN_0056fbeb(iVar1);
  }
  else {
    if (iVar2 != 0x59565955) goto switchD_0056fd5b_caseD_20;
    pvVar3 = operator_new(0x109c);
    *(void **)(unaff_EBP + 8) = pvVar3;
    *(undefined4 *)(unaff_EBP + -4) = 0x1e;
    if (pvVar3 == (void *)0x0) goto LAB_00570410;
    piVar4 = (int *)FUN_0056fce9(iVar1);
  }
  *(undefined4 *)(unaff_EBP + -4) = 0xffffffff;
  if ((piVar4 != (int *)0x0) && (piVar4[4] != 0)) {
    (**(code **)(*piVar4 + 0xc))();
  }
switchD_0056fd5b_caseD_20:
  ExceptionList = *(void **)(unaff_EBP + -0xc);
  return piVar4;
}

