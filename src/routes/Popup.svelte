<script lang="ts">
  import type { EventHandler, KeyboardEventHandler } from "svelte/elements";
  interface PopupProps {
    isOpen: boolean;
    label: string;
    min: number;
    max: number;
    step: number;
    value: number;
    submit: EventHandler<SubmitEvent>;
    close: () => void;
  }

  let popupElement = $state();

  const { isOpen, label, min, max, step, value, submit, close }: PopupProps = $props();

  const handleEscape: KeyboardEventHandler<any> = ({ key }) => key === "Escape" && close();
</script>

<svelte:window on:keyup={handleEscape} />

<!-- svelte-ignore a11y_click_events_have_key_events, a11y_no_static_element_interactions -->
<div
  bind:this={popupElement}
  class="popup {isOpen && 'isVisible'}"
  onclick={(event) => event.target === popupElement && close()}
>
  <div id="popupContent">
    <h4>{label}</h4>
    <form class="formContent" onsubmit={(event) => (event.preventDefault(), submit(event), close())}>
      <input class="button" id="btnCancel" type="button" value="CANCEL" onclick={close} />
      <input type="number" {min} {max} {step} {value} />
      <input class="button" type="submit" value="OK" />
    </form>
  </div>
</div>

<style>
  :root {
    --popup-timing: 0.2s;
    --popup-slide-distance: -1rem;
  }

  .popup {
    position: fixed;
    left: 0;
    top: 0;
    height: 100%;
    width: 100%;
    display: flex;
    justify-content: center;
    align-items: center;

    background-color: rgba(161, 161, 161, 0.5);
    opacity: 0;
    visibility: hidden;
    transition:
      opacity var(--popup-timing) 0s,
      visibility 0s var(--popup-timing);
  }

  .popup.isVisible {
    opacity: 1;
    visibility: visible;
    transition:
      opacity var(--popup-timing) 0s,
      visibility 0s 0s;
  }

  #popupContent {
    color: var(--clr-white);
    background: var(--clr-bkg);
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.5rem;
    box-shadow: 0 0 2rem rgba(0, 0, 0, 0.8);

    padding: 1rem;
    text-align: center;

    display: flex;
    flex-direction: column;
    gap: 1rem;

    transform: translateY(var(--popup-slide-distance));
    transition: transform var(--popup-timing);
  }

  .formContent {
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    align-items: center;
    gap: 1rem;
  }

  .isVisible .formContent {
    transform: translateY(0);
  }

  .formContent input {
    height: 2rem;
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.25rem;
    color: var(--clr-white);
    background-color: var(--clr-bkg);
    font-size: 1rem;
  }

  .formContent input.button {
    width: 5rem;
    font-size: 0.75rem;
  }

  .formContent input[type="number"] {
    width: 5rem;
    padding-left: 0.5rem;
  }

  .formContent input[type="submit"] {
    color: var(--clr-white);
    background-color: var(--clr-green);
    font-size: 1rem;
  }

  .formContent input#btnCancel {
    background-color: var(--clr-red);
  }
</style>
