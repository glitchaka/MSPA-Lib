# MSPA 1.1.2 — Auditoría fase 2: resultados

## Identificación

- Criterios congelados antes de ejecutar: `964933b5a70e885eabbb82b44d5495a9619180ed` (`MSPA-1.1.2-AUDIT-PHASE2-CRITERIA.md`).
- Código finalmente auditado: `448baa2f324bd5627c7b8ea9f2eed23d77ace61a`.
- Workflow de auditoría: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541
- Workflow CI multiplataforma: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956500
- API usada por los tests de fase 2: sólo headers públicos `include/mspa/**` y `mspa/c_api.h`; ningún test incluye `src/` ni símbolos privados.
- GCC Release: PASS.
- Clang + ASan/UBSan: PASS.
- MSVC Release: PASS.
- Shared build: PASS.
- CTest: **17/17 PASS**.
- Fuzz smoke: **5.000 ejecuciones libFuzzer sin crash/sanitizer finding**.
- Mutation testing: **6/6 mutantes eliminados**.

## Hallazgos reales durante la fase 2

### 1. A10 encontró una pérdida de fidelidad en serialización

La primera ejecución de fase 2 falló en `A10 complex round-trip must be byte exact`. El fallo estaba en la deserialización de `Proposition::qualifiers`: se usaba `emplace(r.str(), r.str())`. En C++17 el orden de evaluación de esos argumentos no está garantizado, por lo que una pareja serializada `scope=local` podía reconstruirse como `local=scope`.

Se corrigió leyendo clave y valor secuencialmente antes de insertarlos. Commit de corrección: `3ee66f3356e15b0c45838fceb360896f5d842b17`.

### 2. A12 reveló paridad incompleta de la ABI C

La ABI C podía ejecutar `OBSERVE`, `DECIDE`, `REPORT`, import/export y tiempo, pero no exponía batch; por tanto no podía repetir A1 completo y A12 no debía marcarse PASS. Se añadió `mspa_observe_batch` y se amplió la prueba C para cubrir:

- repetición determinista;
- aislamiento frente a un agente adicional;
- batch barajado == batch canónico;
- batch == llamadas individuales;
- bytes C == C++ para el mismo escenario;
- aprendizaje éxito/fallo y especialización por `ContextTags`;
- `RebaseRelative` y `RekeyForDestination` mediante la política por defecto de la ABI C;
- dos destinos importados desde el mismo origen con RNG distintos;
- `event_time` histórico aceptado y `process_time` regresivo -> `MSPA_TIME_REGRESSION`;
- ninguna excepción cruza `extern "C"`.

La paridad ampliada quedó en `448baa2f324bd5627c7b8ea9f2eed23d77ace61a`.

## Resultado A1–A15

| ID | Requisito congelado | Resultado observado | Estado | Test principal |
|---|---|---|---|---|
| A1 | Determinismo, aislamiento, batch canónico, batch==individual | mismos bytes; agente extra no perturba; batch multiagente canónico; batch e individual byte-equivalentes | **PASS** | `audit_phase2_system_tests.cpp`, `audit_phase2_c_abi_tests.cpp` |
| A2 | Divergencia relativa entre perfiles | certeza rígido `0.399665` < flexible `0.705035`; stable-bold genera menos miedo y recupera antes que unstable | **PASS** | `audit_phase2_behavior_tests.cpp` |
| A3 | Histéresis <=10 cambios y >=3x menos que sin continuidad | `0/99` cambios con/sin continuidad | **PASS** | `audit_phase2_behavior_tests.cpp` |
| A4 | Éxito/fallo y especialización contextual | expectativas `0.9/0.1`; `DECIDE` favorece la acción aprendida; rain/sun divergen por >0.50 | **PASS** | `audit_phase2_behavior_tests.cpp`, `audit_phase2_c_abi_tests.cpp` |
| A5 | Contradicción, rigidez/carga emocional, reapreciación factual | certeza baja antes de invertir; carga emocional y rigidez ralentizan; evidencia suficiente invierte; reapreciación conserva proposición | **PASS** | `audit_phase2_behavior_tests.cpp` |
| A6 | Seis emociones >=1%, sin falsa urgencia ni cambio de decisión | seis baselines presentes; todas >=0.01; `ADVANCE_TO` no recomienda; neutral no sube a Soon/Immediate; elección igual al control | **PASS** | `audit_phase2_behavior_tests.cpp` |
| A7 | Explicación reproducible y diagnostics inerte | suma `weighted` reproduce utilidad; ganador de traza reproduce selección; diagnostics on/off produce mismos bytes, selección y RNG | **PASS** | `audit_phase2_behavior_tests.cpp` |
| A8 | Entrada hostil y fuzz | truncados/versiones/cuotas fallan limpiamente; corpus determinista; 5.000 runs libFuzzer sin crash | **PASS** | `security_tests.cpp`, `audit_security_time_tests.cpp`, fuzz |
| A9 | Tiempo/import/rebase/rekey | `TimeRegression`; event_time histórico aceptado; timestamps complejos rebasados; RNG destino rederivado y diferente entre destinos | **PASS** | `audit_phase2_system_tests.cpp`, `audit_phase2_c_abi_tests.cpp` |
| A10 | Round-trip complejo + migración previa | round-trip binario exacto `659` bytes; schema anterior soportado migra a `state_schema_version` actual | **PASS** | `audit_phase2_system_tests.cpp` |
| A11 | Lifecycle y aislamiento | runtimes independientes; `DuplicateAgent`; `ValidateNoReferences`; rechazo de AgentId de otro mundo | **PASS** | `audit_phase2_system_tests.cpp` |
| A12 | A1/A4/A9 equivalentes por ABI C | `learned_action=10`; estado C byte-equivalente a C++; batch C equivalente; doble rekey distinto; errores C devuelven status | **PASS** | `audit_phase2_c_abi_tests.cpp`, `c_header_tests.c` |
| A13 | Todas las cuotas y no corrupción | límites de agentes/state/proposition/string/serialized probados; diagnostics acota retención; `<6` emociones rechazado; pack no eleva limits | **PASS** | `audit_phase2_system_tests.cpp` |
| A14 | Medición 100/1k/10k, sin umbral | benchmark publicado; no se usa como gate de aprobación | **MEDIDO** | `benchmark.cpp` |
| A15 | Seis mutaciones deben ser detectadas | 6/6 mutantes eliminados por pruebas públicas | **PASS** | `.github/workflows/audit.yml` |

## Salida CTest final

```text
AUDIT_PHASE2_BEHAVIOR PASS certainty_order=0.399665<0.705035 switches=0/99 learning=0.9/0.1 diagnostics=2/0
AUDIT_PHASE2_SYSTEM PASS deterministic_bytes=534 canonical_first=1 roundtrip_bytes=659 rekey=5344374020829512130/2729721105051713814 diagnostics_cap=2
AUDIT_PHASE2_C_ABI PASS learned_action=10 bytes=684 batch_low=428 rekey=7993962808973275737/4608737480990244005
100% tests passed, 0 tests failed out of 17
```

## A14 — benchmark final

Runner: Ubuntu 24.04, GCC 13.3.0, Release. Cada fila ejecuta 100.000 operaciones totales.

| Operación | Agentes | Segundos | ops/s |
|---|---:|---:|---:|
| OBSERVE | 100 | 2.848603 | 35,104.93 |
| DECIDE | 100 | 2.796163 | 35,763.29 |
| OBSERVE | 1.000 | 2.827607 | 35,365.59 |
| DECIDE | 1.000 | 2.802959 | 35,676.58 |
| OBSERVE | 10.000 | 2.789102 | 35,853.83 |
| DECIDE | 10.000 | 2.823607 | 35,415.70 |

No existe umbral PASS/FAIL de rendimiento en esta fase; estos datos sirven para calibrar `DefaultLimits` y detectar regresiones posteriores.

Artifact CTest + benchmark: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588940726
SHA-256 del artifact: `e045031d9b2c9358f1407ed050001f7eebdf1d8f3f17ee04264f4e7936d58625`.

## A8 — fuzz

Workflow final: `fuzz-smoke` sobre el mismo SHA auditado. 5.000 ejecuciones completadas sin crash ni sanitizer finding.

Artifact: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588930794
SHA-256: `c2308e938db9778c8909228f14c1b2b4c0c2cb0e03d44a50de132d4eff4c515b`.

Esto es evidencia de ese smoke run, no una demostración matemática de ausencia de vulnerabilidades.

## A15 — mutation testing

| Mutación deliberada | Assert que la detectó | Resultado |
|---|---|---|
| Histéresis anulada | `A3 continuity switch count must be <=10` | **KILLED** |
| Baseline emocional a cero | `every tonic baseline remains at least one percent` | **KILLED** |
| Rebase usando escala origen como destino | `A9 rebase last_processed_time` | **KILLED** |
| Bypass de cuota de memorias | `memory quota` | **KILLED** |
| Orden canónico invertido | `A1 batch output must use canonical AgentId ordering` | **KILLED** |
| Reapreciación altera polaridad factual | `reappraisal must preserve factual proposition` | **KILLED** |

Artifacts finales:

- hysteresis: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588869593
- baseline: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588714892
- rebase: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12589040514
- quotas: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588099939
- canonical_order: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12589385083
- reappraisal: https://github.com/glitchaka/MSPA-Lib/actions/runs/40167956541/artifacts/12588754798

## Grupo B — estado real

Estas capacidades aparecen en la especificación como objetivos/capacidades culturales o de comunicación, pero **no tienen implementación demostrada** en 1.1.2. Por regla de esta auditoría no reciben tests ficticios ni PASS:

| Requisito | Referencia normativa | Estado |
|---|---|---|
| Aprendizaje observacional de resultados de otros agentes | SPEC v2.4 — aprendizaje/acumulación de expectativas y capa social/cultural | **NOT IMPLEMENTED** |
| Normas percibidas de grupo | SPEC v2.4 — normas/cultura emergente | **NOT IMPLEMENTED** |
| Afirmaciones/rumores transmisibles y expresión explícita en `DECIDE` | SPEC v2.4 — comunicación/transmisión; INTERACTION-CONTRACT — salida de decisión | **NOT IMPLEMENTED** |
| Relaciones con entidades no-agente | SPEC v2.4 — relaciones/portabilidad; implementación actual usa `AgentId` | **NOT IMPLEMENTED** |
| Métricas poblacionales de deriva cultural, diversidad o polarización | SPEC v2.4 — cultura como distribución emergente | **NOT IMPLEMENTED** |

## Archivos para revisión externa

- Criterios congelados: https://github.com/glitchaka/MSPA-Lib/blob/964933b5a70e885eabbb82b44d5495a9619180ed/docs/audit/MSPA-1.1.2-AUDIT-PHASE2-CRITERIA.md
- A2–A7: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/tests/audit_phase2_behavior_tests.cpp
- A1/A9/A10/A11/A13: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/tests/audit_phase2_system_tests.cpp
- A12: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/tests/audit_phase2_c_abi_tests.cpp
- ABI C pública: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/include/mspa/c_api.h
- Mutation workflow: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/.github/workflows/audit.yml
- Benchmark: https://github.com/glitchaka/MSPA-Lib/blob/448baa2f324bd5627c7b8ea9f2eed23d77ace61a/benchmarks/benchmark.cpp

## Conclusión de fase 2

Sobre el SHA `448baa2f324bd5627c7b8ea9f2eed23d77ace61a`, A1–A13 y A15 pasan los criterios congelados; A14 queda medido sin umbral de aprobación. Las capacidades del Grupo B permanecen explícitamente **NOT IMPLEMENTED**.
