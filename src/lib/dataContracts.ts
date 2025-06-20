export interface AppState {
  lastDataTimeStampMs: number;
  connected: boolean;

  tempDegC: number;
  tempSetDegC: number;
  heaterTimeLeftMins: number;
  heaterR: number;
  heaterDutyCycle: number;

  heaterOn: boolean;
  lightSet: boolean;
  heaterFanSet: boolean;
  heaterFanOn: boolean;
  doorFanSet: boolean;
  doorFanOn: boolean;
  auxFanSet: boolean;
  auxFanOn: boolean;
}
