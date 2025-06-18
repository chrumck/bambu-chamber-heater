<script lang="ts">
  interface PopupProps {
    isOpen: boolean;
  }

  const { isOpen }: PopupProps = $props();
</script>

<div class="spinnerBackground {isOpen && 'isVisible'}">
  <span class="spinner"> </span>
</div>

<style>
  :root {
    --spinner-size: 1rem;
    --spinner-size-2: 2rem;
    --spinner-timing: 0.2s;
  }

  .spinnerBackground {
    position: fixed;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0, 0, 0, 0.8);
    display: flex;
    justify-content: center;
    align-items: center;
    z-index: 1000;

    opacity: 0;
    visibility: hidden;
    transition:
      opacity var(--spinner-timing) 0s,
      visibility 0s var(--spinner-timing);
  }

  .spinnerBackground.isVisible {
    opacity: 1;
    visibility: visible;
    transition:
      opacity var(--spinner-timing) 0s,
      visibility 0s 0s;
  }

  .spinner {
    width: var(--spinner-size);
    height: var(--spinner-size);
    border-radius: 50%;
    background-color: #fff;
    box-shadow:
      var(--spinner-size-2) 0 #fff,
      calc(-1 * var(--spinner-size-2)) 0 #fff;
    animation: flash 0.5s ease-out infinite alternate;
  }

  @keyframes flash {
    0% {
      background-color: #fff2;
      box-shadow:
        var(--spinner-size-2) 0 #fff2,
        calc(-1 * var(--spinner-size-2)) 0 #fff;
    }
    50% {
      background-color: #fff;
      box-shadow:
        var(--spinner-size-2) 0 #fff2,
        calc(-1 * var(--spinner-size-2)) 0 #fff2;
    }
    100% {
      background-color: #fff2;
      box-shadow:
        var(--spinner-size-2) 0 #fff,
        calc(-1 * var(--spinner-size-2)) 0 #fff2;
    }
  }
</style>
