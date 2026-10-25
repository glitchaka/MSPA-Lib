# MSPA — Matriz de conformidad y cobertura

Esta matriz enlaza promesas técnicas con implementación y pruebas. Estado referido al código auditado `9c0a8f740fdac51dba886a19e2c0456d1349d95e`.

| Capacidad / invariante | Fuente normativa principal | Implementación | Prueba | Estado |
|---|---|---|---|---|
| IDs fuertes / `AgentId` incluye mundo | README §4; CODE-ARCHITECTURE | `base/ids`, runtime | `core_tests`, `audit_determinism` | PASS |
| `process_time` monotónico / `event_time` tardío | README §5; INTERACTION-CONTRACT | `runtime`, `base/time` | `audit_security_time` | PASS |
| Múltiples runtimes aislados | README §4 | `Runtime` | `core_tests`, `runtime_tests` | PASS |
| RNG independiente por agente | README §20 | `base/rng` | `core_tests`, `audit_determinism` | PASS |
| Batch canónico `(AgentId,input_sequence)` | README §19 | `Runtime::*_batch` | `audit_determinism` | PASS |
| Batch == llamadas individuales | README §19 | runtime | `audit_determinism` | PASS |
| Perfiles psicológicos extensibles | README §6 / 1.1.x | `ProfileRegistry`, `ProfileRecipe` | `profiles_tests`, `audit_behavior` | PASS |
| Evidencia modulada por personalidad | README §§6,11 | `BeliefSystem`, `PersonalitySystem` | `audit_behavior` | PASS |
| Inferencia declarativa acotada | README §10 | `InferenceEngine` | `semantics_tests`, `audit_trace_scenario` | PASS |
| Appraisal -> emoción/mood | README §12 | `AppraisalEngine`, `EmotionSystem`, `MoodSystem` | `psychology_tests`, `audit_trace_scenario` | PASS |
| Baseline tónico >0 para agente activo | README §12 / 1.1.1 | `EmotionSystem` | `emotion_baseline_tests`, `audit_behavior` | PASS |
| Baseline no cuenta como episodio | README §12 / 1.1.2 | decision/runtime | `audit_behavior` | PASS |
| Histéresis reduce oscilación | README §17 | `HysteresisPolicy`, runtime | `audit_behavior` + mutation test | PASS |
| Explicación aritméticamente coherente | README §§17,25 | `CandidateScore::considerations`, runtime | `audit_behavior`, `audit_trace_scenario` | PASS |
| REPORT actualiza expectativa de éxito/fallo | README §16 | `LearningSystem` | `audit_behavior` | PASS |
| Aprendizaje contextual | README §16 | `LearningSystem` | `audit_behavior` | PASS |
| Contradicción / inversión de creencia | README §11 | `BeliefSystem` | `psychology_tests`, `audit_behavior` | PASS |
| Reapreciación no reescribe proposición | README §14 | `ReappraisalSystem` | `audit_behavior` | PASS |
| Export serializado determinista | README §22 | serialization/runtime | `audit_determinism`, `persistence_tests` | PASS |
| Rebase temporal entre escalas | README §21 | `state_transfer` | `persistence_tests`, `audit_security_time` | PASS |
| Entrada hostil / cuotas | README §§23-24 | validation/serialization/domain packs | `security_tests`, `audit_security_time`, fuzz | PASS |
| ABI C compila como C | README §27 | `c_api.h`, `src/c` | `c_api_tests`, `c_header_tests` | PASS |
| GCC / Clang sanitizers / MSVC / shared | README §28 | CMake + CI | `.github/workflows/ci.yml` | PASS |
| Throughput OBSERVE/DECIDE publicado | README §28 | benchmark | `benchmark.cpp`, audit workflow | PASS (baseline) |
| Aprendizaje observacional de otros | Promesa cultural histórica | — | — | **GAP** |
| Normas percibidas de grupo | Promesa cultural histórica | — | — | **GAP** |
| Afirmaciones/rumores transmisibles desde DECIDE | INTERACTION-CONTRACT aspiracional | salida explícita no implementada | — | **GAP** |
| Métricas poblacionales culturales | Promesa cultural histórica | — | — | **GAP** |
| Relaciones con entidades no-agente | Caso de uso planteado | `RelationshipState` usa `AgentId` | — | **GAP** |

## Regla de mantenimiento

Una fila no debe marcarse PASS sólo porque el README lo diga. PASS requiere un camino de código y una prueba ejecutable. Una capacidad nueva debe añadir o ampliar la fila correspondiente y enlazar su test de conformidad.
