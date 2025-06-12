<script lang="ts">
  import { page } from "$app/state";
  import { connectWebSocket, defaultAppState, type AppState } from "$lib";
  import { onMount } from "svelte";

  const appState: AppState = $state(defaultAppState);
  const webSocketUrl = `ws://${page.url.host.replace(/\/+$/, "")}/ws`;

  onMount(() => connectWebSocket(appState, webSocketUrl));
</script>

<svelte:head>
  <style>
    body {
      margin: 0;
      padding: 0;
    }

    p {
      color: goldenrod;
      font-family: "Comic Sans MS", cursive;
      font-size: 2em;
    }
  </style>
</svelte:head>

<div class="mainContainer"></div>

<h1>Welcome to SvelteKit</h1>
<p>
  Visit <a href="https://svelte.dev/docs/kit">foo</a> to read the documentation
</p>
