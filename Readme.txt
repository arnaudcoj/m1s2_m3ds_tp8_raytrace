Camus Tristan
Cojez Arnaud

Nous avons répondu à toutes les questions du TP.

E2.Q1 :
On a :
   A + λu
et :
   x² + y² + z² = 1

A et u sont tous deux composés de coordonnées x, y et z
Nous pouvons donc considérer que :
   x = A.x + λ(u.x)
   y = A.y + λ(u.y)
   z = A.z + λ(u.z)

Par conséquent,
   (A.x + λ(u.x))² + (A.y + λ(u.y))² + (A.z + λ(u.z))² = 1
   => (A + λu)² = 1
   => A² + 2*A.λu + (λu)² = 1
   => (λu)² + 2*A.λu + A² = 1
   => (λu)² + 2*A.λu + A² - 1 = 0

On a donc un polynome a*x² + b*x + c = 0 où :
   x = λ
   a = u²
   b = 2*A.u
   c = A² - 1

Nous pouvons résoudre ce polynôme :
   Δ = b² - 4ac
   Δ = (2*A.u)² - 4*u²*(A² - 1)
   Δ = 4 * (A.u)² - 4 * u.u * (A.A - 1)

En fonction du signe de Δ, le nombre et la valeur des solutions seront différent. (résolution de polynôme de second degré)

E4.Q6.
Le scène 3 contient l'union de la scene 2 et 1, avec une intersection de deux sphères.

NOTE : Dans scene2.csg, la sphere était déjà réflechissante avant que l'on arrive à la question 4, ce qui peut porter à confusion.
