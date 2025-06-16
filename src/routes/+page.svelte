<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";
  import Switch from "./Switch.svelte";
  import Led from "./Led.svelte";
  import MainHeader from "./MainHeader.svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<div id="mainContainer">
  <MainHeader />
  <fieldset class="sectionContainer sectionContainerHorizontal">
    <legend>Chamber Temp °C</legend>
    <fieldset class="gauge">
      <legend>Current</legend>
      <input type="number" value={appState.tempDegC} disabled />
    </fieldset>
    <fieldset class="gauge">
      <legend>Target</legend>
      <input type="number" value={appState.tempSetDegC} disabled />
    </fieldset>
    <input class="setButton" type="button" value="SET" />
  </fieldset>

  <fieldset class="sectionContainer sectionContainerVertical">
    <legend>Heater</legend>
    <div class="sectionContainerHorizontal">
      <div class="gauge" id="heaterOnGauge">
        <Led on={appState.heaterOn} />
        <span>Heater On</span>
      </div>
      <fieldset class="gauge">
        <legend>Time Left Mins</legend>
        <input type="number" value={appState.heaterOnTimeLeftMins} disabled />
      </fieldset>
      <input class="setButton" type="button" value="SET" />
    </div>
    <div class="sectionContainerHorizontal">
      <div class="gaugeHorizontal">
        <label for="heaterR">Heater R:</label>
        <input id="heaterR" type="number" value={appState.heaterR} disabled />
      </div>
      <div class="gaugeHorizontal">
        <label for="heaterDutyCycle">Duty Cycle:</label>
        <input id="heaterDutyCycle" type="number" value={appState.heaterDutyCycle} disabled />
      </div>
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

  .sectionContainerVertical {
    display: flex;
    flex-direction: column;
    gap: 0.6rem;
  }

  .sectionContainerHorizontal {
    display: flex;
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

  .gauge {
    color: var(--clr-white-dark);
    font-size: 0.6rem;
    text-align: center;
    padding: 0.25rem;
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.5rem;
  }

  .gauge > legend {
    padding: 0 0.25rem;
  }

  .gauge input[type="number"] {
    text-align: center;
    color: var(--clr-white);
    background-color: var(--clr-bkg);
    border: none;
    width: 5rem;
    font-size: 1.2rem;
  }

  #heaterOnGauge {
    display: flex;
    flex-direction: column;
    justify-content: end;
    align-items: center;
    gap: 0.3rem;
    border: none;
    width: 5.6rem;
    height: 2.6rem;
  }

  .gaugeHorizontal {
    font-size: 0.75rem;
  }

  .gaugeHorizontal label {
    color: var(--clr-white-dark);
    margin-right: 0.5rem;
  }

  .gaugeHorizontal input[type="number"] {
    background-color: var(--clr-bkg);
    color: var(--clr-white-dark);

    border: none;
    width: 4rem;
  }
</style>
