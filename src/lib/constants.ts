import type { AppState } from "./dataContracts";

/**
 * The default application state.
 */
export const defaultAppState: AppState = {
  lastDataTimeStampMs: 0,
  connected: false,

  tempDegC: 0,
  tempSetDegC: 0,
  heaterTimeLeftMins: 0,
  heaterR: 0,
  heaterDutyCycle: 0,

  heaterOn: false,
  lightSet: false,
  heaterFanSet: false,
  heaterFanOn: false,
  doorFanSet: false,
  doorFanOn: false,
  auxFanSet: false,
  auxFanOn: false,
};

/**
 * The web socket keepalive routine interval in milliseconds.
 */
export const wsKeepAliveIntervalMs = 3500;

/**
 * The web socket timeout in milliseconds after which the connection is considered dead and needs to be re-established.
 */
export const wsDeadMs = wsKeepAliveIntervalMs * 3;

/**
 * The web socket message structure.
 */
export enum WsMessageBytes {
  TempDegC1 = 0,
  TempDegC2 = 1,
  TempSetDegC = 2,
  HeaterOnTimeLeftMins1 = 3,
  HeaterOnTimeLeftMins2 = 4,
  HeaterR1 = 5,
  HeaterR2 = 6,
  HeaterDutyCycle = 7,
  Flags = 8,
}

/**
 * The web socket message length in bytes.
 */
export const wsMessageLength = Math.max(...Object.values(WsMessageBytes).filter((value) => typeof value === "number"));

/**
 * The web socket message flag positions in WsMessageBytes.Flags.
 */
export const enum WsMessageFlags {
  HeaterOn = 0,
  LightOn = 1,
  HeaterFanSet = 2,
  HeaterFanOn = 3,
  DoorVentFanSet = 4,
  DoorVentFanOn = 5,
  AuxFanSet = 6,
  AuxFanOn = 7,
}

/**
 * The web socket request codes.
 */
export const enum WsRequestCode {
  SetTemp = 0xa1,
  SetHeaterTimeLeft = 0xa2,
  SetLight = 0xa3,
  SetHeaterFan = 0xa4,
  SetDoorVentFan = 0xa5,
  SetAuxFan = 0xa6,
}
