# Goell 1969 - Matrix Elements, Global Determinant, and Normalization

These notes rewrite the solver-relevant equations from Sections 2.3, 2.6, and 2.7 of the paper.

## Matrix Matching Equations

Paper eqs. (6a)-(6d):

For the longitudinal electric field,

$$
E^{LA} A = E^{LC} C,
\tag{6a}
$$

for the longitudinal magnetic field,

$$
H^{LB} B = H^{LD} D,
\tag{6b}
$$

for the tangential electric field,

$$
E^{TA} A + E^{TB} B = E^{TC} C + E^{TD} D,
\tag{6c}
$$

and for the tangential magnetic field,

$$
H^{TA} A + H^{TB} B = H^{TC} C + H^{TD} D.
\tag{6d}
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
Z_0 = \left(\frac{\mu_0}{\epsilon_0}\right)^{1/2},
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
R = \cos\left(\theta_c + \frac{\pi}{4}\right),
\qquad
T = \cos\left(\theta_c - \frac{\pi}{4}\right),
\qquad
r_m = \frac{\sqrt{a^2 + b^2}}{4}.
$$

## Matrix Elements

Paper eqs. (7a)-(7l):

$$
e_{mn}^{LA} = J S,
\tag{7a}
$$

$$
e_{mn}^{LC} = K S,
\tag{7b}
$$

$$
h_{mn}^{LB} = J C,
\tag{7c}
$$

$$
h_{mn}^{LD} = K C,
\tag{7d}
$$

$$
e_{mn}^{TA} = -k_z\left(J' S R + \bar J C T\right),
\tag{7e}
$$

$$
e_{mn}^{TB} = k_0 Z_0\left(\bar J S R + J' C T\right),
\tag{7f}
$$

$$
e_{mn}^{TC} = k_z\left(K' S R + \bar K C T\right),
\tag{7g}
$$

$$
e_{mn}^{TD} = -k_0 Z_0\left(\bar K S R + K' C T\right),
\tag{7h}
$$

$$
h_{mn}^{TA} = \frac{\epsilon_r k_0}{Z_0}\left(\bar J C R - J' S T\right),
\tag{7i}
$$

$$
h_{mn}^{TB} = -k_z\left(J' C R - \bar J S T\right),
\tag{7j}
$$

$$
h_{mn}^{TC} = -\frac{k_0}{Z_0}\left(\bar K C R - K' S T\right),
\tag{7k}
$$

$$
h_{mn}^{TD} = k_z\left(K' C R - \bar K S T\right).
\tag{7l}
$$

## Orthogonality / Transverse-Field Note

Paper eqs. (8)-(9):

The transverse fields are orthogonal only when

$$
E_t \cdot H_t = E_r H_r + E_{\theta} H_{\theta} = 0.
\tag{8}
$$

From eq. (3), the paper writes

$$
E_t \cdot H_t
=
\frac{k^2 - k_z^2}{k_z^2}
\left(
\frac{\partial H_z}{\partial r}\frac{\partial E_z}{\partial r}
+
\frac{1}{r^2}
\frac{\partial H_z}{\partial \theta}
\frac{\partial E_z}{\partial \theta}
\right).
\tag{9}
$$

This is not part of the root search, but it matters later for field interpretation.

## Normalization

Paper eq. (10):

$$
h r = \left[k_1^2 - k_0^2 - p^2\right]^{1/2} r.
\tag{10}
$$

The paper then introduces the normalized propagation quantity and the normalized radial quantity.

To keep the notation explicit in these notes:

- I denote the paper's vertical-axis quantity by `P^2_paper`,
- and the unsquared quantity by `P_paper = \sqrt{P^2_paper}`.

Paper eq. (11):

$$
P^2_{paper}
=
\frac{\left(k_z/k_0\right)^2 - 1}{n_r^2 - 1},
\tag{11}
$$

paper eq. (12):

$$
\Omega = r k_0 \left(n_r^2 - 1\right)^{1/2},
\tag{12}
$$

paper eq. (13):

$$
n_r = \frac{k_1}{k_0},
\tag{13}
$$

and therefore

$$
p r = P_{paper}\,\Omega,
\tag{14}
$$

$$
h r = \Omega\left(1 - P^2_{paper}\right)^{1/2}.
\tag{15}
$$

For the propagation curves, the paper uses as horizontal variable

$$
B_{paper} = \frac{2b}{\lambda_0}\left(n_r^2 - 1\right)^{1/2},
\tag{16}
$$

with `\lambda_0 = 2\pi/k_0`.

For small index difference, the paper also gives

$$
P^2_{paper}
\approx
\frac{k_z/k_0 - 1}{\Delta n_r},
\qquad
\Delta n_r = n_r - 1.
\tag{17}
$$

## Global Matrix Equation

Paper eq. (18):

$$
[Q][T] = 0,
\tag{18}
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
\tag{19}
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
