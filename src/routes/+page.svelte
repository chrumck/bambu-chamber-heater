<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<div id="mainContainer">
  <h1>Welcome to SvelteKit</h1>
  <p>
    Visit <a href="https://svelte.dev/docs/kit">foo</a> to read the documentation
  </p>
</div>
