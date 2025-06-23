<script lang="ts">
  import "./layout.css";
  import type { Attachment } from "svelte/attachments";
  import icon from "$lib/assets/pig0_512.webp";
  import { getManifest, getObjectUrl, getObjectUrlFromUrl } from "$lib";

  const { children } = $props();

  const loadIcon: Attachment = (element) => {
    (() => {
      element.setAttribute("href", icon);
    })();
  };

  const loadManifest: Attachment = (element) => {
    (() => {
      const manifestUrl = getObjectUrl(getManifest(icon), "application/manifest+json");
      element.setAttribute("href", manifestUrl);
    })();
  };
</script>

<svelte:head>
  <link rel="icon" href="data:," {@attach loadIcon} />
  <link rel="manifest" {@attach loadManifest} />
</svelte:head>

{@render children()}
