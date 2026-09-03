# Secondary Current Injection Test Set

A programmable, closed-loop current source built to commission and test a protective relay.
Commands a precise current, measures the delivered current independently, and injects it into
a protective relay to verify it trips at the correct pickup threshold — the hardware equivalent
of a relay commissioning tool.

---

## What this project demonstrates

- A **voltage-controlled current source (VCCS)** built from an op-amp and MOSFET that holds a
  commanded current constant regardless of supply voltage or load
- **Digital current command** via a microcontroller and DAC — set any target current in software
- **Independent measurement** of the delivered current (closed-loop verification: commanded vs. measured)
- A complete **relay commissioning demonstration** — injecting a known current into a protective
  relay and confirming it trips at the expected threshold, with correct inverse-time behavior

## Why it matters for P&C engineering

Secondary current injection is how protective relays are tested and commissioned in the field —
a known current is injected into the relay to verify its settings and trip behavior. This project
builds a working test set from the analog fundamentals (a feedback-regulated current source),
commands it digitally, and uses it to validate a real relay. It demonstrates both analog circuit
design and an understanding of how relays are commissioned in practice.

---

## How it works

1. **Command** — a microcontroller sets a target current by outputting a reference voltage through
   an MCP4725 DAC.
2. **Regulate** — an LM324 op-amp compares that reference voltage to the voltage across a sense
   resistor and drives an IRLZ44N MOSFET to force the sense voltage (and therefore the current) to
   match the command. This feedback loop is what makes it a *current source*: it holds the current
   constant even as supply voltage or load changes. Current = V_command / R_sense.
3. **Measure** — an INA219 current sensor independently measures the actual delivered current, so
   the commanded and measured values can be compared (closed-loop verification).
4. **Inject & trip** — the injected current is fed into the protective relay, whose firmware runs an
   IEC 60255 inverse-time trip characteristic. When the injected current exceeds pickup, the relay
   trips — validating the relay against a known, verified current.

## Circuit

- **MCP4725 DAC** — sets the command voltage from the microcontroller (I2C)
- **LM324 op-amp** — the feedback comparator driving the current-source loop
- **IRLZ44N MOSFET** — logic-level pass element carrying the injected current
- **10 Ω 2 W sense resistor** — develops the feedback voltage; sized so the sense signal sits in the
  op-amp's usable range (a smaller resistor produced a near-ground signal the LM324 could not
  regulate — a key design lesson)
- **INA219 current sensor** — independent measurement of delivered current
- **Arduino Nano** — commands the current, reads it back, and runs the relay trip logic

## Validation results

- Commanded vs. measured current agreed to within **~1%** (e.g. commanded 50 mA, measured ~50.6 mA)
  — consistent with sense-resistor tolerance and DAC quantization; a production test set would
  calibrate against a reference.
- The current source held its commanded value **constant across changing supply voltage** (12 V → 15 V),
  confirming true current-source behavior.
- Injecting a commanded current above the relay's pickup **tripped the relay**, with faster trips at
  higher injected currents — confirming both the test set and the relay's inverse-time behavior.

---

## Key design lessons

- A "logic-level" MOSFET still needs sufficient gate drive; the op-amp required a higher supply
  (~12 V) than the logic rail to drive the gate fully on.
- The LM324 op-amp cannot regulate a signal too close to ground — the sense resistor had to be sized
  so the feedback voltage sat comfortably within the op-amp's usable range.
- A common ground between the current-source circuit and the measurement/logic is required for the
  op-amp to compare voltages correctly.

## Contents

- `injection_test_set.ino` — firmware: commands the current, measures it back, runs the trip logic
- Circuit photo, commanded-vs-measured serial capture, and trip demonstration video

---

## Ties to the portfolio

This test set commissions the **IDMT overcurrent relay** project — one instrument built to validate
another. Together they demonstrate the full cycle: designing a protective relay, then building the
test equipment to verify it, exactly as relays are commissioned in the field.

*Part of a Protection & Control engineering portfolio.*
