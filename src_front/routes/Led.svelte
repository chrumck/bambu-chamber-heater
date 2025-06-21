<script lang="ts">
  const { on } = $props();
</script>

<span class="led {on && 'led-on'}"></span>

<style>
  .led {
    --led-size: 0.75rem;
    --led-color: var(--clr-bg);
    --led-duration: 200ms;
    --led-blur: calc(var(--led-size) * 0.5);
    --led-blur-distance: -0.5rem;
    --led-blur-opacity: 0.3;

    position: relative;
    width: var(--led-size);
    height: var(--led-size);
    border: 1px solid var(--clr-white-dark);
    border-radius: 50%;
    background-color: var(--led-color);
    transition-property: background-color;
    transition-duration: var(--led-duration);
    transition-timing-function: ease-in-out;
  }

  .led-on {
    --led-color: rgb(50, 159, 255);
  }

  .led:before {
    content: "";
    position: absolute;
    inset: var(--led-blur-distance);
    border-radius: inherit;
    background-color: var(--led-color);
    filter: blur(var(--led-blur));
    opacity: var(--led-blur-opacity);
    transition-property: background-color;
    transition-duration: var(--led-duration);
    transition-timing-function: ease-in-out;
  }
</style>
