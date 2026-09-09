// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f220 | Name: FUN_0058f220


undefined4 * FUN_0058f220(undefined4 *param_1,undefined4 *param_2,uint param_3)

{
  uint uVar1;
  undefined4 *puVar2;
  
  if ((param_2 < param_1) && (param_1 < (undefined4 *)(param_3 + (int)param_2))) {
    param_2 = (undefined4 *)((param_3 - 4) + (int)param_2);
    puVar2 = (undefined4 *)((param_3 - 4) + (int)param_1);
    if (((uint)puVar2 & 3) == 0) {
      uVar1 = param_3 >> 2;
      param_3 = param_3 & 3;
      if (7 < uVar1) {
        for (; uVar1 != 0; uVar1 = uVar1 - 1) {
          *puVar2 = *param_2;
          param_2 = param_2 + -1;
          puVar2 = puVar2 + -1;
        }
        switch(param_3) {
        case 0:
          return param_1;
        case 2:
          goto switchD_0058f3d7_caseD_2;
        case 3:
          goto switchD_0058f3d7_caseD_3;
        }
        goto switchD_0058f3d7_caseD_1;
      }
    }
    else {
      switch(param_3) {
      case 0:
        goto switchD_0058f3d7_caseD_0;
      case 1:
        goto switchD_0058f3d7_caseD_1;
      case 2:
        goto switchD_0058f3d7_caseD_2;
      case 3:
        goto switchD_0058f3d7_caseD_3;
      default:
        uVar1 = param_3 - ((uint)puVar2 & 3);
        switch((uint)puVar2 & 3) {
        case 1:
          param_3 = uVar1 & 3;
          *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
          param_2 = (undefined4 *)((int)param_2 + -1);
          uVar1 = uVar1 >> 2;
          puVar2 = (undefined4 *)((int)puVar2 - 1);
          if (7 < uVar1) {
            for (; uVar1 != 0; uVar1 = uVar1 - 1) {
              *puVar2 = *param_2;
              param_2 = param_2 + -1;
              puVar2 = puVar2 + -1;
            }
            switch(param_3) {
            case 0:
              return param_1;
            case 2:
              goto switchD_0058f3d7_caseD_2;
            case 3:
              goto switchD_0058f3d7_caseD_3;
            }
            goto switchD_0058f3d7_caseD_1;
          }
          break;
        case 2:
          param_3 = uVar1 & 3;
          *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
          uVar1 = uVar1 >> 2;
          *(undefined1 *)((int)puVar2 + 2) = *(undefined1 *)((int)param_2 + 2);
          param_2 = (undefined4 *)((int)param_2 + -2);
          puVar2 = (undefined4 *)((int)puVar2 - 2);
          if (7 < uVar1) {
            for (; uVar1 != 0; uVar1 = uVar1 - 1) {
              *puVar2 = *param_2;
              param_2 = param_2 + -1;
              puVar2 = puVar2 + -1;
            }
            switch(param_3) {
            case 0:
              return param_1;
            case 2:
              goto switchD_0058f3d7_caseD_2;
            case 3:
              goto switchD_0058f3d7_caseD_3;
            }
            goto switchD_0058f3d7_caseD_1;
          }
          break;
        case 3:
          param_3 = uVar1 & 3;
          *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
          *(undefined1 *)((int)puVar2 + 2) = *(undefined1 *)((int)param_2 + 2);
          uVar1 = uVar1 >> 2;
          *(undefined1 *)((int)puVar2 + 1) = *(undefined1 *)((int)param_2 + 1);
          param_2 = (undefined4 *)((int)param_2 + -3);
          puVar2 = (undefined4 *)((int)puVar2 - 3);
          if (7 < uVar1) {
            for (; uVar1 != 0; uVar1 = uVar1 - 1) {
              *puVar2 = *param_2;
              param_2 = param_2 + -1;
              puVar2 = puVar2 + -1;
            }
            switch(param_3) {
            case 0:
              return param_1;
            case 2:
              goto switchD_0058f3d7_caseD_2;
            case 3:
              goto switchD_0058f3d7_caseD_3;
            }
            goto switchD_0058f3d7_caseD_1;
          }
        }
      }
    }
    switch(uVar1) {
    case 7:
      puVar2[7 - uVar1] = param_2[7 - uVar1];
    case 6:
      puVar2[6 - uVar1] = param_2[6 - uVar1];
    case 5:
      puVar2[5 - uVar1] = param_2[5 - uVar1];
    case 4:
      puVar2[4 - uVar1] = param_2[4 - uVar1];
    case 3:
      puVar2[3 - uVar1] = param_2[3 - uVar1];
    case 2:
      puVar2[2 - uVar1] = param_2[2 - uVar1];
    case 1:
      puVar2[1 - uVar1] = param_2[1 - uVar1];
      param_2 = param_2 + -uVar1;
      puVar2 = puVar2 + -uVar1;
    }
    switch(param_3) {
    case 1:
switchD_0058f3d7_caseD_1:
      *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
      return param_1;
    case 2:
switchD_0058f3d7_caseD_2:
      *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
      *(undefined1 *)((int)puVar2 + 2) = *(undefined1 *)((int)param_2 + 2);
      return param_1;
    case 3:
switchD_0058f3d7_caseD_3:
      *(undefined1 *)((int)puVar2 + 3) = *(undefined1 *)((int)param_2 + 3);
      *(undefined1 *)((int)puVar2 + 2) = *(undefined1 *)((int)param_2 + 2);
      *(undefined1 *)((int)puVar2 + 1) = *(undefined1 *)((int)param_2 + 1);
      return param_1;
    }
switchD_0058f3d7_caseD_0:
    return param_1;
  }
  puVar2 = param_1;
  if (((uint)param_1 & 3) == 0) {
    uVar1 = param_3 >> 2;
    param_3 = param_3 & 3;
    if (7 < uVar1) {
      for (; uVar1 != 0; uVar1 = uVar1 - 1) {
        *puVar2 = *param_2;
        param_2 = param_2 + 1;
        puVar2 = puVar2 + 1;
      }
      switch(param_3) {
      case 0:
        return param_1;
      case 2:
        goto switchD_0058f255_caseD_2;
      case 3:
        goto switchD_0058f255_caseD_3;
      }
      goto switchD_0058f255_caseD_1;
    }
  }
  else {
    switch(param_3) {
    case 0:
      goto switchD_0058f255_caseD_0;
    case 1:
      goto switchD_0058f255_caseD_1;
    case 2:
      goto switchD_0058f255_caseD_2;
    case 3:
      goto switchD_0058f255_caseD_3;
    default:
      uVar1 = (param_3 - 4) + ((uint)param_1 & 3);
      switch((uint)param_1 & 3) {
      case 1:
        param_3 = uVar1 & 3;
        *(undefined1 *)param_1 = *(undefined1 *)param_2;
        *(undefined1 *)((int)param_1 + 1) = *(undefined1 *)((int)param_2 + 1);
        uVar1 = uVar1 >> 2;
        *(undefined1 *)((int)param_1 + 2) = *(undefined1 *)((int)param_2 + 2);
        param_2 = (undefined4 *)((int)param_2 + 3);
        puVar2 = (undefined4 *)((int)param_1 + 3);
        if (7 < uVar1) {
          for (; uVar1 != 0; uVar1 = uVar1 - 1) {
            *puVar2 = *param_2;
            param_2 = param_2 + 1;
            puVar2 = puVar2 + 1;
          }
          switch(param_3) {
          case 0:
            return param_1;
          case 2:
            goto switchD_0058f255_caseD_2;
          case 3:
            goto switchD_0058f255_caseD_3;
          }
          goto switchD_0058f255_caseD_1;
        }
        break;
      case 2:
        param_3 = uVar1 & 3;
        *(undefined1 *)param_1 = *(undefined1 *)param_2;
        uVar1 = uVar1 >> 2;
        *(undefined1 *)((int)param_1 + 1) = *(undefined1 *)((int)param_2 + 1);
        param_2 = (undefined4 *)((int)param_2 + 2);
        puVar2 = (undefined4 *)((int)param_1 + 2);
        if (7 < uVar1) {
          for (; uVar1 != 0; uVar1 = uVar1 - 1) {
            *puVar2 = *param_2;
            param_2 = param_2 + 1;
            puVar2 = puVar2 + 1;
          }
          switch(param_3) {
          case 0:
            return param_1;
          case 2:
            goto switchD_0058f255_caseD_2;
          case 3:
            goto switchD_0058f255_caseD_3;
          }
          goto switchD_0058f255_caseD_1;
        }
        break;
      case 3:
        param_3 = uVar1 & 3;
        *(undefined1 *)param_1 = *(undefined1 *)param_2;
        param_2 = (undefined4 *)((int)param_2 + 1);
        uVar1 = uVar1 >> 2;
        puVar2 = (undefined4 *)((int)param_1 + 1);
        if (7 < uVar1) {
          for (; uVar1 != 0; uVar1 = uVar1 - 1) {
            *puVar2 = *param_2;
            param_2 = param_2 + 1;
            puVar2 = puVar2 + 1;
          }
          switch(param_3) {
          case 0:
            return param_1;
          case 2:
            goto switchD_0058f255_caseD_2;
          case 3:
            goto switchD_0058f255_caseD_3;
          }
          goto switchD_0058f255_caseD_1;
        }
      }
    }
  }
  switch(uVar1) {
  case 7:
    puVar2[uVar1 - 7] = param_2[uVar1 - 7];
  case 6:
    puVar2[uVar1 - 6] = param_2[uVar1 - 6];
  case 5:
    puVar2[uVar1 - 5] = param_2[uVar1 - 5];
  case 4:
    puVar2[uVar1 - 4] = param_2[uVar1 - 4];
  case 3:
    puVar2[uVar1 - 3] = param_2[uVar1 - 3];
  case 2:
    puVar2[uVar1 - 2] = param_2[uVar1 - 2];
  case 1:
    puVar2[uVar1 - 1] = param_2[uVar1 - 1];
    param_2 = param_2 + uVar1;
    puVar2 = puVar2 + uVar1;
  }
  switch(param_3) {
  case 1:
switchD_0058f255_caseD_1:
    *(undefined1 *)puVar2 = *(undefined1 *)param_2;
    return param_1;
  case 2:
switchD_0058f255_caseD_2:
    *(undefined1 *)puVar2 = *(undefined1 *)param_2;
    *(undefined1 *)((int)puVar2 + 1) = *(undefined1 *)((int)param_2 + 1);
    return param_1;
  case 3:
switchD_0058f255_caseD_3:
    *(undefined1 *)puVar2 = *(undefined1 *)param_2;
    *(undefined1 *)((int)puVar2 + 1) = *(undefined1 *)((int)param_2 + 1);
    *(undefined1 *)((int)puVar2 + 2) = *(undefined1 *)((int)param_2 + 2);
    return param_1;
  }
switchD_0058f255_caseD_0:
  return param_1;
}

