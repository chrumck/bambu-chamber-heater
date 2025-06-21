import type { AppState } from "./dataContracts";
import { wsDeadMs, wsKeepAliveIntervalMs, wsMessageConstants } from "./constants";

let webSocket: WebSocket | null = null;
let wsKeepAliveId: number | null = null;

export const connectWebSocket = (
  appState: AppState,
  webSocketUrl: string,
): { send: (request: Uint8Array) => void; close: () => void } => {
  webSocket = new WebSocket(webSocketUrl);
  webSocket.onclose = handleWsClose(appState);
  webSocket.onopen = handleWsOpen(appState, webSocketUrl);
  webSocket.onmessage = handleWsMessage(appState);
  webSocket.onerror = handleWsError(appState);

  return { close: () => webSocket?.close(), send: (request: Uint8Array) => webSocket?.send(request) };
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
      const currentTimeStamp = new Date().getTime();

      appState.connected =
        webSocket?.readyState === WebSocket.OPEN &&
        currentTimeStamp - wsKeepAliveIntervalMs < appState.lastDataTimeStampMs;

      if (appState.connected) return;

      if (webSocket?.readyState !== WebSocket.CLOSED && currentTimeStamp - appState.lastDataTimeStampMs < wsDeadMs) {
        return;
      }

      try {
        webSocket?.close();
      } catch (closingError) {
        console.error("Error closing socket", closingError);
      }
      webSocket = null;

      setTimeout(() => connectWebSocket(appState, webSocketUrl), 100);
    }, wsKeepAliveIntervalMs);
  };

const handleWsMessage: (appState: AppState) => WebSocket["onmessage"] = (appState) => async (event) => {
  appState.lastDataTimeStampMs = new Date().getTime();
  appState.connected = true;

  const dataBytes = new Uint8Array(await event.data.arrayBuffer());
  let tempDegC = dataBytes[0] + (dataBytes[1] << 8);
  appState.tempDegC = tempDegC / wsMessageConstants.tempFactor + wsMessageConstants.tempOffset;
};

const handleWsError: (appState: AppState) => WebSocket["onerror"] = (appState) => (event) => {
  console.warn(new Date(), "handleWsError", event);
  appState.lastDataTimeStampMs = 0;
  appState.connected = false;
  webSocket?.close();
};
