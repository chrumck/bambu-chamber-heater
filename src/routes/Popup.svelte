<script lang="ts">
  const { label, min, max, step = 1 } = $props();
</script>

<div class="popup isVisible">
  <div class="popupContainer">
    <div class="inputContainer">
      <label for="input">{label}:</label>
      <input id="input" type="number" {min} {max} {step} />
    </div>
  </div>
</div>

<style>
  :root {
    --popup-timing: 0.3s;
  }

  .popup {
    position: fixed;
    left: 0;
    top: 0;
    height: 100%;
    width: 100%;
    background-color: rgba(161, 161, 161, 0.5);
    opacity: 0;
    visibility: hidden;
    transition:
      opacity var(--popup-timing) 0s,
      visibility 0s var(--popup-timing);
    display: flex;
    justify-content: center;
    align-items: center;
  }

  .popup.isVisible {
    opacity: 1;
    visibility: visible;
    transition:
      opacity var(--popup-timing) 0s,
      visibility 0s 0s;
  }

  .popupContainer {
    position: relative;
    background: var(--clr-bkg);
    border: 1px solid var(--clr-white);
    border-radius: 0.5rem;
    box-shadow: 0 0 2rem rgba(0, 0, 0, 0.8);

    padding: 1rem;

    transform: translateY(-40px);
    transition: transform var(--popup-timing);
  }

  .isVisible .popupContainer {
    transform: translateY(0);
  }
</style>
