# Step 04 — Multi-file project & error handling

Builds `person_t` (owns two heap strings) across `common.h` / `person.h` /
`person.c`, with `person_create` doing a three-step allocation that can fail
at any step.

## Concepts

1. **`.h` / `.c` split** — the header is your "exported surface": it declares
   types, prototypes, constants; the `.c` holds the bodies. **Include guards**
   (`#ifndef X / #define X / #endif`) exist because `#include` is dumb text
   pasting — without them a header pulled in twice defines everything twice and
   the compile fails. Go has no analogue: capitalisation decides visibility and
   the compiler dedups imports for you.
   - Rule of thumb: nothing goes in a `.h` that other `.c` files don't need.

2. **Error-code return pattern** — a fallible function returns `kv_result_t`
   (0 = OK, negatives = specific errors) and delivers its real result through an
   out-parameter (`person_t **out`). C has no `(value, error)` multi-return, so
   the value goes through a pointer and the error becomes the return value. The
   payoff: every failure mode is visible in the signature and can't be ignored.

3. **`goto cleanup` / `goto fail_N`** — C has no `defer`. When you've allocated
   1..N resources and step N+1 fails, you must free the earlier ones in reverse.
   The idiom: stacked labels ordered so control falls through, unwinding
   backwards. Forward jumps to a cleanup label are the **one** place `goto` is
   idiomatic in modern C — it keeps the error path linear instead of nesting.
   - Go's LIFO `defer` chain does this automatically; here you stage it by hand.

`person_free` is NULL-safe (frees strings then struct); `person_rename` frees
the old name before dup-ing the new. Both mirror the "own it → free it once"
discipline from step 03.
