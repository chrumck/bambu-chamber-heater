<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";
  import MainHeader from "./MainHeader.svelte";
  import Gauge from "./Gauge.svelte";
  import Led from "./Led.svelte";
  import Switch from "./Switch.svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<div id="mainContainer">
  <MainHeader />

  <fieldset class="sectionContainer flexRow">
    <legend>Chamber Temp °C</legend>
    <Gauge label="Current" value={appState.tempDegC} />
    <Gauge label="Target" value={appState.tempSetDegC} />
    <input class="setButton" type="button" value="SET" />
  </fieldset>

  <fieldset class="sectionContainer flexColumn">
    <legend>Heater</legend>
    <div class="flexRow">
      <div id="heaterOnGauge">
        <Led on={appState.heaterOn} />
        <span>Heater On</span>
      </div>
      <Gauge label="Time Left Mins" value={appState.heaterTimeLeftMins} />
      <input class="setButton" type="button" value="SET" />
    </div>
    <div class="flexRow">
      <Gauge horizontal label="Heater R" value={appState.heaterR} />
      <Gauge horizontal label="Duty Cycle" value={appState.heaterDutyCycle} />
    </div>
  </fieldset>

  <fieldset class="sectionContainer">
    <legend>Lights and Fans</legend>

    <Switch id="light" label="Lights" />
    <Switch id="heaterFanSet" label="Heater Fan" />
    <Switch id="doorFanSet" label="Door Fan" />
    <Switch id="auxFanSet" label="Aux Fan" />
  </fieldset>
</div>

<style>
  #mainContainer {
    padding: 0.75rem;

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
