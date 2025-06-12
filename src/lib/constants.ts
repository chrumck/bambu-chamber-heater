import type { AppState } from "./dataContracts";

/**
 * The default application state.
 */
export const defaultAppState: AppState = {
  lastDataTimeStampMs: 0,
  connected: false,

  tempDegC: 0,
  tempSetDegC: 32,
  heaterOnTimeLeftMins: 0,
  heaterR: 0,

  heaterOn: false,
  lightOn: false,
  heaterFanSet: false,
  heaterFanOn: false,
  auxFanSet: false,
  auxFanOn: false,
  doorVentFanSet: false,
  doorVentFanOn: false,
};

/**
 * The web socket keepalive routine interval in milliseconds.
 */
export const wsKeepAliveIntervalMs = 3000;

/**
 * The web socket timeout in milliseconds after which the connection is considered dead and needs to be re-established.
 */
export const wsDeadMs = wsKeepAliveIntervalMs * 3;

/**
 * The web socket message structure.
 */
export enum WsMessageBytes {
  TempDegC = 0,
  TempSetDegC = 1,
  HeaterOnTimeLeftMins1 = 2,
  HeaterOnTimeLeftMins2 = 3,
  HeaterR1 = 4,
  HeaterR2 = 5,
  Flags = 6,
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
  AuxFanSet = 4,
  AuxFanOn = 5,
  DoorVentFanSet = 6,
  DoorVentFanOn = 7,
}
