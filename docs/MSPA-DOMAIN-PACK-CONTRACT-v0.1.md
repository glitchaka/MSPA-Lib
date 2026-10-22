# MSPA Domain Pack Contract v0.1

## Propósito

Un **Domain Pack** describe la semántica psicológicamente relevante de un juego sin introducir reglas específicas del juego dentro del núcleo MSPA.

El runtime MSPA debe poder permanecer igual mientras cambian los juegos, siempre que cada host entregue un pack compatible.

El Domain Pack es, por tanto, una parte central del coste de integración y debe tratarse como producto de primera clase.

---

# 1. Metadatos obligatorios

Todo pack debe declarar conceptualmente:

- `pack_id`
- versión
- namespace
- dependencias
- compatibilidad
- versión mínima de MSPA requerida
- presupuesto de recursos permitido

No se fija todavía sintaxis ni formato físico.

---

# 2. Vocabulario de dominio

El pack registra conceptos y predicados que el juego necesita y que no pertenecen al vocabulario base MSPA.

Ejemplos:

```text
isla:harvested
isla:performed_miracle
isla:entered_foreign_island
```

Siempre que sea posible, un predicado de dominio debe mapear parte de su significado a predicados o categorías base de MSPA.

---

# 3. Predicate Schemas

Cada predicado interpretable debe declarar:

- roles;
- cardinalidad;
- rol con agencia;
- posibles objetivos;
- categorías de consecuencia;
- categorías de relevancia;
- posible intención;
- mapeos a vocabulario base;
- ContextTags relevantes;
- OutcomeTags posibles;
- pistas de appraisal.

Ejemplo conceptual:

```text
predicate: isla:harvested
roles:
  actor: required
  resource: required
  owner: optional
  location: optional
agency_role: actor
semantic_effects:
  - resource_gain
  - labor_success
  - ownership_interaction
possible_relevance:
  - prosperity
  - hunger
  - ownership
```

El esquema no fija una emoción. La interpretación depende del agente.

---

# 4. Event Templates

Un Event Template describe cómo un hecho del host se traduce a proposiciones y pistas semánticas.

Debe poder declarar:

- evento de dominio;
- proposiciones producidas;
- roles relevantes;
- saliencia base;
- posibles observadores;
- información que el host debe completar;
- Predicate Schemas involucrados.

El pack no decide quién percibió realmente el evento: eso corresponde al host.

---

# 5. Action Schemas

Cada tipo de acción debe declarar:

- `ActionTypeId`;
- roles;
- objetivo(s);
- consideraciones psicológicas relevantes;
- ContextTags;
- posibles OutcomeTags;
- riesgos/costes semánticos;
- mapeo a categorías base MSPA cuando exista.

Esto permite que MSPA valore acciones sin entender la implementación física del juego.

---

# 6. Consideraciones y curvas

El pack puede declarar qué consideraciones usa cada acción y cómo transformar entradas normalizadas.

Las curvas candidatas deben ser:

- serializables;
- editables;
- acotadas;
- reproducibles;
- preferiblemente lineales por tramos.

No se permite código arbitrario dentro de una curva.

---

# 7. Reglas de inferencia

Las reglas deben ser declarativas y usar exclusivamente predicados autorizados.

Cada regla puede declarar:

- condiciones;
- hipótesis resultante;
- certeza base;
- dependencias;
- prioridad o coste lógico;
- categorías de evidencia.

No puede:

- ejecutar código nativo;
- acceder a filesystem o red;
- crear recursión ilimitada;
- exceder presupuestos definidos por MSPA.

---

# 8. Aprendizaje

Para aprendizaje causal v1, el pack declara por acción:

- ContextTags permitidos;
- OutcomeTags permitidos;
- límite de combinaciones;
- condiciones para registrar experiencia;
- pesos iniciales opcionales.

Ejemplo:

```text
action: isla:fish
context_tags:
  - coast
  - morning
  - adequate_tool
outcome_tags:
  - food_gain
  - failure
```

El runtime aprende estadísticas sobre combinaciones permitidas, no contextos arbitrarios del mundo.

---

# 9. Valores y normas iniciales

Un pack puede ofrecer catálogos iniciales de:

- valores;
- normas personales;
- normas sociales percibidas;
- conceptos de prestigio;
- prácticas conocidas.

Esto no crea una cultura global. Sólo proporciona vocabulario y distribuciones iniciales que el host puede usar al crear agentes.

---

# 10. Influencia externa

El pack puede registrar clases de fuentes de influencia y cómo se describen semánticamente.

Ejemplo:

```text
isla:divine_inspiration
```

No puede imponer que el agente obedezca. Sólo declara la semántica necesaria para que MSPA pueda interpretarla.

---

# 11. Portabilidad

Un pack debe declarar qué conceptos son:

- portables;
- locales al mundo;
- resolubles por namespace;
- opacos pero preservables.

Esto es necesario para viajeros entre mundos y para conservar recuerdos de entidades ausentes.

---

# 12. Compatibilidad

El pack debe permitir comprobar:

- misma versión;
- versiones compatibles;
- vocabulario compartido;
- dependencias ausentes;
- predicados desconocidos;
- reglas no autorizadas.

Un agente importado no debe perder automáticamente una creencia porque el destino no entienda un predicado. Puede conservarla como dato opaco.

---

# 13. Seguridad y presupuestos

Todo pack externo se considera entrada no confiable.

Debe existir límite para:

- cantidad de predicados;
- Event Templates;
- Action Schemas;
- reglas de inferencia;
- tamaño de reglas;
- profundidad lógica;
- número de ContextTags;
- número de OutcomeTags;
- curvas;
- strings;
- tamaño total.

El pack nunca puede incluir código ejecutable arbitrario.

---

# 14. Core Psychology Pack

MSPA debe incluir un pack base con:

- vocabulario psicológico general;
- rasgos base;
- emociones base;
- relaciones;
- categorías generales de appraisal;
- predicados sociales comunes;
- reglas de inferencia mínimas;
- consideraciones generales;
- curvas iniciales de referencia.

Un Domain Pack extiende este núcleo en lugar de reemplazarlo completamente.

---

# 15. IslaVivaPsychPack como prueba obligatoria

Antes de congelar MSPA debe escribirse un `IslaVivaPsychPack v0.1` suficientemente concreto para describir sin excepciones internas:

- necesidades físicas relevantes;
- familia y cuidado;
- trabajo y producción;
- vivienda;
- nacimiento y muerte;
- duelo;
- llegada de barcos;
- extranjeros;
- viaje entre islas;
- recursos y propiedad;
- influencia divina;
- aprendizaje de hábitos y trabajo;
- comunicación de creencias;
- diferencias normativas entre comunidades.

El objetivo del pack de Isla Viva no es implementar el juego, sino demostrar que la frontera MSPA/host es realmente suficiente.

Si para expresarlo hace falta añadir código especial dentro del runtime, el contrato debe revisarse antes de iniciar producción.
