<script lang="ts">
  import Led from "./Led.svelte";
  const { id, label } = $props();

  let isOn = $state(false);
</script>

<div class="switch">
  <span class="label">{label}</span>
  <label for={id}>
    <input type="checkbox" {id} bind:checked={isOn} />
  </label>
  <Led on={isOn} />
</div>

<style>
  .switch {
    --switch-width: 5rem;
    --switch-height: calc(var(--switch-width) / 2);
    --switch-text-size: 0.75rem;
    --switch-clr: var(--clr-white-dark);
    --switch-clr-bg: var(--clr-bkg);
    --switch-clr-bg-on: var(--clr-green);
    --switch-inset: 2px;
    --switch-duration: 300ms;

    --toggle-size: calc(var(--switch-height) - var(--switch-inset) * 3);
    --toggle-gap: calc(var(--toggle-size) * 1.4);
    --toggle-padding-left: calc(var(--toggle-size) * 0.2);
    --toggle-bg: var(--switch-clr);
    --toggle-bg-on: var(--clr-white);

    padding: 0.8rem 1.2rem 0.8rem 1rem;
    color: var(--clr-white);
    font-size: 1rem;

    display: flex;
    flex-direction: row;
    justify-content: end;
    align-items: center;
    gap: 1.5rem;
  }

  /* switch "on" */
  .switch:has(input[type="checkbox"]:checked) {
    --switch-clr-bg: var(--switch-clr-bg-on);
    --toggle-x: calc(50% + var(--switch-inset));
    --toggle-bg: var(--toggle-bg-on);
    --led-color: var(--led-color-on);
  }

  .switch:has(input[type="checkbox"]:focus-visible) label {
    outline-color: white;
  }

  .switch label {
    position: relative;
    cursor: pointer;
    overflow: hidden;
    width: var(--switch-width);
    height: var(--switch-height);
    border-radius: 999vw;
    border: 1px solid var(--switch-clr);
    outline: 1px dashed transparent;
    outline-offset: 4px;
    background-color: var(--switch-clr-bg);
    isolation: isolate;
    transition-property: background-color;
    transition-duration: var(--switch-duration);
    transition-timing-function: ease-in-out;
  }

  .switch input[type="checkbox"] {
    position: absolute;
    pointer-events: none;
    appearance: none;
    border: none;
    outline: none;
    border-radius: inherit;
    background-color: var(--toggle-bg);
    top: var(--switch-inset);
    left: var(--toggle-x, var(--switch-inset));
    width: var(--toggle-size);
    height: var(--toggle-size);
    display: flex;
    align-items: center;
    justify-content: center;
    gap: var(--toggle-gap);
    padding-left: var(--toggle-padding-left);
    transition-property: left, background-color;
    transition-duration: var(--switch-duration);
    transition-timing-function: ease-in-out;
  }

  .switch input[type="checkbox"]::before,
  .switch input[type="checkbox"]::after {
    font-size: var(--switch-text-size);
    position: relative;
  }

  .switch input[type="checkbox"]::before {
    content: "ON";
    color: var(--clr-white);
  }

  .switch input[type="checkbox"]::after {
    content: "OFF";
    color: var(--switch-clr);
  }
</style>
