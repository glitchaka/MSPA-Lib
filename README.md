# MSPA-Lib

**MSPA — Motor de Simulación Psicológica y Cultural de Agentes**  
**Agent Psychology Simulation Middleware**

MSPA-Lib es una biblioteca C++17 para mantener estado psicológico subjetivo de agentes, interpretar estímulos, formar y revisar creencias, producir appraisal y afecto, conservar memoria y relaciones, aprender expectativas y elegir entre acciones que el host declara físicamente disponibles.

> MSPA procesa significado psicológico; el host controla la realidad y la ejecución.

Este README es autocontenido y funciona como entrada técnica. Los archivos de `docs/` conservan contratos normativos y evidencia de auditoría.

## Estado

Versión actual: **1.1.2**.

El núcleo implementa:

- múltiples runtimes aislados por proceso;
- lifecycle de agentes (`CREATE_AGENT`, `REMOVE_AGENT`, `HAS_AGENT`, `DerivedInit`);
- IDs fuertes, `AgentId = WorldId + local_id`;
- tiempo lógico `PsychTime` y dominios temporales racionales;
- `OBSERVE`, `DECIDE`, `REPORT`, `ADVANCE_TO`, `REACT` y formas batch;
- semántica estructurada, `RegistrySnapshot`, staging/publicación y Domain Packs;
- inferencia declarativa acotada;
- personalidad, perfiles psicológicos extensibles, valores, creencias, appraisal, emociones, mood, metas, memoria, relaciones, reapreciación y aprendizaje contextual;
- Utility AI psicológica, curvas, consideraciones e histéresis;
- export/import, rebase temporal, serialización binaria versionada y migración;
- RNG determinista por agente;
- validación defensiva, cuotas y `Result<T>` sin excepciones cruzando la API;
- diagnostics explicables;
- API C++ y ABI C;
- build estático/compartido, instalación CMake, tests, ASan/UBSan, fuzzing, mutation testing y benchmarks.

### Cambios 1.1.x

**1.1.0 — perfiles psicológicos extensibles.** `PsychologicalProfile`, `ProfileRegistry` y `ProfileRecipe` permiten que el host registre cuantos perfiles necesite, combine varios mediante pesos y aplique variación determinista por agente. MSPA no contiene perfiles concretos de juegos ni personajes.

**1.1.1 — baseline emocional tónico.** Todo agente activo conserva las seis emociones base por encima de cero. El baseline se deriva de personalidad y mood y el decaimiento vuelve a ese nivel en vez de eliminar la emoción.

**1.1.2 — endurecimiento conductual y auditoría.** Se separa explícitamente tono emocional de episodio para utilidad/urgencia/expresión, los fallos reducen la expectativa de éxito, la histéresis queda reflejada en la traza aritmética y se añaden pruebas de aceptación, fuzz smoke, mutation testing y benchmark poblacional. La auditoría encontró además un defecto real de timestamps emocionales (`PsychTime(0)` tratado como ausente), corregido antes del PASS final.

## 1. Límite de responsabilidad

### El host posee

- mundo y entidades físicas;
- percepción física y línea de visión;
- pathfinding, navegación, física y colisiones;
- inventario, recursos, economía y combate;
- animación y presentación;
- catálogo de acciones físicamente disponibles;
- scheduling del mundo;
- resultados reales de acciones;
- almacenamiento y transporte de bytes;
- semántica específica del consumidor.

### MSPA posee

- personalidad y perfiles de inicialización;
- valores;
- creencias subjetivas;
- inferencia;
- appraisal;
- emociones y mood;
- metas;
- memoria;
- relaciones direccionales;
- expectativas aprendidas;
- preferencia entre acciones ofrecidas;
- continuidad de intención;
- trazas explicables.

MSPA nunca inventa una acción física. `DECIDE` sólo selecciona entre `ActionOption` recibidos del host.

## 2. Flujo de integración

```text
HOST / WORLD
    │ OBSERVE
    ▼
MSPA: creencias -> inferencia -> appraisal -> emoción/mood -> memoria
    │ DECIDE(opciones físicamente posibles)
    ▼
INTENCIÓN / ELECCIÓN / EXPLICACIÓN
    │
HOST ejecuta la acción real
    │ REPORT(resultado)
    ▼
MSPA actualiza aprendizaje y estado
```

`REACT` es un wrapper `OBSERVE + DECIDE`; no define otra semántica.

## 3. Build e instalación

Requisitos:

- CMake 3.20+;
- C++17;
- MSVC, GCC o Clang;
- compilador C para validar el header de ABI C.

```bash
cmake -S . -B build -DMSPA_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Build compartido:

```bash
cmake -S . -B build-shared -DMSPA_BUILD_SHARED=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-shared --config Release
```

Instalación:

```bash
cmake --install build --prefix ./mspa-install
```

Consumo instalado:

```cmake
find_package(MSPA 1 CONFIG REQUIRED)
target_link_libraries(mi_programa PRIVATE MSPA::mspa)
```

El alias `MSPA::mspa` también existe en el árbol de construcción, por lo que un consumidor que use `FetchContent` puede enlazar el mismo target.

Opciones:

| Opción | Default | Función |
|---|---:|---|
| `MSPA_BUILD_TESTS` | ON | suites CTest |
| `MSPA_BUILD_BENCHMARKS` | OFF | benchmark OBSERVE/DECIDE |
| `MSPA_BUILD_SHARED` | OFF | biblioteca compartida |
| `MSPA_BUILD_FUZZ` | OFF | harness libFuzzer; requiere Clang |

Política numérica:

- MSVC `/fp:strict`;
- GCC/Clang sin fast-math y con `-ffp-contract=off`;
- `PsychTime` entero de 64 bits;
- escalas temporales racionales;
- no se promete identidad bit-a-bit entre toolchains distintos, pero sí determinismo lógico bajo mismo estado/config/input/seed.

## 4. Estructura

```text
include/mspa/
  mspa.hpp
  c_api.h
  base/
  model/
  semantics/
  psychology/
  decision/
  runtime/
  persistence/
  validation/
  diagnostics/
src/
  base/
  semantics/
  psychology/
  decision/
  runtime/
  persistence/
  validation/
  diagnostics/
  c/
tests/
benchmarks/
fuzz/
docs/
```

## 5. Identidad, runtimes y RNG

Los IDs son tipos fuertes (`RuntimeId`, `WorldId`, `NamespaceId`, `ConceptId`, `PredicateId`, `GoalId`, `BeliefId`, `MemoryId`, `ActionTypeId`, `ActionInstanceId`, `ActionContinuityKey`, `PackId`, `ProfileId`, `TimeDomainId`, etc.).

```text
AgentId
  WorldId world
  uint64 local_id
```

Varios `Runtime` pueden coexistir sin `AgentStore` o RNG mutable global. Cada runtime posee configuración, dominio temporal, snapshot semántico, agentes, seed root, motor psicológico, decisión, diagnostics y límites.

Cada agente deriva streams deterministas de `SeedRoot + AgentId + stream_kind`. Procesar otro agente no consume ni perturba la secuencia del primero.

## 6. Tiempo

```text
PsychTime = uint64 logical ticks
```

Cada runtime fija un `TimeDomain` con `TimeDomainId` y `TimeScale { ticks_per_second_num, ticks_per_second_den }`.

`process_time` indica cuándo MSPA procesa la operación y debe ser monotónico por agente. `event_time` indica cuándo ocurrió el hecho y puede ser anterior al estado actual.

```text
process_time < LastProcessedTime -> TimeRegression
event_time < LastProcessedTime   -> permitido
```

`RebaseRelative` convierte edades relativas al importar entre escalas distintas; nunca reinterpreta ticks de un dominio como si pertenecieran a otro.

## 7. Estado psicológico

`AgentPsychState` contiene:

```text
schema_version
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

### Personalidad

Seis ejes `[0,1]`:

- sociabilidad;
- estabilidad;
- empatía;
- persistencia;
- curiosidad;
- audacia.

De ellos se derivan rigidez de creencias, persistencia de metas, resistencia al cambio, sensibilidad a senales, sensibilidad social y exploración.

### Perfiles psicológicos

MSPA no trae perfiles concretos hardcodeados. El host usa:


``text
PsychologicalProfile
ProfileRegistry
ProfileRecipe
```

para registrar tantos perfiles como necesite, mezclarlos con pesos y añadir variación determinista por agente. Un perfil contiene personalidad, valores, mood basal y metas iniciales. Tras crear el agente, su estado evoluciona individualmente.

## 8. Semántica, registry y Domain Packs

Una `Proposition` contiene predicado, argumentos por rol, polaridad, namespace, calificadores, tiempo opcional y procedencia. `PredicateSchema` declara roles obligatorios/opcionales, agencia, targets, consecuencias, pistas de appraisal e intencionalidad.

`RegistrySnapshot` inmutable contiene predicados, conceptos, acciones, consideraciones, reglas de inferencia, packs cargados y valores iniciales.

Los cambios semánticos se realizan con `RegistryBuilder` en staging y se publican entre batches. El batch en curso conserva su snapshot.

Un `DomainPack` puede declarar vocabulario, schemas, acciones, consideraciones, curvas, reglas de inferencia, tags y valores iniciales. No puede ejecutar código nativo arbitrario, acceder a red/filesystem, elevar límites ni introducir clases concretas del host en el core.

## 9. Inferencia y creencias

`InferenceEngine` realiza forward chaining declarativo acotado por reglas, hipótesis y profundidad, evitando duplicados/ciclos simples.

Una creencia conserva proposición, certeza, carga emocional, fuente, agente fuente opcional, evidencia y tiempo. La actualización considera certeza previa, confianza de fuente, personalidad y resistencia emocional. Evidencia contradictoria puede debilitar la creencia y eventualmente invertir su polaridad.

## 10. Appraisal, emociones y mood

`AppraisalEngine` produce relevancia, consecuencia, agencia, intencionalidad, certeza, control, alineación normativa, error de expectativa y relación.

Emociones básicas:

- Joy;
- Sadness;
- Fear;
- Anger;
- Anxiety;
- Aversion.

Todo agente activo conserva las seis. El `EmotionSystem` distingue:

- **baseline tónico**: nivel basal derivado de personalidad/mood, mínimo 1%;
- **exceso episódico**: `max(0, intensidad - baseline)`.

El baseline tónico no se usa como si el agente estuviera episódicamente asustado/enfadado: no eleva por sólo el riesgo de `DECIDE`, no genera urgencia de `ADVANCE_TO` y no se publica como `expressed_emotion`. El decaimiento reduce episodios hacia el baseline.

`MoodSystem` mantiene valencia/activación y decae perezosamente con el tiempo.

## 11. Metas, memoria, relaciones y reapreciación

Las metas tienen prioridad, compromiso, estado y timestamps. Estados: `Active`, `Suspended`, `Blocked`, `Completed`, `Abandoned`.

La memoria conserva proposición, importancia, carga emocional, certeza, fuente, participantes y tiempos. La reapreciación puede alterar interpretación/carga afectiva sin reescribir silenciosamente la proposición factual almacenada.

Las relaciones actuales son direccionales `A -> B` y usan `AgentId`
como destino. El soporte para entidades no-agente sigue siendo un gap conocido.

## 12. Aprendizaje

El aprendizaje causal implementado es:

```text
ActionType + ContextTags -> OutcomeTag
```

Cada patrón conserva intentos, ocurrencias/éxitos, confianza y última activación. Desde 1.1.2 cada `REPORT` actualiza expectativas complementarias de `success` y `failuref� por lo que exitos repetidos elevan la expectativa de éxito y fallos repetidos la reducen. Los tags se canonicalizan y permiten especialización contextual.

El aprendizaje observacional de resultados de otros agentes todav nünca forma parte del core certificado.

## 13. Decisión e histéresis

`ActionOption` puede incluir tipo/instancia/continuidad, actor/targets, coste, riesgo, progreso, novedad, exposición social, coste de interrupción, estabilidad de contexto, tags, efectos esperados y señales.

`DecisionContext` permite presiones genéricas del host (por ejemplo necesidades) sin hardcodearlas en MSPA.

`UtilityComposer` combina coste, riesgo, progreso, personalidad, exceso emocional episódico, necesidades, valores, metas, relaciones, mood, expectativas aprendidas y consideraciones configuradas por Domain Packs.

`HysteresisPolicy` usa `ActionContinuityKey`, persistencia, progreso, coste de interrupción, estabilidad y urgencia. En 1.1.2 sus aportes aparecen dentro de `CandidateScore::considerations`, de modo que la suma de la traza reproduce la utilidad con la que se seleccionó la acción.

## 14. Lifecycle y operaciones públicas

### `CREATE_AGENT`
Crea estado psicológico validado. `DerivedInit` permite mezcla determinista de fuentes sin asumir parentesco/biología.

### `REMOVE_AGENT`
`ForgetRuntimeOnly` elimina el estado. `ValidateNoReferences` rechaza si otros estados aún lo referencian.

### `OBSERVE`
Valida proposición y tiempos, actualiza creencias, ejecuta inferencia, appraisal, emociones/mood y memoria. Devuelve `ObserveResult` con cambio, recomendación, urgencia, amenaza a acción actual y token diagnóstico.

### `DECIDE`
Selecciona únicamente entre acciones ofrecidas. Devuelve selección, target, confianza, intención, emoción expresada episódica y traza de candidatos.

### `REPORT`
Comunica el resultado real y actualiza aprendizaje, relaciones y proposiciones resultantes.

### `ADVANCE_TO`
Materializa decaimiento emocional/mood, metas y consolidación de memoria. Tiempo regresivo devuelve `TimeRegression`.

## 15. Batch y determinismo

Existen `observe_batch`, `decide_batch`, `report_batch`, `advance_batch`. Orden canónico:

```text
(AgentId, input_sequence)
```

Duplicar `input_sequence` para el mismo agente o producir tiempo irresoluble devuelve `BatchConflict`. Una llamada individual es conceptualmente batch de uno.

Auditoría 1.1.2 demuestra:

- mismo escenario dos veces -> bytes exportados idénticos;
- añadir/procesar otro agente no cambia el estado exportado del objetivo;
- batch barajado == batch canónico;
- batch == llamadas individuales.

## 16. Persistencia

`ExportedAgent` incluye estado, dominio temporal de origen y `exported_at`.

Políticas:

- `RebaseRelative` (default);
- `PreserveAbsolute` para dominio/escala compatibles;
- `RejectIncompatible`.

RNG de importación:

- `RekeyForDestination` por defecto;
- `PreserveState` para restore/replay explícito.

La serialización binaria versionada valida magic, versión, longitudes, límites y truncado. El host controla dónde se guardan/transportan bytes.

## 17. Validación, cuotas y errores

Jerarquía:

```text
HardLimits
  ↓
RuntimeLimits
  ↓
DefaultLimits
```

Se limitan agentes, valores, creencias, emociones, memorias, metas, relaciones, patrones de aprendizaje, argumentos, strings, bytes serializados y diagnostics. Los packs no pueden elevar límites.

La API C++ usa `Result<T>`. Códigos principales:

- `InvalidArgument`;
- `UnknownAgent`;
- `DuplicateAgent`;
- `InvalidState`;
- `InvalidPack`;
- `QuotaExceeded`;
- `VersionMismatch`;
- `NamespaceConflict`;
- `TimeRegression`;
- `BatchConflict`;
- `OutOfMemory`;
- `UnsupportedOperation`;
- `SerializationError`;
- `InternalInvariantViolation`.

`std::bad_alloc` se captura en fronteras públicas y se traduce a `OutOfMemory`.

## 18. Diagnostics y explicación

`TraceSink` conserva trazas acotadas. La instrumentación no consume RNG ni modifica el resultado. La traza puede cubrir:

```text
OBSERVE
-> belief update / inference
-> appraisal
-> emotion / mood
-> DECIDE candidates
-> considerations
-> hysteresis
-> selected action
-> REPORT / learning
```

Las pruebas 1.1.2 verifican que las contribuciones listadas suman la utilidad y que el candidato de mayor utilidad trazada coincide con la acción seleccionada.

## 19. ABI C

`mspa/c_api.h` expone un `mspa_runtime*` opaco, versión, lifecycle, observe, decide/decide_ex, report/report_ex, advance, export/import de bytes, buffers, errores y registro de perfiles. El header se compila como C real.

## 20. Auditoría y pruebas

Suites principales CTest:

```text
mspa_core_tests
mspa_semantics_tests
mspa_psychology_tests
mspa_profiles_tests
mspa_emotion_baseline_tests
mspa_runtime_tests
mspa_persistence_tests
mspa_c_api_tests
mspa_security_tests
mspa_audit_determinism_tests
mspa_audit_behavior_tests
mspa_audit_security_time_tests
mspa_audit_trace_scenario_tests
mspa_c_header_tests
```

El workflow normal valida GCC/Linux, Clang con ASan/UBSan, MSVC/Windows y build compartido. `.github/workflows/audit.yml` añade CTest verbose, benchmark, 5.000 runs libFuzzer y mutation testing de histéresis.

Evidencia de 1.1.2:

- [`docs/audit/MSPA-1.1.2-AUDIT-PLAN.md`](docs/audit/MSPA-1.1.2-AUDIT-PLAN.md)
- [`docs/audit/MSPA-1.1.2-AUDIT-RESULTS.md`](docs/audit/MSPA-1.1.2-AUDIT-RESULTS.md)
- [`docs/audit/MSPA-1.1.2-CTEST-VERBOSE.md`](docs/audit/MSPA-1.1.2-CTEST-VERBOSE.md)
- [`docs/audit/MSPA-1.1.2-TRACE-FOUR-PROFILES.md`](docs/audit/MSPA-1.1.2-TRACE-FOUR-PROFILES.md)
- [`docs/audit/MSPA-CONFORMANCE-MATRIX.md`](docs/audit/MSPA-CONFORMANCE-MATRIX.md)

Benchmark de referencia en GitHub Actions (Ubuntu 24.04 / GCC 13.3, 100.000 operaciones por caso): aproximadamente 35k operaciones/s tanto en `OBSERVE` como en `DECIDE` para poblaciones de 100, 1.000 y 10.000 agentes. Es una línea base, no un SLA.

## 21. Gaps culturales explícitos

El nombre MSPA conserva la ambición psicológica y cultural, pero **la auditoría 1.1.2 no declara completa la capa cultural**. Aún faltan contratos/implementación/tests para:

- aprendizaje observacional del resultado de otros agentes;
- normas percibidas de grupo;
- afirmaciones/rumores transmisibles como salida explícita;
- métricas poblacionales de deriva, diversidad y polarización;
- relaciones con destinos no representados por `AgentId`.

Estos puntos figuran como `GAP` en la matriz de conformidad y no deben presentarse como funcionalidad ya implementada.

## 22. Invariantes de mantenimiento

1. El core permanece agnóstico al host.
2. El host conserva autoridad sobre realidad y ejecución.
3. MSPA sólo elige acciones ofrecidas.
4. IDs fuertes no se intercambian implícitamente.
5. `process_time` es monotónico por agente; `event_time` puede ser anterior.
6. Registry/configuración visibles son inmutables durante batch.
7. RNG es independiente por agente.
8. Ninguna excepción cruza las fronteras públicas.
9. Toda entrada externa se valida bajo cuotas.
10. Instrumentación no altera resultados.
11. Packs y adaptadores concretos viven fuera del core.
12. Nuevas reglas psicológicas deben conservar trazabilidad y tests.
13. Una capacidad no se marca como PASS sólo porque esté descrita; debe existir una prueba de conformidad asociada.

## 23. Documentos normativos

Se mantienen bajo sus nombres existentes:

- `docs/MSPA-SPEC-v2.4.md`
- `docs/MSPA-INTERACTION-CONTRACT-v0.2.md`
- `docs/MSPA-DOMAIN-PACK-CONTRACT-v0.1.md`
- `docs/MSPA-CODE-ARCHITECTURE-v0.1.md`

La matriz `docs/audit/MSPA-CONFORMANCE-MATRIX.md` enlaza capacidades con implementación y tests para reducir el riesgo de que README y documentos normativos se desincronicen.
