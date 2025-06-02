<script lang="ts">
  import { onMount } from "svelte";

  const value = "/tutorial/image.gif";

  let socket: WebSocket;

  onMount(() => {
    socket = new WebSocket("ws://localhost:8000/chat");

    socket.addEventListener("open", () => {
      console.log("WebSocket connection opened");
    });

    socket.addEventListener("message", (event) => {
      console.log("Message from server:", event.data);
    });

    socket.addEventListener("close", () => {
      console.log("WebSocket connection closed");
    });

    return () => {
      socket.close();
    };
  });
</script>

<svelte:head>
  <style>
    p {
      color: goldenrod;
      font-family: "Comic Sans MS", cursive;
      font-size: 2em;
    }
  </style>
</svelte:head>

<h1>Welcome to SvelteKit</h1>
<p>
  Visit <a href="https://svelte.dev/docs/kit">{value}</a> to read the documentation
</p>
