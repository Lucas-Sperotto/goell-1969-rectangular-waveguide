# Goell 1969 - Field Expansions, Symmetry, and Matching Points

These notes rewrite the solver-relevant equations from Sections 2.0, 2.1, and 2.2 of the paper in renderable Markdown math.

## Notation Convention Used In These Notes

Two glyphs in the scan/OCR are easy to misread, so I am naming them explicitly here:

- `P^2_paper`: the normalized propagation variable plotted on the vertical axis in Figs. 16-22.
  In the repository, this is currently stored in the CSV column `Pprime`.
- `B_paper`: the horizontal plotting variable from eq. (16).
  In the repository, this is currently called `B`.

Please compare the rendered math with the PDF glyph-by-glyph and correct any symbol that should use a different letter style.

## Geometry

The core is rectangular, centered at the origin, with full dimensions `a` (x direction) and `b` (y direction). The propagation direction is `+z`.

In the first quadrant, the corner lies at

$$
\left(\frac{a}{2}, \frac{b}{2}\right),
$$

and the corner angle is

$$
\theta_c = \tan^{-1}\!\left(\frac{b}{a}\right).
$$

## Longitudinal Field Expansions

Paper eqs. (1a)-(1d):

$$
E_{zi} =
\sum_{n=0}^{\infty}
a_n J_n(hr)\,\sin(n\theta + \phi_n)\,
\exp[i(k_z z - \omega t)],
$$

$$
H_{zi} =
\sum_{n=0}^{\infty}
b_n J_n(hr)\,\sin(n\theta + \psi_n)\,
\exp\!\left[i(k_z z - \omega t)\right],
\tag{1b}
$$

inside the core, and

$$
E_{zo} =
\sum_{n=0}^{\infty}
c_n K_n(pr)\,\sin(n\theta + \phi_n)\,
\exp\!\left[i(k_z z - \omega t)\right],
\tag{1c}
$$

$$
H_{zo} =
\sum_{n=0}^{\infty}
d_n K_n(pr)\,\sin(n\theta + \psi_n)\,
\exp\!\left[i(k_z z - \omega t)\right],
\tag{1d}
$$

outside the core.

## Radial Wavenumbers

Paper eqs. (2a)-(2b):

$$
h = \left(k_1^2 - k_z^2\right)^{1/2},
\tag{2a}
$$

$$
p = \left(k_z^2 - k_0^2\right)^{1/2},
\tag{2b}
$$

with

$$
k_1 = \omega(\mu_0 \epsilon_1)^{1/2},
\qquad
k_0 = \omega(\mu_0 \epsilon_0)^{1/2}.
$$

## Transverse Field Components

Paper eqs. (3a)-(3d), where `k` can be either `k_1` or `k_0` depending on the region:

$$
E_r =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{\partial E_z}{\partial r}
+
\frac{\mu_0 \omega}{k_z r}
\frac{\partial H_z}{\partial \theta}
\right],
\tag{3a}
$$

$$
E_{\theta} =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{1}{r}\frac{\partial E_z}{\partial \theta}
-
\frac{\mu_0 \omega}{k_z}
\frac{\partial H_z}{\partial r}
\right],
\tag{3b}
$$

$$
H_r =
\frac{i k_z}{k^2 - k_z^2}
\left[
-
\frac{k^2}{\mu_0 \omega k_z r}
\frac{\partial E_z}{\partial \theta}
+
\frac{\partial H_z}{\partial r}
\right],
\tag{3c}
$$

$$
H_{\theta} =
\frac{i k_z}{k^2 - k_z^2}
\left[
\frac{k^2}{\mu_0 \omega k_z}
\frac{\partial E_z}{\partial r}
+
\frac{1}{r}\frac{\partial H_z}{\partial \theta}
\right].
\tag{3d}
$$

## Tangential Field On The Rectangle Boundary

Paper eqs. (4a)-(4b):

On the vertical sides,

$$
E_t = \pm\left(E_r \sin\theta + E_{\theta}\cos\theta\right),
\qquad
-\theta_c < \theta < \theta_c,
\qquad
\pi-\theta_c < \theta < \pi+\theta_c,
\tag{4a}
$$

and on the horizontal sides,

$$
E_t = \pm\left(-E_r \cos\theta + E_{\theta}\sin\theta\right),
\qquad
\theta_c < \theta < \pi-\theta_c,
\qquad
\pi+\theta_c < \theta < 2\pi-\theta_c.
\tag{4b}
$$

The paper states that similar expressions exist for the tangential magnetic field.

## Symmetry About The x Axis

Section 2.1 states that two phase families must exist:

1. first type:

$$
\phi_n = 0,
\qquad
\psi_n = \frac{\pi}{2},
$$

2. second type:

$$
\phi_n = \frac{\pi}{2},
\qquad
\psi_n = \pi.
$$

In implementation terms, the paper later packages this as one phase parameter `\phi` in the `S` and `C` factors of eq. (7):

$$
S = \sin(n\theta_m + \phi),
\qquad
C = \cos(n\theta_m + \phi),
\qquad
\phi \in \left\{0,\frac{\pi}{2}\right\}.
$$

This means:

- if `\phi = 0`, then `S = \sin(n\theta_m)` and `C = \cos(n\theta_m)`;
- if `\phi = \pi/2`, then `S = \cos(n\theta_m)` and `C = -\sin(n\theta_m)`.

## Symmetry About The y Axis

The paper argues that any given mode must consist entirely of either odd harmonics or even harmonics.

For the illustrative example in Section 2.1, letting

$$
\alpha = \theta - \frac{\pi}{2},
$$

eq. (1c) is rewritten as

$$
E_{zo}
=
\sum_{n=0}^{\infty}
c_n K_n(pr)
\left[
\sin(n\alpha)\cos\frac{n\pi}{2}
+
\cos(n\alpha)\sin\frac{n\pi}{2}
\right].
\tag{5}
$$

From this, the paper concludes:

- pure symmetry about the y axis requires all `n` odd,
- pure antisymmetry about the y axis requires all `n` even.

## Matching Points

Section 2.2 gives the matching-point rules used in the computations of Section 3.

### Odd-harmonic cases

For odd harmonics, the matching points are

$$
\theta_m = \frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,2,\ldots,N,
$$

where `N` is the number of space harmonics.

### Even-harmonic cases, unity aspect ratio

For even harmonics and `a/b = 1`:

- components with even symmetry about `\theta = 0` use

$$
\theta_m = \frac{(m - 1/2)\pi}{2N},
\qquad
m = 1,2,\ldots,N,
$$

- components with odd symmetry about `\theta = 0` use

$$
\theta_m = \frac{(m - N - 1/2)\pi}{2(N-1)},
\qquad
m = N+1, N+2, \ldots, 2N-1.
$$

This leads to `4N - 2` unknown coefficients instead of `4N`.

### Even-harmonic cases, non-unity aspect ratio

For `a/b \neq 1`, the paper states:

- all points follow the first formula above,
- except that the first and last points for the odd `z` component are omitted.

That statement is easy to mis-implement, so it is worth checking carefully against the scan.
