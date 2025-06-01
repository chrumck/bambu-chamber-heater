<script lang="ts">
  import icon from "$lib/assets/pig0_512.webp";
  import { getManifestUrl } from "./manifest";

  const manifestURL = getManifestUrl(icon);

  const value = "/tutorial/image.gif";
</script>

<svelte:window
  onload={() => {
    window.document
      .querySelector("#manifest-placeholder")
      ?.setAttribute("href", manifestURL);
  }}
/>

<svelte:head>
  <link rel="icon" href={icon} />

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
