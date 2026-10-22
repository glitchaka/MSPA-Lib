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

## 3. Modelo estructural e identidad

MSPA no modela jerarquías del mundo del host.

Tipos contractuales principales:
- `RuntimeId`
- `WorldId`
- `AgentId`
- `EntityRef`
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
- `TimeDomainId`
- `PsychTime`

Los IDs fuertes evitan mezclar accidentalmente dominios distintos.

### `AgentId`

`AgentId` incluye explícitamente el mundo de origen:

```text
AgentId
  WorldId world
  uint64 local_id
```

Dos agentes con el mismo `local_id` pero distinto `WorldId` son identidades diferentes. El remapeo durante importación nunca elimina silenciosamente el origen: si cambia la identidad efectiva, la operación devuelve el mapeo aplicado.

## 4. Múltiples runtimes en un proceso

Debe ser válido crear varias instancias `Runtime` simultáneas dentro del mismo proceso.

Cada runtime posee de forma aislada:

```text
Runtime
  RuntimeId
  RuntimeConfig
  TimeDomain
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
│   ├── base/
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
- `PsychTime`, `TimeScale` y `TimeDomain`;
- `Result<T>` y `Error`;
- límites/cuotas;
- RNG determinista;
- orden canónico;
- tipos normalizados;
- versiones de schema/runtime/state.

### 6.1 `PsychTime` y dominio temporal

```text
PsychTime = uint64 logical ticks
```

Cada runtime fija un `TimeDomain` al crearse. El dominio contiene una identidad y una escala racional, nunca un factor `float`.

```text
TimeDomain
  TimeDomainId id
  TimeScale scale

TimeScale
  uint64 ticks_per_second_num
  uint64 ticks_per_second_den
```

La escala no cambia durante la vida del runtime.

### 6.2 Tiempo de evento vs tiempo de procesamiento

Las operaciones simulacionales usan un **tiempo de procesamiento monotónico**. Un estímulo puede además llevar un `event_time` opcional que representa cuándo ocurrió el hecho conocido.

```text
StimulusTime
  PsychTime process_time
  optional<PsychTime> event_time
```

`process_time` no puede retroceder respecto al agente. `event_time` sí puede ser anterior: testimonios tardíos, mensajes de red, recuerdos o evidencia histórica no producen `TimeRegression` sólo por referirse al pasado.

Si el host recibe una marca temporal desde otro dominio, debe normalizarla o entregarla mediante la política de importación definida abajo.

### 6.3 RNG

- stream independiente por agente;
- seed derivada de `SeedRoot + AgentId + stream_kind`;
- procesar, agregar o eliminar otro agente no altera secuencias existentes;
- las operaciones batch no comparten un cursor aleatorio mutable.

## 7. Política de errores

### 7.1 API C++

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
- `OutOfMemory`
- `UnsupportedOperation`
- `InternalInvariantViolation`

Las APIs públicas no usan excepciones como control de flujo normal y ninguna excepción cruza la frontera pública.

La primera implementación se compila con excepciones C++ habilitadas internamente. Toda frontera pública captura `std::bad_alloc` y lo convierte en `OutOfMemory`. Un build futuro sin excepciones sólo será válido cuando exista una política de asignación que conserve esa garantía sin terminar el proceso.

### 7.2 Fachada C

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
  AgentId
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

Entrada conceptual:

```text
OBSERVE(runtime, agent, stimulus, process_time)
```

El `Stimulus` puede incluir `event_time` opcional. Sólo `process_time` participa en la regla de monotonía del runtime.

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

Entrada: `AgentId`, `process_time`, opciones disponibles y acción/continuidad actual opcional.

Salida: acción seleccionada, objetivo, confianza, intención, expresión observable, afirmaciones a transmitir y explicación causal.

MSPA nunca devuelve una acción que no estuviera disponible en la entrada.

## 15. `REPORT`

Comunica el resultado real de una acción y puede actualizar expectativas, aprendizaje, emociones, creencias, metas, relaciones y memoria.

## 16. `ADVANCE_TO`

No es un tick obligatorio. Sirve para materializar efectos perezosos, caducar metas, actualizar estado diferido y reactivar decisiones por paso del tiempo.

`target_time < LastProcessedTime` devuelve `TimeRegression` salvo que una API futura de replay/snapshot defina explícitamente otro comportamiento.

## 17. Batch

Toda operación simulacional con forma batch debe conservar equivalencia semántica con la ejecución individual canónica.

Una llamada individual se define semánticamente como **un batch de una operación**, de modo que captura los mismos snapshots de registry/configuración que la forma batch.

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

### Orden canónico

Cada entrada de batch recibe un `input_sequence` igual a su posición estable en la entrada del host.

Orden lógico:

```text
(AgentId, input_sequence)
```

Así dos operaciones del mismo agente conservan el orden explícito del host sin depender del scheduling de threads.

`BatchConflict` se produce, como mínimo, cuando:
- el mismo agente recibe dos operaciones con el mismo `input_sequence`;
- las operaciones de un agente exigen tiempos de procesamiento incompatibles con ese orden;
- se mezcla una mutación de lifecycle con una operación simulacional del mismo agente de forma que el orden no pueda resolverse por la secuencia proporcionada.

Un conflicto nunca se resuelve mediante orden de hash, dirección de memoria o thread scheduling.

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

## 19. `persistence` e importación temporal

`AgentPsychState` es el modelo portable.

Requisitos:
- versionado explícito;
- sin punteros locales;
- namespaces;
- referencias externas preservables;
- validación previa al import;
- remapeo explícito de IDs cuando exista colisión;
- migraciones controladas;
- metadatos de dominio temporal de origen;
- `exported_at` del runtime de origen.

### Política de tiempo al importar

La política por defecto es `RebaseRelative`:
- las edades/duraciones de timestamps se calculan respecto de `exported_at`;
- esas duraciones se convierten racionalmente a la escala del runtime destino;
- se vuelven a anclar en `import_time`;
- la conversión usa enteros con detección de overflow y una política de redondeo definida.

También pueden existir:
- `PreserveAbsolute`, sólo si origen y destino declaran dominios compatibles;
- `RejectIncompatible`, para hosts que no quieran conversión.

Nunca se reinterpretan ticks de un dominio como si pertenecieran a otro.

### Política de RNG al importar

`AgentPsychState` conserva `RngState`, pero la política depende del propósito:
- **por defecto en traslado a otro runtime/mundo:** `RekeyForDestination`; se deriva un nuevo stream desde el `SeedRoot` destino + `AgentId` efectivo + `stream_kind`, evitando secuencias correlacionadas entre mundos;
- **restore/replay del mismo contexto lógico:** `PreserveState`, permitido sólo cuando el host lo solicita explícitamente.

La política aplicada queda registrada en el resultado de importación.

El formato físico final de serialización no se fija en esta etapa.

## 20. `diagnostics`

Debe existir desde la simulación mínima, no como añadido tardío.

Nivel mínimo obligatorio para depuración:
- operación procesada;
- `AgentId`;
- tiempos de evento/procesamiento cuando existan;
- cambios de creencias;
- appraisal resumido;
- emociones relevantes;
- opciones consideradas;
- contribuciones principales de utilidad;
- razón compacta de decisión;
- errores y cuotas alcanzadas.

Una traza extendida puede añadirse después. La instrumentación no modifica el resultado simulacional.

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

### Etapa 0 — conformidad y trazas mínimas

Antes de desarrollar comportamiento complejo se fija:
- formato neutral de escenarios de prueba;
- runner mínimo de conformidad;
- traza compacta de `OBSERVE` / `DECIDE`;
- criterios de determinismo y comparación.

### Etapa 1 — `base`

IDs, `AgentId` compuesto, tiempo/dominios temporales, `Result`, errores, RNG, cuotas.

### Etapa 2 — lifecycle y runtime mínimo

`Runtime`, `AgentStore`, `CREATE_AGENT`, `REMOVE_AGENT`, `HAS_AGENT` y diagnostics mínimos.

### Etapa 3 — semantics y simulación mínima

Proposiciones, registry snapshot, schemas, `OBSERVE`, `DECIDE`, `REPORT`, `ADVANCE_TO`.

### Etapa 4 — conocimiento y afecto

Beliefs, appraisal, emociones y actualización por evidencia.

### Etapa 5 — decisión

Considerations, curvas, utilidad e histéresis, siempre con traza compacta activa.

### Etapa 6 — continuidad psicológica

Memoria, relaciones, metas, ánimo y reapreciación.

### Etapa 7 — inferencia y aprendizaje

Inferencia declarativa y aprendizaje causal acotado.

### Etapa 8 — portabilidad

Export/import, conversión temporal, políticas RNG, validación y remapeo.

### Etapa 9 — batch y rendimiento

API batch, orden secundario, paralelización opcional y equivalencia.

### Etapa 10 — fachada C

ABI C y bindings externos.

## 24. Suite de conformidad previa a estabilidad

Casos abstractos, sin nombres ni contenido de hosts:
- IDs fuertes y aislamiento por `WorldId`;
- crear/eliminar agentes;
- inicialización derivada determinista;
- divergencia de creencias ante evidencia equivalente;
- contradicción y reapreciación;
- estímulo tardío con `event_time < process_time`;
- conversión/rebase de timestamps entre dominios;
- continuidad/histéresis;
- `OBSERVE` con cada nivel de urgencia;
- batch vs individual;
- dos operaciones del mismo agente conservando `input_sequence`;
- detección de `BatchConflict`;
- múltiples runtimes en el mismo proceso;
- import con símbolos desconocidos durante un batch;
- colisión/remapeo de IDs;
- `RekeyForDestination` y `PreserveState` del RNG;
- conversión de `std::bad_alloc` a `OutOfMemory` en frontera pública;
- rechazo de estados hostiles/cuotas;
- aprendizaje causal simple;
- determinismo repetido con misma seed y orden canónico.

## 25. Estado de decisiones técnicas

Fijado para la primera implementación:
- C++17;
- `PsychTime` entero de 64 bits;
- escala temporal racional por runtime;
- `AgentId = WorldId + local_id`;
- `process_time` separado de `event_time`;
- import temporal con `RebaseRelative` por defecto;
- `Result<T>` para errores esperables;
- `std::bad_alloc` convertido a `OutOfMemory` en fronteras públicas;
- excepciones no atraviesan API pública;
- múltiples runtimes independientes;
- registry snapshots inmutables durante batch;
- publicación de mutaciones semánticas entre batches;
- orden batch `(AgentId, input_sequence)`;
- llamada individual equivalente a batch de uno;
- RNG independiente por agente;
- `RekeyForDestination` como política RNG por defecto al trasladar agentes;
- curvas lineales por tramos como formato configurable inicial;
- diagnostics mínimos desde el runtime inicial;
- ninguna dependencia de hosts concretos.

Aún no fijado:
- formato físico de serialización;
- sistema de build definitivo más allá del soporte CMake inicial;
- modelo de threading interno;
- representación numérica final para garantías bit-a-bit multiplataforma;
- layout exacto de la ABI C.
