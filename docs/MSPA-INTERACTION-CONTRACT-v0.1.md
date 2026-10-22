# MSPA Interaction Contract v0.1

## 1. Propósito

Este documento define el contrato lógico mínimo entre MSPA y un proyecto anfitrión.

No define lenguaje de programación, ABI, formato binario, serialización concreta ni API final. Define **qué información debe poder entrar y salir** del sistema para mantener desacoplamiento, portabilidad y explicabilidad.

MSPA no ejecuta el mundo. El host no decide la psicología interna del agente.

---

## 2. Principio de interacción

El contrato fundamental se expresa mediante cinco familias de operación:

1. **OBSERVE** — entregar un estímulo a un agente.
2. **DECIDE** — ofrecer acciones posibles y solicitar una preferencia.
3. **REPORT** — informar el resultado real de una acción intentada.
4. **ADVANCE** — comunicar paso de tiempo lógico.
5. **EXPORT / IMPORT** — mover o persistir estado psicológico.

No es obligatorio que una implementación futura use estos nombres exactos, pero sí debe conservar estas responsabilidades separadas.

---

# 3. OBSERVE

## 3.1 Objetivo

El host comunica que un agente ha percibido algo psicológicamente relevante.

OBSERVE puede actualizar:

- creencias;
- emociones;
- ánimo;
- memoria;
- relaciones;
- metas;
- expectativas;
- aprendizaje;
- interpretación de normas;
- relación con una fuente de influencia.

OBSERVE no obliga a tomar una decisión inmediatamente.

---

## 3.2 Stimulus

Forma lógica mínima:

```text
Stimulus
    id
    type
    source
    target
    propositions[]
    perceptual_confidence
    salience
    timestamp
    namespace
    metadata
```

Los campos pueden ser opcionales según tipo.

### `id`

Identidad del estímulo/evento observable.

### `type`

Categoría semántica general o específica del Domain Pack.

Ejemplos:

- evento físico;
- testimonio;
- resultado de acción;
- sensación interna;
- inspiración/influencia;
- observación social;
- cambio ambiental.

### `source`

Entidad, agente, sistema o fuente percibida.

Puede ser desconocida.

### `target`

Entidad principalmente afectada, si existe.

### `propositions[]`

Contenido semántico percibido.

Ejemplo:

```text
mspa:harmed(Torvando, Mara)
```

### `perceptual_confidence`

Claridad de percepción aportada por el host.

No equivale automáticamente a certeza final de creencia.

### `salience`

Importancia perceptiva previa al appraisal subjetivo.

### `timestamp`

Tiempo lógico del mundo/host.

### `namespace`

Procedencia semántica del estímulo.

### `metadata`

Información opcional declarada por el Domain Pack y no necesariamente comprendida por el core.

---

## 3.3 Ejemplos de estímulo

### Evento físico

```text
source: Torvando
propositions:
    mspa:harmed(Torvando, Mara)
confidence: 0.95
```

### Testimonio

```text
source: Lencio
propositions:
    mspa:failed_to_help(Torvando, Mara)
confidence: 0.80
metadata:
    epistemic_source = testimony
```

### Estado interno

```text
type: internal_state
propositions:
    host:hunger(agent, critical)
```

### Inspiración

```text
type: influence
source: divine_player_01
propositions:
    mspa:desires(source, protect(Mara))
```

MSPA puede interpretar esa inspiración de manera diferente por agente.

---

# 4. PROPOSITION

## 4.1 Forma mínima

```text
Proposition
    predicate
    arguments[]
    polarity
    temporal_scope
    qualifiers
    namespace
```

Los argumentos deben tener roles semánticos cuando sea necesario.

Ejemplo:

```text
predicate: mspa:present_at
arguments:
    actor = Torvando
    location = Casa_Mara
    event = Incendio_17
polarity: true
```

---

## 4.2 Vocabulario base

MSPA define un vocabulario base interoperable.

Los Domain Packs amplían el vocabulario mediante namespaces propios.

Una proposición desconocida debe poder conservarse de forma opaca y transportable aunque el runtime receptor no pueda razonarla completamente.

---

# 5. DECIDE

## 5.1 Objetivo

El host comunica qué acciones son realmente posibles en este momento y solicita una preferencia.

MSPA no inventa una acción fuera del conjunto proporcionado.

DECIDE puede ejecutarse sin OBSERVE previo. Un agente puede decidir por metas, necesidades, aprendizaje, ánimo, aburrimiento, finalización de una tarea o paso de tiempo.

---

## 5.2 ActionOption

Forma lógica mínima:

```text
ActionOption
    id
    semantic_type
    actor
    target
    expected_effects[]
    costs[]
    risks[]
    tags[]
    context
    namespace
```

### `id`

Identidad concreta que el host puede ejecutar.

### `semantic_type`

Categoría comprensible por MSPA o por el Domain Pack.

Ejemplos:

- `mspa:communicate`
- `mspa:avoid`
- `mspa:help`
- `domain:harvest`

### `actor`

Agente que ejecutaría la acción.

### `target`

Entidad objetivo, si existe.

### `expected_effects[]`

Consecuencias que el agente/host espera razonablemente de la acción.

No son garantía del resultado real.

### `costs[]`

Costes relevantes declarados por el host.

### `risks[]`

Riesgos subjetivamente evaluables.

### `tags[]`

Características útiles para curvas y reglas del Domain Pack.

### `context`

Características generalizables del entorno.

### `namespace`

Origen semántico.

---

## 5.3 Regla de autoridad

El host filtra previamente acciones físicamente imposibles.

MSPA evalúa preferencia psicológica entre opciones válidas.

Las normas culturales o tabúes normalmente modifican utilidad; no convierten una acción físicamente válida en imposible salvo que el juego haya decidido una restricción absoluta externa a la psicología.

---

# 6. DECISION

## 6.1 Salida mínima

```text
Decision
    selected_action
    selected_target
    confidence
    intention
    expressed_state
    outgoing_statements[]
    explanation
```

### `selected_action`

ID de una ActionOption entregada por el host.

### `selected_target`

Objetivo relevante, cuando la opción lo permita.

### `confidence`

Qué tan dominante fue la preferencia respecto a alternativas cercanas.

No representa probabilidad objetiva de éxito.

### `intention`

Motivo psicológico abstracto.

Ejemplos:

- buscar apoyo;
- evitar amenaza;
- cumplir compromiso;
- obtener información;
- regular ira;
- explorar alternativa.

### `expressed_state`

Emoción/actitud que el agente deja observar, que puede diferir del estado interno.

### `outgoing_statements[]`

Proposiciones que el agente pretende transmitir cuando la acción lo permite.

El host decide cómo se representan: diálogo, icono, silencio, registro, etc.

### `explanation`

Traza resumida de las principales consideraciones que determinaron la elección.

---

# 7. REPORT

## 7.1 Objetivo

El host informa lo que realmente ocurrió tras intentar una acción.

Forma conceptual:

```text
ActionResult
    action_id
    actor
    status
    actual_effects[]
    observed_by[]
    costs_paid[]
    timestamp
    metadata
```

### `status`

Ejemplos:

- success;
- partial_success;
- failed;
- interrupted;
- impossible_after_selection.

### `actual_effects[]`

Proposiciones/resultados reales relevantes para aprendizaje y memoria.

---

## 7.2 Función psicológica

REPORT puede:

- confirmar o debilitar expectativas causales;
- crear frustración;
- reforzar hábitos;
- alterar relaciones;
- cerrar o suspender metas;
- generar nuevos estímulos observables para terceros.

El host decide qué terceros perciben el resultado y emite sus OBSERVE correspondientes.

---

# 8. ADVANCE

## 8.1 Objetivo

El host comunica paso de tiempo lógico.

MSPA no debe asumir tiempo real ni frecuencia fija.

ADVANCE permite actualizar diferidamente:

- decaimiento emocional;
- retorno de ánimo a baseline;
- olvido;
- metas temporales;
- relaciones;
- consolidación;
- envejecimiento psicológico si el Domain Pack lo usa.

La implementación debería favorecer marcas de tiempo y evaluación perezosa.

---

# 9. AgentPsychState

## 9.1 Propiedad

En ejecución normal el runtime MSPA puede mantener el estado internamente por rendimiento.

Debe existir una representación lógica exportable/importable.

```text
AgentPsychState
    identity
    schema_version
    namespace_origin
    personality
    values
    goals
    beliefs
    learned_expectations
    emotions
    mood
    memories
    relationships
    external_relationships
    cultural_beliefs
    temporal_markers
    random_stream_state
    pack_dependencies
```

No se fija formato físico todavía.

---

## 9.2 Portabilidad

Un estado exportado debe poder viajar entre runtimes sin perder:

- personalidad;
- memoria;
- creencias sobre mundo de origen;
- relaciones externas;
- expectativas aprendidas;
- normas percibidas;
- procedencia de conceptos.

---

# 10. Namespaces y referencias

## 10.1 Identidad portable

Referencias portables deben distinguir mundo/namespace de identidad local.

Forma conceptual:

```text
GlobalRef
    origin_world
    namespace
    local_id
    optional_metadata
```

---

## 10.2 Referencias no resolubles

Una memoria puede apuntar a una entidad inexistente en el mundo receptor.

MSPA conserva la referencia y metadatos psicológicamente relevantes. No debe eliminarla ni reasignarla automáticamente.

---

# 11. DomainPack

## 11.1 Propósito

El Domain Pack es el paquete de contenido que adapta MSPA a un juego concreto.

```text
DomainPack
    pack_id
    version
    namespace
    dependencies
    predicates
    stimulus_templates
    inference_rules
    action_categories
    consideration_curves
    learning_context_tags
    concept_mappings
    temporal_profile
    compatibility_policy
```

Las reglas de inferencia deben ser declarativas.

---

## 11.2 Compatibilidad multiplayer

Dos mundos pueden comparar:

- pack IDs;
- versiones;
- namespaces;
- vocabulario base compartido;
- mappings disponibles;
- extensiones desconocidas.

Conceptos desconocidos pueden mantenerse opacos y transportables.

---

# 12. Core Psychology Pack

MSPA debería proporcionar un paquete base para evitar que cada proyecto reconstruya desde cero:

- vocabulario general;
- personalidad base;
- emociones base;
- relaciones base;
- appraisal básico;
- algunas inferencias sociales generales;
- curvas de consideración iniciales;
- degradaciones por defecto.

Su contenido exacto debe validarse en sandbox antes de congelarlo.

---

# 13. Reglas declarativas de inferencia

Forma conceptual mínima:

```text
InferenceRule
    rule_id
    namespace
    premises[]
    conclusion_template
    base_confidence
    modifiers[]
    required_concepts[]
```

No se define sintaxis final todavía.

Una regla debe poder serializarse, versionarse, validarse y viajar con su Domain Pack.

---

# 14. Aprendizaje causal

Las expectativas aprendidas se representan mediante relaciones acción-contexto-resultado.

```text
LearnedExpectation
    action_semantics
    context_features[]
    expected_result
    confidence
    evidence_count
    last_update
```

El Domain Pack controla qué rasgos de contexto son generalizables.

MSPA debe evitar crecimiento ilimitado mediante políticas futuras de:

- compactación;
- fusión;
- olvido;
- especialización;
- límite de complejidad.

---

# 15. Cada agente se procesa individualmente

MSPA no requiere un objeto global “Cultura” para decidir.

Flujo social:

```text
A actúa
-> host obtiene salida observable
-> host genera estímulo para B
-> B procesa su propio estado
```

Las métricas poblacionales se calculan externamente leyendo estados individuales.

---

# 16. Degradación de módulos

Si una implementación permite módulos opcionales, debe declarar comportamiento cuando falten dependencias.

Ejemplos:

- sin memoria: no hay reactivación episódica;
- sin relaciones: se usa baseline neutral;
- sin creencias avanzadas: observación produce representación temporal simple;
- sin aprendizaje: expectativas no se actualizan;
- sin expresión: `expressed_state` puede ser vacío;
- sin transmisión social: `outgoing_statements` puede existir pero el host decide ignorarlo.

Una configuración inválida debe fallar explícitamente durante validación, no degradarse de forma silenciosa e imprevisible.

---

# 17. Determinismo contractual

Para una ejecución reproducible deben fijarse:

- orden de estímulos;
- orden de opciones;
- timestamps;
- estado inicial;
- seeds/streams pseudoaleatorios;
- versiones de packs;
- política numérica.

Se recomienda un stream independiente por agente para impedir que añadir agentes modifique azar de otros.

La garantía bit-a-bit multiplataforma queda fuera de v0.1 hasta seleccionar representación numérica.

---

# 18. Tres pruebas contractuales obligatorias

## 18.1 Mara

Debe expresarse únicamente mediante Stimulus, Propositions, ActionOptions, Decision y ActionResult.

No se permiten callbacks especiales del tipo `handle_mara_case`.

## 18.2 Inspiración

El jugador emite una influencia semántica. Dos agentes deben poder interpretarla de manera diferente sin cambiar el contrato.

## 18.3 Viajero extranjero

Un agente exportado desde Mundo A se importa en Mundo B conservando memoria, creencias y conceptos externos. Debe poder enfrentarse a prácticas desconocidas sin regla especial `if foreign_culture`.

---

# 19. Ejemplo de ciclo completo

```text
HOST
Mara insulta a Lencio

OBSERVE(Lencio, stimulus)

MSPA
- forma/actualiza creencias
- genera ira
- cambia relación
- codifica memoria

HOST
ofrece:
- continuar trabajo
- confrontar Mara
- marcharse
- hablar con Aldren

DECIDE(Lencio, options)

MSPA -> Decision
selected_action: hablar_con_Aldren
intention: buscar apoyo
outgoing_statement: “Mara me humilló”

HOST
realiza movimiento y conversación

REPORT(Lencio, success)

HOST
convierte la afirmación en Stimulus para Aldren

OBSERVE(Aldren, testimony)

MSPA
Aldren forma una creencia testimonial con su propia certeza
```

No se necesita un sistema global de rumores.

---

# 20. Estado del contrato

`MSPA Interaction Contract v0.1` es una propuesta lógica para sandbox.

Antes de convertirlo en API concreta deben validarse:

1. suficiencia de Proposition;
2. compatibilidad de namespaces;
3. utilidad de `expected_effects` y `context` en ActionOption;
4. forma de Decision y outgoing statements;
5. portabilidad real de AgentPsychState;
6. degradación de módulos;
7. tres escenarios contractuales obligatorios.

No se autoriza todavía elegir ABI, lenguaje, formato de archivo ni bindings definitivos a partir de este documento.
