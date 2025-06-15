<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<div id="mainContainer">
  <div class="switch">
    <span class="label">Light</span>
    <label for="light">
      <input type="checkbox" id="light" />
    </label>
    <span class="led"></span>
  </div>
  <div class="switch">
    <span class="label">Heater Fan</span>
    <label for="heaterFanSet">
      <input type="checkbox" id="heaterFanSet" />
    </label>
    <span class="led"></span>
  </div>
  <div class="switch">
    <span class="label">Door Fan</span>
    <label for="doorFanSet">
      <input type="checkbox" id="doorFanSet" />
    </label>
    <span class="led"></span>
  </div>
  <div class="switch">
    <span class="label">Aux Fan</span>
    <label for="auxFanSet">
      <input type="checkbox" id="auxFanSet" />
    </label>
    <span class="led"></span>
  </div>
</div>
