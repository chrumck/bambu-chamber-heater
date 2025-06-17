<script lang="ts">
  const { horizontal = false, label, value } = $props();
</script>

{#if !horizontal}
  <fieldset class="gauge">
    <legend>{label}</legend>
    <input type="number" {value} disabled />
  </fieldset>
{:else}
  <div class="gaugeHorizontal">
    <label for="gaugeHorizontal_{label}">Duty Cycle:</label>
    <input id="gaugeHorizontal_{label}" type="number" {value} disabled />
  </div>
{/if}

<style>
  .gauge {
    color: var(--clr-white-dark);
    font-size: 0.6rem;
    text-align: center;
    padding: 0.25rem;
    border: 1px solid var(--clr-white-dark);
    border-radius: 0.5rem;
  }

  .gauge > legend {
    padding: 0 0.25rem;
  }

  .gauge input[type="number"] {
    text-align: right;
    padding-right: 0.25rem;
    color: var(--clr-white);
    background-color: var(--clr-bkg);
    border: none;
    width: 5rem;
    font-size: 1.2rem;
  }

  .gaugeHorizontal {
    font-size: 0.75rem;
  }

  .gaugeHorizontal label {
    color: var(--clr-white-dark);
    margin-right: 0.5rem;
  }

  .gaugeHorizontal input[type="number"] {
    background-color: var(--clr-bkg);
    color: var(--clr-white-dark);

    border: none;
    width: 4rem;
  }
</style>
