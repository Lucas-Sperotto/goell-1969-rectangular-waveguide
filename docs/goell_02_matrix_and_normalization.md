# Goell 1969 - Matrix Elements, Global Determinant, and Normalization

These notes rewrite the solver-relevant equations from Sections 2.3, 2.6, and 2.7 of the paper.

> REVIEW-PDF: delete each `REVIEW-PDF` note after checking the marked equation against the article scan.

## Matrix Matching Equations

Paper eqs. (6a)-(6d):

For the longitudinal electric field,

$$
E^{LA} A = E^{LC} C,
$$

for the longitudinal magnetic field,

$$
H^{LB} B = H^{LD} D,
$$

for the tangential electric field,

$$
E^{TA} A + E^{TB} B = E^{TC} C + E^{TD} D,
$$

and for the tangential magnetic field,

$$
H^{TA} A + H^{TB} B = H^{TC} C + H^{TD} D.
$$

Here `A`, `B`, `C`, and `D` are column vectors of the modal coefficients `a_n`, `b_n`, `c_n`, and `d_n`.

## Definitions Used In The Matrix Elements

Paper page 2140 defines:

$$
S = \sin(n\theta_m + \phi),
\qquad
\phi = 0 \ \text{or} \ \frac{\pi}{2},
$$

$$
C = \cos(n\theta_m + \phi),
\qquad
\phi = 0 \ \text{or} \ \frac{\pi}{2},
$$

$$
J = J_n(h r_m),
\qquad
K = K_n(p r_m),
$$

> REVIEW-PDF: this line may not belong as a standalone definition in the paper. The matrix elements seem to use the scaled quantities in the next line instead.
> Check whether the article actually defines raw derivatives here, or only the divided forms `J_n'(h r_m)/h` and `K_n'(p r_m)/p`.

$$
J' = J_n'(h r_m),
\qquad
K' = K_n'(p r_m),
$$

$$
J' = \frac{J_n'(h r_m)}{h},
\qquad
K' = \frac{K_n'(p r_m)}{p},
$$

and, to distinguish them from the radial derivatives above, I will denote the angular factors with bars:

$$
\bar J = \frac{n J_n(h r_m)}{h^2 r_m},
\qquad
\bar K = \frac{n K_n(p r_m)}{p^2 r_m}.
$$

The paper also defines

$$
Z_0 = (\frac{\mu_0}{\epsilon_0})^{1/2},
\qquad
\epsilon_r = \frac{\epsilon_1}{\epsilon_0}.
$$

## Boundary Geometry Factors

For `\theta < \theta_c`,

$$
R = \sin\theta_m,
\qquad
T = \cos\theta_m,
\qquad
r_m = \frac{a/2}{\cos\theta_m},
$$

and for `\theta > \theta_c`,

$$
R = -\cos\theta_m,
\qquad
T = \sin\theta_m,
\qquad
r_m = \frac{b/2}{\sin\theta_m}.
$$

At the corner `\theta = \theta_c`, the paper assumes the boundary to be perpendicular to the radial line, giving

$$
R = \cos(\theta_c + \frac{\pi}{4}),
\qquad
T = \cos(\theta_c - \frac{\pi}{4}),
\qquad
r_m = \frac{\sqrt{a^2 + b^2}}{4}.
$$

## Matrix Elements

Paper eqs. (7a)-(7l):

$$
e_{mn}^{LA} = J S,
$$

$$
e_{mn}^{LC} = K S,
$$

$$
h_{mn}^{LB} = J C,
$$

$$
h_{mn}^{LD} = K C,
$$

$$
e_{mn}^{TA} = -k_z (J' S R + \bar J C T),
$$

$$
e_{mn}^{TB} = k_0 Z_0 (\bar J S R + J' C T),
$$

$$
e_{mn}^{TC} = k_z (K' S R + \bar K C T),
$$

$$
e_{mn}^{TD} = -k_0 Z_0 (\bar K S R + K' C T),
$$

$$
h_{mn}^{TA} = \frac{\epsilon_r k_0}{Z_0} (\bar J C R - J' S T),
$$

$$
h_{mn}^{TB} = -k_z (J' C R - \bar J S T),
$$

$$
h_{mn}^{TC} = -\frac{k_0}{Z_0} (\bar K C R - K' S T),
$$

$$
h_{mn}^{TD} = k_z (K' C R - \bar K S T).
$$

## Orthogonality / Transverse-Field Note

Paper eqs. (8)-(9):

The transverse fields are orthogonal only when

$$
E_t \cdot H_t = E_r H_r + E_{\theta} H_{\theta} = 0.
$$

From eq. (3), the paper writes

> REVIEW-PDF: check the sign of the prefactor in eq. (9).
> The current line below uses `(k_z^2 - k^2)/k_z^2`, but the scan previously looked like `(k^2 - k_z^2)/k_z^2`.

$$
E_t \cdot H_t = \frac{k_z^2 - k^2}{k_z^2} ( \frac{\partial H_z}{\partial r}\frac{\partial E_z}{\partial r} + \frac{1}{r^2} \frac{\partial H_z}{\partial \theta} \frac{\partial E_z}{\partial \theta}).
$$

This is not part of the root search, but it matters later for field interpretation.

## Normalization

Paper eq. (10):

$$
h r = \left[k_1^2 - k_0^2 - p^2\right]^{1/2} r.
$$

The paper then introduces the normalized propagation quantity and the normalized radial quantity.

To keep the notation explicit in these notes:

- I denote the paper's vertical-axis quantity by `P^2_paper`,
- and the unsquared quantity by `P_paper = \sqrt{P^2_paper}`.

Paper eq. (11):

$$
P^2_{paper}
=
\frac{(k_z/k_0)^2 - 1}{n_r^2 - 1},
$$

paper eq. (12):

$$
\Omega = r k_0 (n_r^2 - 1)^{1/2},
$$

paper eq. (13):

> REVIEW-PDF: the exponent on the right-hand side is suspicious here.
> I expect the paper to read `n_r = k_1/k_0`, not `n_r = (k_1/k_0)^{1/2}`.

$$
n_r = \frac{k_1}{k_0}^{1/2},
$$

and therefore

$$
p r = P_{paper}\,\Omega,
$$

$$
h r = \Omega(1 - P^2_{paper})^{1/2}.
$$

For the propagation curves, the paper uses as horizontal variable

$$
B_{paper} = \frac{2b}{\lambda_0}(n_r^2 - 1)^{1/2},
$$

with `\lambda_0 = 2\pi/k_0`.

For small index difference, the paper also gives

$$
P^2_{paper}
\approx
\frac{k_z/k_0 - 1}{\Delta n_r},
\qquad
\Delta n_r = n_r - 1.
$$

## Global Matrix Equation

Paper eq. (18):

$$
[Q][T] = 0,
$$

with

$$
Q =
\begin{bmatrix}
E^{LA} & 0      & -E^{LC} & 0 \\
0      & H^{LB} & 0       & -H^{LD} \\
E^{TA} & E^{TB} & -E^{TC} & -E^{TD} \\
H^{TA} & H^{TB} & -H^{TC} & -H^{TD}
\end{bmatrix},
$$

and

$$
[T] =
\begin{bmatrix}
A \\
B \\
C \\
D
\end{bmatrix}.
$$

The modal condition is then

$$
\det[Q] = 0.
$$

## Root Search Notes From Section 2.7.1

The paper states:

1. test values of the normalized propagation variable were first sampled uniformly in `(0, 1)`;
2. Newton's method was then used to refine the roots;
3. for propagation curves, about one Newton step was typically used;
4. five harmonics required about `0.1 s` per determinant evaluation on an IBM 360/65.

## Scaling Note From Page 2144

To control overflow/underflow, the paper notes that rows or columns of the determinant may be multiplied by positive functions without shifting the zeros.

The text specifically says that a brute-force scaling was used:

> REVIEW-PDF: these two scaling formulas are still uncertain and should be checked directly in the scan before we trust them in the implementation notes.
> In particular, verify whether the numerator really includes the extra factor `d`, and confirm the exact denominator arguments.

- Bessel terms multiplied by

$$
\frac{h^d d}{\left|J_n(h b)\right|},
$$

- modified Bessel terms multiplied by

$$
\frac{p^d d}{K_n(p b)},
$$

where `d` is the average waveguide dimension.

The paper also says that `Z_0` was then set to unity because that does not shift the zeros of the determinant.

Please verify these two scaling formulas carefully against the PDF, because they are especially easy to corrupt in OCR.
