<script lang="ts">
  import type { Attachment } from "svelte/attachments";
  import icon from "$lib/assets/pig0_512.webp";
  import { getManifest, getObjectUrl, getObjectUrlFromUrl } from "$lib";

  const { children } = $props();

  const loadIcon: Attachment = (element) => {
    (async () => {
      const iconUrl = await getObjectUrlFromUrl(icon);
      element.setAttribute("href", iconUrl);
    })();
  };

  const loadManifest: Attachment = (element) => {
    (async () => {
      const iconUrl = await getObjectUrlFromUrl(icon);
      const manifestUrl = getObjectUrl(
        getManifest(iconUrl),
        "application/manifest+json"
      );
      element.setAttribute("href", manifestUrl);
    })();
  };
</script>

<svelte:head>
  <link rel="icon" {@attach loadIcon} />
  <link rel="manifest" {@attach loadManifest} />
</svelte:head>

{@render children()}
