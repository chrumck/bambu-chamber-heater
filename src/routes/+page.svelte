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
      font-family: Arial, Helvetica, sans-serif;
      user-select: none;
      background-image: linear-gradient(#262626, #8c8c8c);
      background-attachment: fixed;
    }

    #mainContainer {
      position: absolute;
      left: 50%;
      top: 50%;
      transform: translate(-50%, -50%);
      text-align: center;
      padding: 10px;
      color: #dfdede;
      background-color: #1d1d1d;
      border-radius: 10px;
      border-width: 3px;
      border-style: groove;
      border-color: #bab9b9;
    }
  </style>
</svelte:head>

<div id="mainContainer">
  <h1>Welcome to SvelteKit</h1>
  <p>
    Visit <a href="https://svelte.dev/docs/kit">foo</a> to read the documentation
  </p>
</div>
