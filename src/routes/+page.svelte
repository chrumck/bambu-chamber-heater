<script lang="ts">
  import { onMount } from "svelte";
  import icon from "$lib/assets/pig0_512.webp";
  import { getManifest } from "./manifest";
  import { getObjectUrl, getObjectUrlFromUrl } from "$lib";

  const value = "/tutorial/image.gif";

  onMount(async () => {
    const iconUrl = await getObjectUrlFromUrl(icon);

    window.document
      .querySelector("#favicon-placeholder")
      ?.setAttribute("href", iconUrl);

    const manifestUrl = getObjectUrl(
      getManifest(iconUrl),
      "application/manifest+json"
    );

    window.document
      .querySelector("#manifest-placeholder")
      ?.setAttribute("href", manifestUrl);
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
