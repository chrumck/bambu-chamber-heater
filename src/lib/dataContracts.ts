export interface AppState {
  lastDataTimeStampMs: number;
  connected: boolean;

  tempDegC: number;
  tempSetDegC: number;
  heaterOnTimeLeftMins: number;
  heaterR: number;

  heaterOn: boolean;
  lightOn: boolean;
  heaterFanSet: boolean;
  heaterFanOn: boolean;
  auxFanSet: boolean;
  auxFanOn: boolean;
  doorVentFanSet: boolean;
  doorVentFanOn: boolean;
}
