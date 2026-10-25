# MSPA — Matriz de conformidad y cobertura

Estado referido al código auditado `448baa2f324bd5627c7b8ea9f2eed23d77ace61a`. Los criterios de fase 2 fueron congelados previamente en `964933b5a70e885eabbb82b44d5495a9619180ed`. PASS requiere ruta de código pública y prueba ejecutable; una promesa sin implementación aparece como **NOT IMPLEMENTED** y una promesa implementada sin prueba debe aparecer como **SIN COBERTURA**.

## Grupo A — requisitos auditados

| ID | Requisito normativo / promesa | Módulo / API pública | Test | Resultado | Commit auditado |
|---|---|---|---|---|---|
| A1 | Determinismo; aislamiento por agente; batch canónico; batch==individual — README §§19–20; CODE-ARCHITECTURE determinismo/batch | `Runtime`, `*_batch`, `export_agent_bytes`, `mspa_observe_batch` | `audit_phase2_system_tests`, `audit_phase2_c_abi_tests` | **PASS** | `448baa2f…` |
| A2 | Personalidad debe producir divergencia psicológica — README §§6, 11–12; SPEC personalidad/creencias/emoción | `ProfileRegistry`, `BeliefSystem`, `AppraisalEngine`, `EmotionSystem` | `audit_phase2_behavior_tests` | **PASS** — certeza `0.399665<0.705035`; miedo/recuperación ordenados | `448baa2f…` |
| A3 | Histéresis debe reducir oscilación — README §17; SPEC/arquitectura Utility AI | `HysteresisPolicy`, `Runtime::decide` | `audit_phase2_behavior_tests` + mutation | **PASS** — `0/99` cambios | `448baa2f…` |
| A4 | Aprendizaje éxito/fallo y especialización contextual — README §16; SPEC aprendizaje | `LearningSystem`, `REPORT`, `DECIDE` | `audit_phase2_behavior_tests`, `audit_phase2_c_abi_tests` | **PASS** — `0.9/0.1`; contextos diferenciados | `448baa2f…` |
| A5 | Contradicción, sesgo por rigidez/carga y reapreciación factual — README §§11,14; SPEC creencias/memoria | `BeliefSystem`, `ReappraisalSystem` | `audit_phase2_behavior_tests` | **PASS** | `448baa2f…` |
| A6 | Baseline emocional >=1% sin falsa urgencia/decisión — README §12; SPEC emociones | `EmotionSystem`, `ADVANCE_TO`, `OBSERVE`, `DECIDE` | `audit_phase2_behavior_tests` + baseline mutation | **PASS** | `448baa2f…` |
| A7 | Explicabilidad real; diagnostics no altera estado/RNG — README §§17,25; SPEC explicabilidad | `CandidateScore::considerations`, `TraceSink`, `Runtime::decide` | `audit_phase2_behavior_tests` | **PASS** | `448baa2f…` |
| A8 | Entrada hostil y presupuestos — README §§23–24; INTERACTION-CONTRACT seguridad/cuotas | validation, serialization, domain packs, fuzz target | `security_tests`, `audit_security_time_tests`, libFuzzer | **PASS** — 5.000 runs sin crash | `448baa2f…` |
| A9 | `process_time`, `event_time`, `RebaseRelative`, `RekeyForDestination` — README §§5,21; INTERACTION-CONTRACT tiempo/import | `base/time`, `state_transfer`, runtime import; ABI C import | `audit_phase2_system_tests`, `audit_phase2_c_abi_tests` | **PASS** | `448baa2f…` |
| A10 | Round-trip exacto y migración de estado anterior — README §22; INTERACTION-CONTRACT persistencia | serialization, `migrate_agent_state` | `audit_phase2_system_tests` | **PASS** — round-trip complejo 659 bytes | `448baa2f…` |
| A11 | Lifecycle, runtimes aislados, referencias y mundos — INTERACTION-CONTRACT lifecycle; CODE-ARCHITECTURE múltiples runtimes | `CREATE_AGENT`, `REMOVE_AGENT`, `HAS_AGENT`, `RemovePolicy` | `audit_phase2_system_tests` | **PASS** | `448baa2f…` |
| A12 | A1/A4/A9 equivalentes por ABI C; ninguna excepción cruza C — README §27; CODE-ARCHITECTURE ABI | `mspa/c_api.h`, `src/c`, `mspa_observe_batch` | `audit_phase2_c_abi_tests`, `c_header_tests` | **PASS** — bytes C==C++; batch/context/rekey cubiertos | `448baa2f…` |
| A13 | Cuotas efectivas y sin corrupción; emociones>=6; pack no eleva limits — INTERACTION-CONTRACT cuotas; CODE-ARCHITECTURE validation | `RuntimeLimits`, validation, `TraceSink`, Domain Pack staging | `audit_phase2_system_tests` + quota mutation | **PASS** | `448baa2f…` |
| A14 | Benchmark OBSERVE/DECIDE 100/1k/10k — plan de calibración de límites | `benchmark.cpp` | audit workflow | **MEDIDO** — ~35–36k ops/s en runner final; sin SLA | `448baa2f…` |
| A15 | Mutation testing: histéresis, baseline, rebase, cuotas, orden, reapreciación | workflow de auditoría + APIs anteriores | matrix `mutation-coverage` | **PASS** — 6/6 mutantes eliminados | `448baa2f…` |

## Cobertura técnica adicional

| Capacidad / invariante | Fuente normativa principal | Implementación | Prueba | Estado |
|---|---|---|---|---|
| IDs fuertes / `AgentId` incluye mundo | README §4; CODE-ARCHITECTURE | `base/ids`, runtime | `core_tests`, A11 | PASS |
| RNG independiente por agente | README §20 | `base/rng` | `core_tests`, A1/A9/A12 | PASS |
| Inferencia declarativa acotada | README §10; SPEC inferencia | `InferenceEngine` | `semantics_tests`, `audit_trace_scenario` | PASS |
| Appraisal -> emoción/mood | README §12; SPEC appraisal/emoción | `AppraisalEngine`, `EmotionSystem`, `MoodSystem` | `psychology_tests`, `audit_trace_scenario`, A2/A6 | PASS |
| GCC / Clang sanitizers / MSVC / shared | README §28 | CMake + CI | `.github/workflows/ci.yml` | PASS |

## Grupo B — carencias explícitas

Estas filas no se consideran cubiertas por inferencia ni por proximidad con otras funciones.

| Requisito normativo | Fuente normativa | Módulo/API actual | Test | Estado |
|---|---|---|---|---|
| Aprendizaje observacional del resultado de otros agentes | SPEC v2.4 — aprendizaje y capa social/cultural | no existe operación pública que aprenda del resultado ajeno como observador | — | **NOT IMPLEMENTED** |
| Normas percibidas de grupo | SPEC v2.4 — normas/cultura emergente | no existe estado/API de norma percibida grupal | — | **NOT IMPLEMENTED** |
| Afirmaciones/rumores transmisibles y expresión explícita desde `DECIDE` | SPEC v2.4 — comunicación/transmisión; INTERACTION-CONTRACT salida de decisión | no existe comportamiento completo de generación/transmisión verificable | — | **NOT IMPLEMENTED** |
| Relaciones con entidades no-agente | SPEC v2.4 — relaciones/portabilidad | `RelationshipState::other` es `AgentId` | — | **NOT IMPLEMENTED** |
| Métricas poblacionales de deriva cultural, diversidad o polarización | SPEC v2.4 — cultura como distribución emergente | no existe agregador poblacional público | — | **NOT IMPLEMENTED** |

## Historial de defectos detectados por la auditoría

| Fase | Defecto detectado | Estado |
|---|---|---|
| 1 | `PsychTime(0)` tratado como timestamp ausente durante decay emocional | corregido y cubierto |
| 2 | `Proposition::qualifiers` deserializado con orden de evaluación no garantizado (`emplace(r.str(), r.str())`) | corregido en `3ee66f3356e15b0c45838fceb360896f5d842b17` y cubierto por A10 |
| 2 | ABI C sin batch, insuficiente para afirmar A12/A1 completo | `mspa_observe_batch` añadido y cubierto en `448baa2f324bd5627c7b8ea9f2eed23d77ace61a` |

## Regla de mantenimiento

Una fila no debe marcarse PASS sólo porque una especificación o README la prometa. PASS requiere implementación accesible por la API correspondiente y una prueba ejecutable contra esa API. Los requisitos no implementados permanecen `NOT IMPLEMENTED`; los implementados sin prueba permanecen `SIN COBERTURA`.
