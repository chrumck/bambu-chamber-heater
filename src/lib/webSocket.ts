import type { AppState } from "./dataContracts";
import { wsDeadMs, wsKeepAliveIntervalMs } from "./constants";

let webSocket: WebSocket | null = null;
let wsKeepAliveId: number | null = null;

export const connectWebSocket = (appState: AppState, webSocketUrl: string): (() => void) => {
  webSocket = new WebSocket(webSocketUrl);
  webSocket.onclose = handleWsClose(appState);
  webSocket.onopen = handleWsOpen(appState, webSocketUrl);
  webSocket.onmessage = handleWsMessage(appState);
  webSocket.onerror = handleWsError(appState);

  return () => webSocket?.close();
};

const handleWsClose: (appState: AppState) => WebSocket["onclose"] = (appState) => () => {
  clearInterval(wsKeepAliveId || undefined);
  wsKeepAliveId = null;
  appState.lastDataTimeStampMs = 0;
  appState.connected = false;
};

const handleWsOpen: (appState: AppState, webSocketUrl: string) => WebSocket["onopen"] =
  (appState, webSocketUrl) => () => {
    appState.lastDataTimeStampMs = new Date().getTime();
    appState.connected = true;

    wsKeepAliveId = setInterval(() => {
      appState.connected =
        webSocket?.readyState === WebSocket.OPEN &&
        new Date().getTime() - wsKeepAliveIntervalMs < appState.lastDataTimeStampMs;

      if (
        webSocket?.readyState !== WebSocket.CLOSED &&
        new Date().getTime() - appState.lastDataTimeStampMs < wsDeadMs
      ) {
        return;
      }

      webSocket?.close();
      webSocket = null;
      setTimeout(() => connectWebSocket(appState, webSocketUrl), 100);
    }, wsKeepAliveIntervalMs);
  };

const handleWsMessage: (appState: AppState) => WebSocket["onmessage"] = (appState) => async (event) => {
  appState.lastDataTimeStampMs = new Date().getTime();
  appState.connected = true;

  const dataBytes = new Uint8Array(await event.data.arrayBuffer());
};

const handleWsError: (appState: AppState) => WebSocket["onerror"] = (appState) => (event) => {
  console.warn(new Date(), "handleWsError", event);
  appState.lastDataTimeStampMs = 0;
  appState.connected = false;
  webSocket?.close();
};
