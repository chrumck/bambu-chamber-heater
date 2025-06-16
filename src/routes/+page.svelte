<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";
  import Switch from "./Switch.svelte";
  import MainHeader from "./MainHeader.svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<div id="mainContainer">
  <MainHeader />
  <Switch id="light" label="Light" />
  <Switch id="heaterFanSet" label="Heater Fan" />
  <Switch id="doorFanSet" label="Door Fan" />
  <Switch id="auxFanSet" label="Aux Fan" />
</div>

<style>
  #mainContainer {
    padding: 0.75rem;

    display: flex;
    flex-direction: column;
    gap: 0.2rem;

    text-align: center;
    background-color: var(--clr-bkg);
    border-radius: 0.6rem;
    border-width: 1px;
    border-style: solid;
    border-color: var(--clr-white);
  }
</style>
