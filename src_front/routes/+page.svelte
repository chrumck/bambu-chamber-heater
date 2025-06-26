<script lang="ts">
  import { onMount } from "svelte";
  import { pushState } from "$app/navigation";
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, invalidTemp, WsRequestCode, type AppState } from "$lib";
  import MainHeader from "./MainHeader.svelte";
  import Gauge from "./Gauge.svelte";
  import Led from "./Led.svelte";
  import Switch from "./Switch.svelte";
  import Popup from "./Popup.svelte";
  import Connecting from "./Connecting.svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;
  let webSocket: ReturnType<typeof connectWebSocket> | null = null;

  const closePopup = () => history.back();

  const openTempPopup = () => pushState("", { ...page.state, showTempPopup: true });
  const setChamberTemp = (newTemp: number) => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetTemp, newTemp]));
    appState.tempSetDegC = newTemp;
  };

  const openHeaterPopup = () => pushState("", { ...page.state, showHeaterPopup: true });
  const setHeaterTime = (newTime: number) => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetHeaterTimeLeft, newTime & 0xff, newTime >> 8]));
    appState.heaterTimeLeftMins = newTime;
  };

  const toggleLight = () => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetLight, appState.lightSet ? 0 : 1]));
    appState.lightSet = !appState.lightSet;
  };

  const toggleHeaterFan = () => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetHeaterFan, appState.heaterFanSet ? 0 : 1]));
    appState.heaterFanSet = !appState.heaterFanSet;
  };

  const toggleDoorFan = () => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetDoorFan, appState.doorFanSet ? 0 : 1]));
    appState.doorFanSet = !appState.doorFanSet;
  };

  const toggleAuxFan = () => {
    webSocket?.send(new Uint8Array([WsRequestCode.SetAuxFan, appState.auxFanSet ? 0 : 1]));
    appState.auxFanSet = !appState.auxFanSet;
  };

  onMount(() => {
    webSocket = connectWebSocket(appState, webSocketUrl);
    return webSocket.close;
  });
</script>

<div id="mainContainer">
  <MainHeader />

  <fieldset class="sectionContainer flexRow">
    <legend>Chamber Temp °C</legend>
    <Gauge label="Current" value={appState.tempDegC} invalidValue={invalidTemp} precision={1} />
    <Gauge label="Target" value={appState.tempSetDegC} invalidValue={0} />
    <input class="setButton" type="button" value="SET" onclick={openTempPopup} />
  </fieldset>

  <fieldset class="sectionContainer flexColumn">
    <legend>Heater</legend>
    <div class="flexRow">
      <div id="heaterOnGauge">
        <Led on={appState.heaterOn} />
        <span>Heater On</span>
      </div>
      <Gauge label="Time Left Mins" value={appState.heaterTimeLeftMins} />
      <input class="setButton" type="button" value="SET" onclick={openHeaterPopup} />
    </div>
    <div class="flexRow">
      <Gauge horizontal label="HeaterR" value={appState.heaterR} invalidValue={0} />
      <Gauge horizontal label="Duty Cycle" value={appState.heaterDutyCycle} invalidValue={0} precision={2} />
    </div>
  </fieldset>

  <fieldset class="sectionContainer">
    <legend>Lights and Fans</legend>

    <Switch id="light" label="Lights" on={appState.lightSet} ledOn={appState.lightSet} toggle={toggleLight} />
    <Switch
      id="heaterFanSet"
      label="Heater Fan"
      on={appState.heaterFanSet}
      ledOn={appState.heaterFanOn}
      toggle={toggleHeaterFan}
    />
    <Switch
      id="doorFanSet"
      label="Door Fan"
      on={appState.doorFanSet}
      ledOn={appState.doorFanOn}
      toggle={toggleDoorFan}
    />
    <Switch id="auxFanSet" label="Aux Fan" on={appState.auxFanSet} ledOn={appState.auxFanOn} toggle={toggleAuxFan} />
  </fieldset>
  <Popup
    isOpen={page.state.showTempPopup}
    label="Set Temp °C"
    min={0}
    max={100}
    step={1}
    startValue={appState.tempSetDegC}
    submit={setChamberTemp}
    close={closePopup}
  />
  <Popup
    isOpen={page.state.showHeaterPopup}
    label="Set Heater Time"
    min={0}
    max={48 * 60}
    step={1}
    startValue={appState.heaterTimeLeftMins}
    submit={setHeaterTime}
    close={closePopup}
  />
  <Connecting isOpen={!appState.connected} />
</div>

<style>
  #mainContainer {
    padding: 0.5rem;

    display: flex;
    flex-direction: column;
    gap: 0.3rem;

    background-color: var(--clr-bkg);
    border: 1px solid var(--clr-white);
    border-radius: 0.6rem;
  }

  .sectionContainer {
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.6rem;
    text-align: left;
    padding: 0 0.4rem 0.3rem;
  }

  .flexColumn {
    display: flex;
    flex-direction: column;
    gap: 0.6rem;
  }

  .flexRow {
    display: flex;
    flex-direction: row;
    justify-content: center;
    align-items: center;
    gap: 0.8rem;
  }

  .sectionContainer > legend {
    color: var(--clr-white-dark);
    font-size: 0.75rem;
    padding: 0 0.25rem;
  }

  .setButton {
    color: var(--clr-white);
    background-color: var(--clr-green);
    width: 5rem;
    height: 2.3rem;
    font-size: 0.8rem;
    padding: 0.5rem;
    margin-top: 0.2rem;
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.5rem;
    cursor: pointer;
  }

  .setButton:active {
    background-color: #4caf50;
  }

  #heaterOnGauge {
    display: flex;
    flex-direction: column;
    justify-content: end;
    align-items: center;
    gap: 0.3rem;
    color: var(--clr-white-dark);
    font-size: 0.6rem;
    width: 5.6rem;
    height: 2.6rem;
  }
</style>
