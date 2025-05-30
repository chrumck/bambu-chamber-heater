<script>
  import manifest from "./manifest";

  const value = "/tutorial/image.gif";
</script>

<svelte:head>
  <link
    rel="manifest"
    href="data:application/manifest+json,{encodeURIComponent(manifest)}"
  />

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
