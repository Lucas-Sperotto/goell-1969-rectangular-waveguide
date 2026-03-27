# Goell 1969 - Review Checklist For PDF Cross-Check

This file is intentionally short and is meant to be used side-by-side with the rendered equations in:

- [goell_01_field_expansions.md](./goell_01_field_expansions.md)
- [goell_02_matrix_and_normalization.md](./goell_02_matrix_and_normalization.md)

## High-Priority Items To Verify Against The Scan

1. Confirm the exact glyph used for the vertical plotting variable in eq. (11) and on the y-axis of Figs. 16-22.
   In the repo we have been calling this `Pprime`, but the scan may really correspond to `P^2`.

2. Confirm the exact glyph used for the horizontal plotting variable in eq. (16) and on the x-axis of Figs. 16-22.
   In the repo we have been calling this `B`, but the scan glyph can look like beta or a stylized capital B.

3. Confirm the phase-family statement in Section 2.1:

   - first type: `\phi_n = 0`, `\psi_n = \pi/2`
   - second type: `\phi_n = \pi/2`, `\psi_n = \pi`

4. Confirm that the `S` and `C` definitions on page 2140 really use the same `\phi` parameter for both:

   $$
   S = \sin(n\theta_m + \phi),
   \qquad
   C = \cos(n\theta_m + \phi),
   \qquad
   \phi = 0 \ \text{or} \ \pi/2.
   $$

5. Confirm the signs in eqs. (7e)-(7l), especially:

   - the leading minus sign in `(7e)`
   - the `k_0 Z_0` factor in `(7f)` and `(7h)`
   - the `\epsilon_r k_0 / Z_0` factor in `(7i)`
   - the leading minus sign in `(7k)`

6. Confirm the definitions of the angular factors:

   $$
   \bar J = \frac{n J_n(h r_m)}{h^2 r_m},
   \qquad
   \bar K = \frac{n K_n(p r_m)}{p^2 r_m}.
   $$

7. Confirm the piecewise geometry factors:

   - for `\theta < \theta_c`:
     `R = \sin\theta_m`, `T = \cos\theta_m`, `r_m = (a/2)\sec\theta_m`
   - for `\theta > \theta_c`:
     `R = -\cos\theta_m`, `T = \sin\theta_m`, `r_m = (b/2)\csc\theta_m`

8. Confirm the special rule in Section 2.2 for even harmonics and `a/b \neq 1`:

   - all points follow the first formula,
   - except the first and last points for the odd `z` component, which are omitted.

9. Confirm the exact block structure of `Q` in eq. (18), especially the minus signs in the third and fourth block rows.

10. Confirm the scaling formulas on page 2144.
    Those OCR lines are the least trustworthy part of the extraction.

## What I Expect You To Mark Up

If you want, the easiest review loop is:

- mark anything that is symbolically wrong,
- mark anything that is numerically equivalent but not written exactly as in the paper,
- mark anything whose glyph naming should be changed to match the article.

After your pass, I can use your corrections to tighten both the documentation and the solver implementation.
