# MSPA-Lib — Arquitectura de código v0.2

> Estado: **plan de arquitectura de implementación**. Documento completamente agnóstico al host.

Esta versión reemplaza v0.1 como arquitectura de código vigente. Cierra explícitamente ciclo de vida de agentes, mutabilidad de registros durante batch, errores, múltiples runtimes, estándar de C++ y política básica de compilación determinista.

---

## 1. Invariante de separación

MSPA no incorpora nombres, clases, personajes, escenarios, repositorios, namespaces ni reglas pertenecientes a un consumidor concreto.

El core sólo conoce contratos genéricos. Los proyectos consumidores realizan externamente el mapeo entre sus entidades/eventos/acciones y MSPA.

> **MSPA posee estado psicológico y lógica de interpretación/decisión. El host posee el mundo, las entidades reales, la percepción física, las acciones disponibles y su ejecución.**

---

## 2. Lenguaje y estándar

### 2.1 Núcleo

- lenguaje: **C++17**;
- sin dependencia obligatoria de motor;
- biblioteca construible como estática o dinámica;
- fachada C opcional como ABI para otros lenguajes;
- ninguna excepción C++ cruza la API pública ni la fachada C.

C++17 se fija para maximizar compatibilidad con toolchains de motores y hosts nativos. MSPA implementará su propio tipo `Result<T>` en lugar de depender de `std::expected`.

### 2.2 Política numérica y compilación

Build determinista recomendado:

- MSVC: `/fp:strict`;
- GCC/Clang: sin `-ffast-math`; desactivar contracción FMA cuando pueda alterar conformidad (`-ffp-contract=off` o equivalente disponible);
- no depender de orden de evaluación no especificado;
- no usar funciones trascendentales en curvas configurables cuando exista una representación determinista por tabla/segmentos;
- `PsychTime` usa entero de 64 bits.

Estas banderas reducen divergencias, pero MSPA no promete igualdad bit-a-bit entre toolchains distintos hasta validar una política numérica de conformidad. La lógica que deba ser estrictamente reproducible no puede depender de comportamiento de coma flotante no especificado.

---

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

---

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
- un runtime no puede observar ni modificar otro salvo que el host exporte/importa datos explícitamente;
- cachés compartidas entre runtimes sólo pueden compartirse si son inmutables desde la perspectiva de la simulación;
- la destrucción de un runtime libera exclusivamente sus recursos propios.

Esto permite ejecutar mundos, sesiones, pruebas o simulaciones independientes en un mismo proceso.

---

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

---

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

### 6.1 `PsychTime`

```text
uint64 logical ticks
```

La escala del tick la define el host al crear el runtime y no cambia durante la vida de ese runtime.

Todos los timestamps contractuales usan el mismo dominio temporal del runtime.

### 6.2 RNG

- stream independiente por agente;
- seed derivada de `SeedRoot + AgentId + stream_kind`;
- procesar/agregar/eliminar otro agente no altera secuencias existentes;
- las operaciones batch no comparten un cursor aleatorio mutable.

---

## 7. Política de errores

### 7.1 API C++

Errores esperables usan `Result<T>`:

```text
Result<T> = Value(T) | Error(ErrorCode, details)
```

Categorías mínimas:

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

Reglas:

- las APIs públicas no usan excepciones para control de flujo normal;
- ninguna excepción cruza la frontera pública;
- fallos de validación siempre se devuelven como `Result`;
- invariantes imposibles pueden activar `assert` en debug y producir `InternalInvariantViolation` en builds de producción cuando sea recuperable.

### 7.2 Fachada C

La ABI C usa:

```text
mspa_status
```

más parámetros de salida explícitos.

Debe existir una forma de obtener detalles de error sin devolver punteros a memoria temporal. La fachada C nunca expone excepciones C++.

---

## 8. `semantics`

Responsabilidad: vocabulario y lenguaje semántico portable.

### 8.1 `VocabularyRegistry`

Contiene vocabulario base, namespaces externos, predicados, conceptos, tipos de acción y schemas.

### 8.2 Regla de mutación

El registro activo se maneja por **snapshots inmutables**.

- un batch captura un `RegistrySnapshot` al comenzar;
- ese snapshot es read-only hasta terminar el batch;
- `IMPORT`, carga de packs o registro de símbolos nuevos no mutan el snapshot activo;
- las mutaciones semánticas se construyen en un staging registry;
- el staging se valida y se publica atómicamente como un snapshot nuevo **entre batches**;
- un batch en curso continúa usando su snapshot anterior;
- referencias desconocidas pueden conservarse como valores opacos namespaced sin exigir registrar semántica durante el batch.

Así `IMPORT` puede descubrir nuevos conceptos sin romper equivalencia batch/individual ni introducir carreras.

### 8.3 `PropositionModel`

Representa predicado, roles/argumentos, polaridad, tiempo, calificadores, namespace y procedencia.

### 8.4 `PredicateSchemaRegistry`

Declara roles, agencia, consecuencias semánticas, relevancia, compatibilidad/contradicción, namespace y versión.

### 8.5 `InferenceEngine`

- declarativo;
- no Turing-completo;
- sin código arbitrario;
- sin recursión ilimitada;
- profundidad y presupuesto máximos;
- máximo de hipótesis;
- terminación garantizada bajo cuotas.

---

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

---

## 10. Ciclo de vida de agentes

Las operaciones de ciclo de vida forman parte del runtime público.

### 10.1 `CREATE_AGENT`

Entrada conceptual:

```text
AgentId
AgentInit
PsychTime
```

`AgentInit` puede contener:

- estado explícito completo o parcial;
- personalidad inicial;
- valores iniciales;
- relaciones iniciales;
- creencias/memorias iniciales permitidas;
- seed/derivación de RNG;
- política de defaults.

MSPA rellena únicamente campos definidos por la política de inicialización. No inventa datos de dominio.

### 10.2 Inicialización derivada

MSPA puede ofrecer un mecanismo **genérico** de inicialización derivada a partir de uno o más `AgentPsychState` fuente:

```text
DerivedInit
  source_agents[]
  trait_blend_policy
  variation_seed
  inherited_value_policy
```

No supone parentesco, nacimiento ni biología. El host decide cuándo y por qué utiliza esta operación.

La política debe ser declarativa/configurable y determinista.

### 10.3 `REMOVE_AGENT`

Elimina el estado activo del `AgentStore`.

Opciones de política:

- `ForgetRuntimeOnly`: retira el agente del runtime sin reescribir estados de terceros;
- `ValidateNoReferences`: rechaza la eliminación si la política del host exige referencias resolubles;
- referencias desde recuerdos/creencias de otros agentes pueden permanecer como `EntityRef` no resoluble.

### 10.4 `HAS_AGENT` / consulta

Consulta sin mutación para comprobar registro y metadatos básicos.

### 10.5 Importación como creación

`IMPORT_AGENT` crea un agente únicamente después de validar que `AgentId` no colisiona o después de aplicar una política explícita de remapeo. Nunca sobrescribe silenciosamente un agente existente.

---

## 11. `decision`

### `ActionOption`

Contiene:

- `ActionTypeId`;
- `ActionInstanceId`;
- `ActionContinuityKey`;
- actor;
- objetivo(s);
- costes;
- riesgos;
- efectos esperados;
- tags/contexto;
- progreso opcional.

### `Considerations`

Sólo se evalúan consideraciones registradas para esa acción.

### `ResponseCurves`

Curvas lineales por tramos como formato inicial de referencia.

### `UtilityComposer`

Combina contribuciones, condicionantes, histéresis, continuidad y ruido determinista pequeño.

### `Hysteresis`

La continuidad depende de `ActionContinuityKey`, progreso, coste de interrupción, meta y urgencia.

---

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

---

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

---

## 14. `DECIDE`

Entrada:

- `AgentId`;
- timestamp;
- opciones disponibles;
- acción/continuidad actual opcional.

Salida:

- acción seleccionada;
- objetivo;
- confianza;
- intención;
- expresión observable;
- afirmaciones a transmitir;
- explicación causal.

MSPA nunca devuelve una acción que no estuviera disponible en la entrada.

---

## 15. `REPORT`

Comunica resultado real de una acción.

Puede actualizar:

- expectativas;
- aprendizaje;
- emociones;
- creencias;
- metas;
- relaciones;
- memoria.

---

## 16. `ADVANCE_TO`

No es un tick obligatorio.

Sirve para:

- materializar efectos perezosos;
- caducar metas;
- actualizar estado diferido;
- reactivar decisiones por paso del tiempo.

`target_time < LastProcessedTime` devuelve `TimeRegression` salvo que una API futura de replay/snapshot defina explícitamente otro comportamiento.

---

## 17. Batch

Toda operación simulacional que tenga forma batch debe conservar equivalencia semántica con ejecución individual canónica.

### 17.1 Snapshot rule

Antes de iniciar un batch se congelan:

- `RegistrySnapshot`;
- configuración;
- curvas;
- reglas;
- packs efectivos;
- límites efectivos.

No cambian durante el batch.

### 17.2 Estado mutable permitido

Sólo:

- estado privado de cada agente procesado;
- buffers locales de salida/diagnóstico;
- staging buffers que no sean visibles para decisiones del mismo batch.

### 17.3 Publicación diferida

Nuevos símbolos, cambios de pack o cachés semánticas que alteren resultados se publican después del batch en un punto de sincronización definido.

### 17.4 Orden canónico

El orden lógico se deriva de IDs estables, nunca de orden de hash, dirección de memoria ni scheduling de threads.

---

## 18. `validation` y cuotas

Toda entrada externa se considera no confiable.

Jerarquía:

1. `HardLimits` compilados en MSPA;
2. `RuntimeLimits` configurados al crear el runtime;
3. recomendaciones de packs externos.

Reglas:

- un pack nunca eleva `RuntimeLimits`;
- `RuntimeLimits` nunca excede `HardLimits`;
- si el host no configura un campo, se usa `DefaultLimits` de MSPA;
- los defaults son conservadores y versionados;
- las cifras concretas se fijan mediante benchmarks y suite de conformidad, no por un pack externo.

Se limitan, como mínimo:

- tamaño total de estado;
- creencias;
- memorias;
- metas;
- relaciones;
- reglas;
- profundidad de proposiciones;
- longitud de strings;
- patrones de aprendizaje;
- derivaciones por operación.

---

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

---

## 20. `diagnostics`

Debe exponer de forma opcional:

- estímulos procesados;
- cambios de creencias;
- inferencias;
- memorias activadas;
- emociones relevantes;
- opciones consideradas;
- contribuciones de utilidad;
- razones de decisión;
- errores y cuotas alcanzadas.

La instrumentación no modifica el resultado simulacional.

---

## 21. Conectores

Dentro de MSPA sólo existen conectores genéricos:

- API C++ pública;
- fachada C opcional.

Adaptadores para motores, frameworks o proyectos concretos viven fuera de MSPA.

---

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

---

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

---

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

---

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
