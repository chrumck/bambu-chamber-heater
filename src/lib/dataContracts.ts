export interface AppState {
  lastDataTimeStampMs: number;
  connected: boolean;

  tempDegC: number;
  tempSetDegC: number;
  heaterOnTimeLeftMins: number;
  heaterR: number;
  heaterDutyCycle: number;

  heaterOn: boolean;
  lightOn: boolean;
  heaterFanSet: boolean;
  heaterFanOn: boolean;
  doorVentFanSet: boolean;
  doorVentFanOn: boolean;
  auxFanSet: boolean;
  auxFanOn: boolean;
}
