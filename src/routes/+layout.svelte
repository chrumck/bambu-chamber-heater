<script lang="ts">
  import { onMount } from "svelte";
  import icon from "$lib/assets/pig0_512.webp";
  import { getManifest } from "./manifest";
  import { getObjectUrl, getObjectUrlFromUrl } from "$lib";

  const { children } = $props();

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

{@render children()}
