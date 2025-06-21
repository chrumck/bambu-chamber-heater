<script lang="ts">
  import { invalidTempDegC } from "$lib";

  interface Props {
    horizontal?: boolean;
    label: string;
    value: number;
    precision?: number;
  }

  const { horizontal, label, value, precision = 0 }: Props = $props();
</script>

{#if !horizontal}
  <fieldset class="gauge">
    <legend>{label}</legend>
    <input
      type={value === invalidTempDegC ? "text" : "number"}
      value={value === invalidTempDegC ? "---" : value.toFixed(precision)}
      disabled
    />
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

  .gauge input {
    text-align: right;
    padding-right: 0.25rem;
    color: var(--clr-white);
    background-color: var(--clr-bkg);
    border: none;
    width: 5rem;
    font-size: 1.2rem;
  }

  .gauge input[type="text"] {
    text-align: center;
  }

  .gaugeHorizontal {
    font-size: 0.75rem;
  }

  .gaugeHorizontal label {
    color: var(--clr-white-dark);
    margin-right: 0.5rem;
  }

  .gaugeHorizontal input {
    background-color: var(--clr-bkg);
    color: var(--clr-white-dark);

    border: none;
    width: 4rem;
  }
</style>
