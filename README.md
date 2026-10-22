# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una biblioteca C++17, agnóstica al motor y al juego, para dotar agentes autónomos de personalidad, valores, metas, creencias subjetivas, emociones, ánimo, memoria, relaciones, aprendizaje y selección de acciones explicable.

> **Este README es autocontenido.** Contiene el contrato técnico necesario para implementar y continuar MSPA sin depender de leer los archivos de `docs/`. Los documentos separados se conservan como copias normativas y material de referencia, no como capítulos obligatorios para reconstruir la arquitectura.

## Estado actual

La arquitectura lógica, el contrato de interacción, el contrato de Domain Packs y la arquitectura C++ ya están definidos. Existe un esqueleto C++17 compilable con implementación funcional mínima de los contratos base, runtime, lifecycle, registro semántico, observación, decisión, persistencia, validación, diagnostics y subsistemas psicológicos separados.

La lógica psicológica avanzada todavía es deliberadamente provisional: las fórmulas de appraisal, emociones, utilidad, histéresis, aprendizaje e inferencia deben desarrollarse y validarse mediante escenarios de conformidad. **La arquitectura y las fronteras entre módulos no deben rediseñarse durante esa implementación salvo incompatibilidad demostrable.**

---

# 1. Frontera fundamental

```text
HOST / WORLD
    ↓ OBSERVE
MSPA actualiza estado subjetivo
    ↓ DECIDE
MSPA elige exclusivamente entre opciones ofrecidas
    ↓
HOST ejecuta la acción
    ↓ REPORT
MSPA aprende y actualiza estado
```

El host conserva autoridad sobre realidad y ejecución. MSPA conserva autoridad únicamente sobre estado psicológico e interpretación subjetiva.

### El host posee

- mundo y entidades reales;
- percepción física;
- pathfinding, navegación, animación y física;
- inventario, recursos y combate;
- acciones físicamente disponibles;
- resultados reales de las acciones;
- scheduling de agentes;
- persistencia física y transporte de bytes;
- semántica específica del proyecto consumidor.

### MSPA posee

- personalidad;
- valores;
- creencias;
- appraisal;
- emociones y ánimo;
- metas;
- memoria;
- relaciones direccionales;
- expectativas aprendidas;
- inferencia declarativa;
- preferencia entre acciones ofrecidas;
- intención y expresión;
- explicación causal de la decisión.

**MSPA no contiene clases, personajes, nombres, eventos, reglas, repositorios ni conceptos pertenecientes a un host concreto.**

---

# 2. Build y pruebas

Requisitos mínimos:

- CMake 3.20+
- compilador C++17
- MSVC, GCC o Clang

Configuración y build:

```bash
cmake -S . -B build -DMSPA_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Política numérica de conformidad:

- MSVC: `/fp:strict`
- GCC/Clang: sin `-ffast-math`
- GCC/Clang: `-ffp-contract=off` cuando corresponda
- no depender de orden de evaluación no especificado
- evitar funciones trascendentales cuando una curva determinista por segmentos pueda sustituirlas

No se promete todavía igualdad bit-a-bit entre toolchains diferentes.

---

# 3. Árbol de código

```text
MSPA-Lib/
├── CMakeLists.txt
├── README.md
├── include/mspa/
│   ├── mspa.hpp
│   ├── base/
│   │   ├── error.hpp
│   │   ├── ids.hpp
│   │   ├── result.hpp
│   │   ├── rng.hpp
│   │   └── time.hpp
│   ├── model/
│   │   └── agent_state.hpp
│   ├── semantics/
│   │   └── registry.hpp
│   ├── psychology/
│   │   ├── appraisal.hpp
│   │   ├── beliefs.hpp
│   │   ├── emotions.hpp
│   │   ├── engine.hpp
│   │   ├── goals.hpp
│   │   ├── learning.hpp
│   │   ├── memory.hpp
│   │   └── relationships.hpp
│   ├── decision/
│   │   ├── decision.hpp
│   │   └── hysteresis.hpp
│   ├── runtime/
│   │   ├── agent_store.hpp
│   │   └── runtime.hpp
│   ├── persistence/
│   │   └── state_transfer.hpp
│   ├── validation/
│   │   └── limits.hpp
│   └── diagnostics/
│       └── trace.hpp
├── src/
│   ├── base/
│   │   ├── rng.cpp
│   │   └── time.cpp
│   ├── semantics/
│   │   └── registry.cpp
│   ├── psychology/
│   │   ├── appraisal.cpp
│   │   ├── beliefs.cpp
│   │   ├── emotions.cpp
│   │   ├── engine.cpp
│   │   ├── goals.cpp
│   │   ├── learning.cpp
│   │   ├── memory.cpp
│   │   └── relationships.cpp
│   ├── decision/
│   │   ├── decision.cpp
│   │   └── hysteresis.cpp
│   ├── runtime/
│   │   ├── agent_store.cpp
│   │   └── runtime.cpp
│   ├── persistence/
│   │   └── state_transfer.cpp
│   ├── validation/
│   │   └── limits.cpp
│   └── diagnostics/
│       └── trace.cpp
├── tests/
│   ├── base_tests.cpp
│   └── runtime_tests.cpp
└── docs/
```

`include/mspa/mspa.hpp` es el header público agregador.

---

# 4. Dependencias de módulos

Dependencia principal permitida:

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
validation  -> base + model
persistence -> base + model
 diagnostics -> model + decision trace
```

No se permiten dependencias inversas desde el core hacia proyectos consumidores.

---

# 5. Identidad

Los IDs son tipos fuertes y no deben intercambiarse implícitamente.

Tipos principales:

- `RuntimeId`
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
- `TimeDomainId`

`AgentId` incluye mundo de origen:

```text
AgentId
  WorldId world
  uint64 local_id
```

El mismo `local_id` en dos mundos distintos representa agentes distintos.

Los remapeos de identidad durante importación son explícitos y deben devolverse al host. Nunca se sobrescribe silenciosamente un agente existente.

---

# 6. Runtime

Debe ser legal crear varios runtimes simultáneos dentro del mismo proceso.

Cada `Runtime` posee aisladamente:

```text
Runtime
  RuntimeConfig
  TimeDomain
  RegistrySnapshot
  AgentStore
  SeedRoot
  PsychologyEngine
  UtilityComposer
  Diagnostics
  RuntimeLimits
```

No existe:

- RNG global;
- `AgentStore` global;
- registry mutable global;
- estado mutable obligatorio compartido entre runtimes.

Cachés compartidas sólo pueden existir si son inmutables desde la perspectiva de la simulación.

---

# 7. Ciclo de vida de agentes

API conceptual:

```text
CREATE_AGENT
REMOVE_AGENT
HAS_AGENT
```

### CREATE_AGENT

Entrada mínima:

```text
AgentId
AgentInit
PsychTime process_time
```

`AgentInit` puede incluir personalidad, valores y más adelante estado inicial explícito o políticas derivadas.

La inicialización derivada se representa conceptualmente como:

```text
DerivedInit
  source_agents[]
  trait_blend_policy
  variation_seed
  inherited_value_policy
```

No implica biología ni parentesco. El host decide cuándo usarla.

### REMOVE_AGENT

Retira el estado activo del runtime. Referencias históricas desde memorias o creencias de terceros pueden permanecer como referencias no resolubles.

### HAS_AGENT

Consulta sin mutación y sin consumo de RNG.

---

# 8. Estado psicológico

`AgentPsychState` reúne:

```text
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

## Personalidad

Seis ejes base normalizados:

- sociabilidad: reservado ↔ sociable
- estabilidad: reactivo ↔ sereno
- empatía: egocéntrico ↔ empático
- persistencia: flexible ↔ obstinado
- curiosidad: tradicional ↔ curioso
- audacia: prudente ↔ temerario

La personalidad modifica sensibilidades, pesos y curvas. **No debe convertirse en bonificaciones rígidas directas a acciones.**

## Valores

Un valor es `ConceptId + intensidad`. Los catálogos concretos pertenecen a datos externos o Domain Packs.

## Relaciones

Son direccionales:

- afecto
- confianza
- respeto
- familiaridad

`A → B` y `B → A` son estados independientes.

---

# 9. Semántica y proposiciones

Las creencias no se almacenan como texto libre sino como proposiciones estructuradas.

Una `Proposition` contiene como mínimo:

- `PredicateId`
- argumentos por rol
- polaridad
- namespace
- posteriormente tiempo, calificadores y procedencia completos

Un `PredicateSchema` declara:

- roles requeridos y opcionales
- agencia
- categorías de consecuencia
- relevancia
- intencionalidad
- tags de aprendizaje
- pistas de appraisal
- mapeos al vocabulario base

Un schema **no fija una emoción universal**.

`RegistrySnapshot` es inmutable durante una operación batch. `RegistryBuilder` crea un staging mutable y publica un snapshot nuevo entre batches.

Símbolos desconocidos pueden preservarse de forma opaca sin modificar el snapshot activo.

---

# 10. OBSERVE

Entrada conceptual:

```text
OBSERVE(runtime, agent, stimulus, process_time)
```

`Stimulus` puede representar:

- observación física
- testimonio
- resultado observado
- estado interno
- cambio ambiental
- comunicación
- influencia externa
- recordatorio contextual

Salida mínima:

```text
ObserveResult
  state_changed
  decision_recommended
  urgency
  current_action_threatened
```

`urgency`:

- `None`
- `Normal`
- `Soon`
- `Immediate`

MSPA recomienda scheduling; el host conserva el control.

---

# 11. Tiempo

`PsychTime` es un entero de 64 bits.

```text
PsychTime = uint64 logical ticks
```

Cada runtime fija un dominio temporal:

```text
TimeDomain
  TimeDomainId id
  TimeScale scale

TimeScale
  uint64 ticks_per_second_num
  uint64 ticks_per_second_den
```

La escala es racional y no cambia durante la vida del runtime.

## process_time vs event_time

`process_time` es el tiempo monotónico en que MSPA procesa una operación.

`event_time` es cuándo ocurrió el hecho referido por un estímulo.

Un testimonio tardío, mensaje de red retrasado o recuerdo puede tener:

```text
event_time < LastProcessedTime
```

sin producir `TimeRegression`.

En cambio:

```text
process_time < LastProcessedTime
```

sí produce `TimeRegression`, salvo un mecanismo futuro explícito de replay/snapshot.

---

# 12. Creencias

Cada creencia mantiene al menos:

- `BeliefId`
- predicado/proposición
- certeza
- carga emocional
- timestamp de actualización

El diseño completo contempla además:

- fuente
- origen
- evidencia resumida
- dependencias de inferencia

Fuentes conceptuales:

- observación
- testimonio
- inferencia
- recuerdo
- influencia externa
- aprendizaje

**El agente reacciona a sus creencias, no a la verdad global.**

La actualización futura debe considerar certeza previa, carga emocional, confianza en la fuente, claridad perceptiva, rigidez, curiosidad, emociones actuales, coherencia e independencia de fuentes.

---

# 13. Inferencia

La inferencia produce hipótesis, no verdades.

Las reglas deben ser:

- declarativas
- serializables
- versionadas
- no Turing-completas
- sin código arbitrario
- sin acceso a red/filesystem
- limitadas por profundidad, reglas evaluadas, hipótesis y presupuesto lógico

La terminación debe estar garantizada bajo cuotas.

---

# 14. Appraisal y emociones

El appraisal considera conceptualmente:

- relevancia
- consecuencia
- agencia
- intención percibida
- certeza
- control
- norma
- relación
- expectativa previa

La novedad deriva preferentemente del error de predicción.

Emociones base candidatas:

- alegría
- tristeza
- miedo
- ira
- ansiedad
- aversión

Estados como orgullo, culpa, gratitud, duelo, resentimiento, alivio o esperanza deben derivarse del estado base, no convertirse automáticamente en barras independientes.

El ánimo mantiene:

- valencia
- activación

La reapreciación puede cambiar la carga emocional de una memoria sin alterar su contenido factual.

---

# 15. Metas

Las metas pueden derivar de:

- necesidades
- valores
- compromisos
- eventos
- creencias
- relaciones
- regulación emocional
- influencia externa

Estados conceptuales:

- activa
- suspendida
- bloqueada
- completada
- abandonada

Sólo unas pocas metas deben dominar simultáneamente; el resto permanece latente o suspendido.

---

# 16. Memoria

Una memoria completa debe poder conservar:

- proposición/evento
- interpretación
- emoción original
- importancia
- fuente
- certeza
- participantes
- timestamp
- reactivaciones
- referencias externas

La consolidación es selectiva y está limitada por cuotas.

---

# 17. Aprendizaje

La primera versión de aprendizaje causal está deliberadamente acotada:

```text
ActionType + ContextTags -> OutcomeTags
```

Cada patrón puede conservar:

- intentos
- éxitos
- frecuencia de resultados
- confianza
- última activación

No se intenta aprendizaje simbólico abierto en la primera implementación.

Curiosidad modula exploración vs explotación.

El aprendizaje social podrá ponderar observación, consejo, testimonio e imitación mediante confianza, respeto, familiaridad, prestigio, claridad y compatibilidad de creencias.

---

# 18. DECIDE

El host entrega únicamente acciones físicamente posibles.

`ActionOption` contiene al menos:

- `ActionTypeId`
- `ActionInstanceId`
- `ActionContinuityKey`
- actor
- objetivos
- costes
- riesgos
- progreso
- tags/contexto

`Decision` puede devolver:

- acción seleccionada
- objetivo
- confianza
- intención
- emoción expresada
- afirmaciones a transmitir
- traza de candidatos

MSPA **nunca puede seleccionar una acción que no esté en la entrada**.

## Utility AI

Las consideraciones posibles incluyen:

- necesidad
- meta
- valor
- emoción
- relación
- memoria
- norma
- contexto
- coste
- riesgo
- expectativa aprendida

No todas las consideraciones se calculan para todas las acciones.

Las curvas configurables deben ser serializables y deterministas; el formato inicial preferido son curvas lineales por tramos.

---

# 19. Histéresis

`ActionContinuityKey` permanece estable mientras una opción represente la misma actividad lógica.

La continuidad puede depender de:

- progreso
- coste de interrupción
- persistencia de meta
- compromiso
- contexto
- urgencia nueva

Una alternativa debe superar suficientemente a la acción actual salvo urgencia crítica.

---

# 20. REPORT

Entrada conceptual:

```text
REPORT(runtime, agent, action_result, process_time)
```

El resultado real puede actualizar:

- aprendizaje
- expectativas
- emociones
- creencias
- relaciones
- metas
- memoria

El host informa lo que realmente ocurrió; MSPA no simula el resultado físico por su cuenta.

---

# 21. ADVANCE_TO

No existe obligación de ejecutar un tick psicológico por frame.

`ADVANCE_TO` sirve para:

- materializar decaimiento perezoso
- caducar metas
- actualizar estado diferido
- detectar que el paso del tiempo recomienda una nueva decisión

`target_time < LastProcessedTime` produce `TimeRegression`.

---

# 22. Batch y determinismo

Toda operación individual se define semánticamente como un batch de una operación.

Al comenzar un batch se congelan:

- registry
- configuración
- curvas
- reglas
- packs efectivos
- límites efectivos

Durante el batch sólo pueden mutar:

- estado privado del agente procesado
- buffers locales de salida/diagnóstico
- staging no visible para decisiones del mismo batch

Orden canónico:

```text
(AgentId, input_sequence)
```

`input_sequence` conserva el orden explícito proporcionado por el host para varias operaciones del mismo agente.

`BatchConflict` ocurre como mínimo cuando:

- dos operaciones del mismo agente comparten `input_sequence`
- el orden implica tiempos de procesamiento incompatibles
- lifecycle y simulación sobre el mismo agente no poseen un orden resoluble

Nunca se usa como desempate:

- orden de hash
- dirección de memoria
- scheduling de threads

---

# 23. RNG

No existe RNG global compartido.

Cada agente utiliza un stream derivado de:

```text
SeedRoot + AgentId + stream_kind
```

Procesar, crear o eliminar otro agente no debe alterar su secuencia.

`AgentPsychState` conserva `RngState`.

Al trasladar entre mundos/runtimes:

```text
RngImportPolicy::RekeyForDestination
```

es el comportamiento por defecto.

Para restore/replay del mismo contexto lógico puede pedirse explícitamente:

```text
RngImportPolicy::PreserveState
```

---

# 24. Export / Import

`EXPORT_AGENT` devuelve estado portable y versionado junto con:

- identidad original
- dominio temporal de origen
- `exported_at`
- `RngState`

`IMPORT_AGENT` valida:

1. versión
2. estructura
3. namespaces/packs
4. cuotas
5. referencias
6. colisiones de identidad
7. remapeo
8. dominio temporal
9. política de RNG

## Política temporal

Por defecto:

```text
ImportTimePolicy::RebaseRelative
```

Las edades relativas a `exported_at` se convierten a la escala destino y se reanclan en `import_time`.

Opciones:

- `RebaseRelative`
- `PreserveAbsolute`
- `RejectIncompatible`

Nunca se reinterpretan ticks de un dominio como ticks de otro.

---

# 25. Errores

La API C++ utiliza:

```text
Result<T>
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

Los errores esperables no usan excepciones como control de flujo.

La primera implementación puede usar excepciones internamente, pero ninguna excepción cruza una frontera pública. `std::bad_alloc` debe convertirse a `OutOfMemory`.

La futura C ABI devolverá `mspa_status` y parámetros de salida estables.

---

# 26. Cuotas y entrada hostil

Estados, packs y reglas externas se consideran no confiables.

Jerarquía:

```text
HardLimits
  ↓
RuntimeLimits
  ↓
DefaultLimits cuando el host omite configuración
  ↓
recomendaciones externas
```

Un pack nunca puede elevar límites del runtime o del core.

Se limitan como mínimo:

- agentes
- tamaño total de estado
- creencias
- memorias
- metas
- relaciones
- reglas
- profundidad de proposiciones
- strings
- patrones de aprendizaje
- derivaciones

---

# 27. Domain Packs

MSPA define el contrato de Domain Packs, pero **los packs concretos viven fuera de este repositorio**, dentro de los proyectos consumidores.

Un pack puede declarar:

- `pack_id`
- versión
- namespace
- dependencias
- versión mínima MSPA
- vocabulario
- Predicate Schemas
- Event Templates
- Action Schemas
- consideraciones
- curvas
- reglas de inferencia
- `ContextTags`
- `OutcomeTags`
- valores y normas iniciales
- clases de influencia externa
- metadatos de portabilidad

Un Domain Pack no puede:

- ejecutar código nativo arbitrario
- acceder a filesystem o red
- elevar cuotas
- imponer emociones universales
- imponer obediencia a influencias
- introducir clases de un host dentro del core MSPA

---

# 28. Cultura

La cultura no es un controlador global.

Surge de la distribución poblacional de:

- valores
- creencias normativas
- expectativas
- prácticas aprendidas
- símbolos y conceptos

Las normas personales y las normas sociales percibidas son estados individuales.

La convergencia, divergencia y conflicto cultural deben emerger de los agentes y no de una regla especial `culture_conflict`.

---

# 29. Influencia externa

La influencia externa se modela de forma genérica.

El agente puede:

- aceptar
- resistir
- reinterpretar
- ignorar

Una influencia no equivale automáticamente a una orden ejecutada.

---

# 30. Diagnostics y explicabilidad

Desde el primer runtime funcional debe existir una traza compacta.

Debe poder registrar:

- operación
- agente
- `process_time`
- `event_time`
- cambios de creencias
- appraisal resumido
- emociones relevantes
- acciones candidatas
- contribuciones de utilidad
- decisión
- errores y cuotas alcanzadas

Ruta explicativa objetivo:

```text
estímulo
→ proposición
→ creencia
→ inferencia
→ appraisal
→ emoción
→ memoria/meta/relación
→ candidatos
→ consideraciones
→ decisión
→ REPORT
→ aprendizaje
```

La instrumentación no puede alterar el resultado de la simulación.

---

# 31. Conformidad

Antes de considerar estable la API deben existir escenarios abstractos que cubran:

- IDs aislados por mundo
- create/remove
- inicialización derivada
- OBSERVE/DECIDE/REPORT
- evidencia contradictoria
- estímulos tardíos
- conversión entre dominios temporales
- reapreciación
- urgencia
- histéresis
- import/export
- rekey/preserve de RNG
- símbolos desconocidos
- múltiples runtimes
- batch vs individual
- múltiples operaciones para un agente
- `BatchConflict`
- entrada hostil/cuotas
- `OutOfMemory` en frontera pública
- aprendizaje causal
- determinismo con misma seed y mismo orden

Los escenarios son datos neutrales y no contienen personajes, juegos o mundos concretos.

---

# 32. Orden de implementación

## Etapa 0 — conformidad y trazas

Antes de profundizar algoritmos, mantener escenarios neutrales y diagnostics mínimos para observar el comportamiento.

## Etapa 1 — base

- IDs fuertes
- tiempo
- dominios temporales
- `Result<T>`
- errores
- RNG
- cuotas

## Etapa 2 — runtime/lifecycle

- `Runtime`
- `AgentStore`
- create/remove/has
- múltiples runtimes

## Etapa 3 — semantics

- proposiciones
- registry snapshots
- schemas
- staging/publicación

## Etapa 4 — flujo mínimo

- OBSERVE
- DECIDE
- REPORT
- ADVANCE_TO
- diagnostics

## Etapa 5 — psicología

- beliefs
- appraisal
- emotions
- mood
- goals
- memory
- relationships

## Etapa 6 — decisión

- consideraciones
- curvas
- utility composition
- histéresis

## Etapa 7 — inferencia y aprendizaje

- reglas declarativas acotadas
- expectativas causales
- exploración/explotación
- transmisión social

## Etapa 8 — persistencia y seguridad

- export/import
- remapeo
- time rebase
- RNG policy
- validación profunda
- cuotas

## Etapa 9 — batch/rendimiento

- API batch
- orden canónico
- paralelización opcional
- equivalencia individual/batch

## Etapa 10 — bindings

- fachada C
- bindings externos construidos sobre la ABI C

---

# 33. Estado de implementación actual

Ya existen y están conectados al build:

- base IDs
- `AgentId` compuesto por mundo
- tiempo lógico y conversión racional
- `Result<T>` y errores
- RNG determinista
- `AgentPsychState`
- registry semántico por snapshots
- `AgentStore`
- `Runtime`
- lifecycle
- OBSERVE mínimo
- DECIDE mínimo
- REPORT mínimo
- ADVANCE_TO
- export/import mínimo
- validación/cuotas
- diagnostics
- belief subsystem
- appraisal subsystem
- emotion subsystem
- goal subsystem
- memory subsystem
- relationship subsystem
- learning subsystem
- utility composer básico
- hysteresis policy básica
- tests de `base`
- tests del flujo runtime

Lo que aún debe desarrollarse para producción:

- lenguaje completo de proposiciones
- inferencia declarativa
- actualización sofisticada de creencias
- appraisal definitivo
- sistema emocional definitivo
- mood/reappraisal completos
- metas derivadas
- consolidación de memoria
- utility curves configurables por Domain Pack
- histéresis completa
- aprendizaje `ActionType + ContextTags -> OutcomeTags`
- batches públicos
- serialización física versionada
- migraciones
- C ABI
- benchmarks y fuzz/security tests

---

# 34. Regla de handoff

Quien continúe la implementación debe:

1. trabajar sobre las interfaces existentes;
2. mantener MSPA agnóstico al host;
3. no añadir nombres ni reglas de un proyecto consumidor al core;
4. acompañar cada módulo con tests de conformidad;
5. mantener determinismo y orden canónico;
6. no introducir `unordered_map` en rutas donde el orden pueda afectar resultados;
7. mantener `RegistrySnapshot` inmutable durante batch;
8. no dejar excepciones cruzar APIs públicas;
9. conservar diagnostics suficientes para explicar cada decisión;
10. modificar la arquitectura sólo cuando una incompatibilidad real quede demostrada por implementación o tests.

---

# 35. Documentos canónicos separados

El README contiene el contrato técnico completo necesario para trabajar. Estos archivos permanecen para revisión normativa y trazabilidad:

- `docs/MSPA-SPEC-v2.4.md`
- `docs/MSPA-INTERACTION-CONTRACT-v0.2.md`
- `docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`
- `docs/MSPA-CODE-ARCHITECTURE-v0.1.md`

Si README y uno de esos documentos divergen, la inconsistencia debe corregirse; no debe resolverse inventando comportamiento.
