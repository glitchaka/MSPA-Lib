# MSPA-Lib — Arquitectura de código v0.1

> Estado: **plan de arquitectura de implementación**. Documento completamente agnóstico al host.

## 1. Invariante de separación

MSPA no incorpora nombres, clases, personajes, escenarios, repositorios, namespaces ni reglas pertenecientes a un consumidor concreto.

El core sólo conoce contratos genéricos. Los proyectos consumidores realizan externamente el mapeo entre sus entidades, eventos y acciones y MSPA.

> **MSPA posee estado psicológico y lógica de interpretación/decisión. El host posee el mundo, las entidades reales, la percepción física, las acciones disponibles y su ejecución.**

## 2. Lenguaje, estándar y ABI

La primera implementación se define en **C++17**.

Requisitos:
- sin dependencia obligatoria de motor;
- biblioteca construible como estática o dinámica;
- ownership explícito;
- orden estable cuando afecte resultados;
- RNG determinista propio;
- validación defensiva de toda entrada externa;
- límites de memoria y complejidad;
- fachada C opcional como ABI para otros lenguajes;
- ninguna excepción C++ cruza la API pública ni la fachada C.

MSPA usa un tipo propio `Result<T>` para errores esperables, sin depender de `std::expected`.

### Política numérica y compilación

Para builds de conformidad:
- MSVC: `/fp:strict`;
- GCC/Clang: sin `-ffast-math` y, cuando proceda, sin contracción FMA (`-ffp-contract=off` o equivalente);
- no depender de orden de evaluación no especificado;
- evitar funciones trascendentales en curvas configurables cuando exista representación por tablas o segmentos;
- `PsychTime` usa entero de 64 bits.

Estas reglas reducen divergencias, pero no implican todavía garantía bit-a-bit entre toolchains distintos.

## 3. Modelo estructural

MSPA no modela jerarquías del mundo del host.

Tipos contractuales principales:
- `RuntimeId`
- `AgentId`
- `EntityRef`
- `WorldId`
- `NamespaceId`
- `ConceptId`
- `PredicateId`
- `StimulusId`
- `GoalId`
- `BeliefId`
- `MemoryId`
- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- `PackId`
- `PsychTime`

Los IDs fuertes evitan mezclar accidentalmente dominios distintos.

## 4. Múltiples runtimes en un proceso

Debe ser válido crear varias instancias `Runtime` simultáneas dentro del mismo proceso.

Cada runtime posee de forma aislada:

```text
Runtime
  RuntimeId
  RuntimeConfig
  RegistrySnapshot
  AgentStore
  SeedRoot
  DiagnosticsContext
  ResourcePolicy
```

Reglas:
- no existe estado mutable global obligatorio;
- no existe RNG global;
- no existe `AgentStore` global;
- un runtime no puede observar ni modificar otro salvo exportación/importación explícita del host;
- cachés compartidas entre runtimes sólo pueden compartirse si son inmutables desde la perspectiva de la simulación;
- destruir un runtime libera exclusivamente sus recursos propios.

## 5. Árbol de módulos previsto

```text
MSPA-Lib/
├── include/mspa/
│   ├── api/
│   ├── ids/
│   ├── model/
│   ├── runtime/
│   ├── config/
│   ├── diagnostics/
│   └── version/
├── src/
│   ├── base/
│   ├── semantics/
│   ├── psychology/
│   ├── decision/
│   ├── runtime/
│   ├── validation/
│   ├── persistence/
│   └── diagnostics/
├── connectors/
│   ├── cpp/
│   └── c/
├── schemas/
│   ├── core-vocabulary/
│   ├── domain-pack/
│   └── conformance/
├── tests/
│   ├── unit/
│   ├── conformance/
│   ├── determinism/
│   ├── lifecycle/
│   ├── security/
│   └── performance/
├── scenarios/conformance/
├── tools/sandbox/
└── docs/
```

Los escenarios de conformidad son abstractos y no representan personajes, juegos ni mundos concretos.

## 6. `base`

Responsabilidad: tipos fundamentales sin lógica psicológica.

Incluye:
- IDs fuertes;
- `PsychTime`;
- `Result<T>` y `Error`;
- límites/cuotas;
- RNG determinista;
- orden canónico;
- tipos normalizados;
- versiones de schema/runtime/state.

### `PsychTime`

```text
uint64 logical ticks
```

La escala del tick la define el host al crear el runtime y no cambia durante la vida de ese runtime. Todos los timestamps contractuales usan el mismo dominio temporal.

### RNG

- stream independiente por agente;
- seed derivada de `SeedRoot + AgentId + stream_kind`;
- procesar, agregar o eliminar otro agente no altera secuencias existentes;
- las operaciones batch no comparten un cursor aleatorio mutable.

## 7. Política de errores

### API C++

Errores esperables usan:

```text
Result<T> = Value(T) | Error(ErrorCode, details)
```

Códigos mínimos:
- `InvalidArgument`
- `UnknownAgent`
- `DuplicateAgent`
- `InvalidState`
- `InvalidPack`
- `QuotaExceeded`
- `VersionMismatch`
- `NamespaceConflict`
- `TimeRegression`
- `BatchConflict`
- `UnsupportedOperation`
- `InternalInvariantViolation`

Las APIs públicas no usan excepciones como control de flujo normal y ninguna excepción cruza la frontera pública.

### Fachada C

La ABI C devuelve `mspa_status` y usa parámetros de salida explícitos. Los detalles de error se consultan mediante estructuras o buffers estables definidos por la ABI; nunca se exponen excepciones C++ ni punteros a memoria temporal.

## 8. `semantics`

Responsabilidad: vocabulario y lenguaje semántico portable.

### `VocabularyRegistry`

Contiene vocabulario base, namespaces externos, predicados, conceptos, tipos de acción y schemas.

### Registro por snapshots

El registro activo se maneja mediante **snapshots inmutables**:
- un batch captura un `RegistrySnapshot` al comenzar;
- ese snapshot permanece read-only hasta terminar;
- `IMPORT`, carga de packs o registro de símbolos nuevos no mutan el snapshot activo;
- las mutaciones se construyen en un staging registry;
- el staging se valida y se publica como snapshot nuevo entre batches;
- un batch en curso continúa usando el snapshot anterior;
- referencias desconocidas pueden preservarse como valores opacos namespaced sin exigir registro semántico inmediato.

### `PropositionModel`

Representa predicado, roles/argumentos, polaridad, tiempo, calificadores, namespace y procedencia.

### `PredicateSchemaRegistry`

Declara roles, agencia, consecuencias semánticas, relevancia, compatibilidad/contradicción, namespace y versión.

### `InferenceEngine`

Motor declarativo acotado:
- no Turing-completo;
- sin código arbitrario;
- sin recursión ilimitada;
- profundidad y presupuesto máximos;
- máximo de hipótesis;
- terminación garantizada bajo cuotas.

## 9. `psychology`

Estado privado por agente:

```text
AgentPsychState
  Personality
  Values
  Beliefs
  Emotions
  Mood
  Goals
  Memories
  Relationships
  LearnedExpectations
  CurrentIntent
  RngState
  LastProcessedTime
```

Submódulos:
- `Personality`
- `Values`
- `Beliefs`
- `Appraisal`
- `EmotionStore`
- `Mood`
- `Goals`
- `Memory`
- `Relationships`
- `Learning`

No dependen de clases del host.

## 10. Ciclo de vida de agentes

Las operaciones de ciclo de vida forman parte del runtime público.

### `CREATE_AGENT`

Entrada conceptual:

```text
AgentId
AgentInit
PsychTime
```

`AgentInit` puede contener estado completo o parcial, personalidad inicial, valores, relaciones, creencias o memorias iniciales autorizadas, seed/derivación de RNG y política de defaults.

MSPA sólo completa campos definidos por la política de inicialización. No inventa datos de dominio.

### Inicialización derivada

Puede existir un mecanismo genérico y determinista:

```text
DerivedInit
  source_agents[]
  trait_blend_policy
  variation_seed
  inherited_value_policy
```

No supone parentesco, nacimiento ni biología. El host decide cuándo y por qué usarlo.

### `REMOVE_AGENT`

Elimina el estado activo del `AgentStore`. Referencias desde otros estados pueden permanecer como `EntityRef` no resolubles.

Políticas mínimas:
- `ForgetRuntimeOnly`;
- `ValidateNoReferences`.

### `HAS_AGENT`

Consulta read-only de existencia y metadatos básicos.

### Importación como creación

`IMPORT_AGENT` valida colisiones de `AgentId` y sólo remapea cuando existe una política explícita. Nunca sobrescribe silenciosamente un agente existente.

## 11. `decision`

### `ActionOption`

Contiene `ActionTypeId`, `ActionInstanceId`, `ActionContinuityKey`, actor, objetivo(s), costes, riesgos, efectos esperados, tags/contexto y progreso opcional.

### `Considerations`

Sólo se evalúan las consideraciones registradas para esa acción.

### `ResponseCurves`

Curvas lineales por tramos como formato inicial de referencia.

### `UtilityComposer`

Combina contribuciones, condicionantes, histéresis, continuidad y ruido determinista pequeño.

### `Hysteresis`

La continuidad depende de `ActionContinuityKey`, progreso, coste de interrupción, meta y urgencia.

## 12. `runtime`

Operaciones públicas conceptuales:

### Ciclo de vida
- `CREATE_AGENT`
- `REMOVE_AGENT`
- `HAS_AGENT`

### Simulación
- `OBSERVE`
- `DECIDE`
- `REPORT`
- `ADVANCE_TO`

### Persistencia/traslado
- `EXPORT_AGENT`
- `IMPORT_AGENT`

### Administración semántica
- cargar/validar pack;
- construir staging registry;
- publicar nuevo registry snapshot entre batches.

`REACT` puede existir únicamente como wrapper de `OBSERVE + DECIDE`.

## 13. `OBSERVE`

Entrada: agente, estímulo, timestamp.

Salida:

```text
ObserveResult
  state_changed
  decision_recommended
  urgency
  current_action_threatened
  diagnostic_token?
```

`urgency` es enum contractual:
- `None`: no requiere reevaluación;
- `Normal`: puede esperar al scheduling normal;
- `Soon`: conviene reevaluar antes del ciclo normal;
- `Immediate`: el host debería reconsiderar la acción actual antes de continuarla si es posible.

MSPA recomienda; el host controla scheduling y ejecución.

## 14. `DECIDE`

Entrada: `AgentId`, timestamp, opciones disponibles y acción/continuidad actual opcional.

Salida: acción seleccionada, objetivo, confianza, intención, expresión observable, afirmaciones a transmitir y explicación causal.

MSPA nunca devuelve una acción que no estuviera disponible en la entrada.

## 15. `REPORT`

Comunica el resultado real de una acción y puede actualizar expectativas, aprendizaje, emociones, creencias, metas, relaciones y memoria.

## 16. `ADVANCE_TO`

No es un tick obligatorio. Sirve para materializar efectos perezosos, caducar metas, actualizar estado diferido y reactivar decisiones por paso del tiempo.

`target_time < LastProcessedTime` devuelve `TimeRegression` salvo que una API futura de replay/snapshot defina explícitamente otro comportamiento.

## 17. Batch

Toda operación simulacional con forma batch debe conservar equivalencia semántica con la ejecución individual canónica.

Antes de iniciar un batch se congelan:
- `RegistrySnapshot`;
- configuración;
- curvas;
- reglas;
- packs efectivos;
- límites efectivos.

Durante el batch sólo son mutables:
- estado privado de cada agente procesado;
- buffers locales de salida/diagnóstico;
- staging buffers no visibles para decisiones del mismo batch.

Nuevos símbolos, cambios de pack o cachés semánticas que alteren resultados se publican después del batch en un punto de sincronización definido.

El orden lógico se deriva de IDs estables, nunca de hash, dirección de memoria ni scheduling de threads.

## 18. `validation` y cuotas

Toda entrada externa se considera no confiable.

Jerarquía:
1. `HardLimits` compilados en MSPA;
2. `RuntimeLimits` configurados al crear el runtime;
3. recomendaciones externas.

Reglas:
- un pack nunca eleva `RuntimeLimits`;
- `RuntimeLimits` nunca excede `HardLimits`;
- si el host no configura un campo, se usa `DefaultLimits` versionado de MSPA;
- los defaults son conservadores;
- las cifras concretas se fijan mediante benchmarks y conformidad, no por contenido externo.

Se limitan como mínimo tamaño total de estado, creencias, memorias, metas, relaciones, reglas, profundidad de proposiciones, strings, patrones de aprendizaje y derivaciones por operación.

## 19. `persistence`

`AgentPsychState` es el modelo portable.

Requisitos:
- versionado explícito;
- sin punteros locales;
- namespaces;
- referencias externas preservables;
- validación previa al import;
- remapeo explícito de IDs cuando exista colisión;
- migraciones controladas.

El formato físico final no se fija en esta etapa.

## 20. `diagnostics`

Debe exponer opcionalmente estímulos procesados, cambios de creencias, inferencias, memorias activadas, emociones relevantes, opciones consideradas, contribuciones de utilidad, razones de decisión, errores y cuotas alcanzadas.

La instrumentación no modifica el resultado simulacional.

## 21. Conectores

Dentro de MSPA sólo existen conectores genéricos:
- API C++ pública;
- fachada C opcional.

Adaptadores para motores, frameworks o proyectos concretos viven fuera de MSPA.

## 22. Dependencias permitidas

```text
base
  ↑
semantics
  ↑
psychology
  ↑
decision
  ↑
runtime
```

Dependencias laterales:

```text
validation  -> base + semantics + public model
persistence -> base + public model
diagnostics -> public model + decision trace
connectors  -> public api only
```

No se permiten dependencias inversas hacia un consumidor.

## 23. Orden de implementación

1. `base`: IDs, tiempo, `Result`, errores, RNG, cuotas.
2. lifecycle mínimo: `Runtime`, `AgentStore`, `CREATE_AGENT`, `REMOVE_AGENT`.
3. semantics: proposiciones, registry snapshot, schemas.
4. simulación mínima: `OBSERVE`, `DECIDE`, `REPORT`, `ADVANCE_TO`.
5. beliefs + appraisal + emociones.
6. utility + histéresis.
7. memoria + relaciones + metas + ánimo.
8. inferencia declarativa + aprendizaje acotado.
9. export/import + validación + remapeo.
10. batch + paralelización opcional.
11. diagnostics.
12. fachada C.

## 24. Suite de conformidad previa a estabilidad

Casos abstractos, sin nombres ni contenido de hosts:
- crear/eliminar agentes;
- inicialización derivada determinista;
- divergencia de creencias ante evidencia equivalente;
- contradicción y reapreciación;
- continuidad/histéresis;
- `OBSERVE` con cada nivel de urgencia;
- batch vs individual;
- múltiples runtimes en el mismo proceso;
- import con símbolos desconocidos durante un batch;
- colisión/remapeo de IDs;
- rechazo de estados hostiles/cuotas;
- aprendizaje causal simple;
- determinismo repetido con misma seed y orden canónico.

## 25. Estado de decisiones técnicas

Fijado para la primera implementación:
- C++17;
- `PsychTime` entero de 64 bits;
- `Result<T>` para errores esperables;
- excepciones no atraviesan API pública;
- múltiples runtimes independientes;
- registry snapshots inmutables durante batch;
- publicación de mutaciones semánticas entre batches;
- RNG independiente por agente;
- curvas lineales por tramos como formato configurable inicial;
- ninguna dependencia de hosts concretos.

Aún no fijado:
- formato físico de serialización;
- sistema de build definitivo;
- modelo de threading interno;
- representación numérica final para garantías bit-a-bit multiplataforma;
- layout exacto de la ABI C.