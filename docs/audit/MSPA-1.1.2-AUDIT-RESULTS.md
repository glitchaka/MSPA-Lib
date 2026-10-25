# MSPA 1.1.2 — Resultados de auditoría conductual

## Identificación de la ejecución

- Código auditado: `9c0a8f740fdac51dba886a19e2c0456d1349d95e`.
- Workflow: [MSPA Behavioral Audit](https://github.com/glitchaka/MSPA-Lib/actions/runs/40164846977).
- Runner principal: Ubuntu 24.04, GCC 13.3.0, Release.
- Validación adicional: Clang + ASan/UBSan, MSVC/Windows y build compartido mediante el workflow normal de CI.
- CTest: **14/14 PASS**.
- Fuzz smoke: **5.000 ejecuciones libFuzzer sin caída**.
- Mutation test: **PASS**; la suite detectó la eliminación deliberada de histéresis.

## Hallazgo durante la auditoría

La primera ejecución (`191da2fdf16533bb4e8606d17fed83eb2e37d632`) falló correctamente en la prueba «el perfil estable se calma antes que el inestable». La causa no era el test: `EmotionSystem::ensure_baseline()` trataba `PsychTime(0)` como timestamp ausente y, al entrar a `ADVANCE_TO`, reemplazaba `updated_at` por el tiempo actual antes de calcular el decaimiento. Eso convertía el tiempo transcurrido en cero.

Se corrigió preservando `PsychTime(0)` como tiempo válido. La corrección quedó en `9c0a8f740fdac51dba886a19e2c0456d1349d95e` y la auditoría completa se repitió desde cero. Esta secuencia es evidencia de que la nueva capa de aceptación detecta regresiones conductuales reales.

## Resultado de las once pruebas solicitadas

| # | Área | Resultado | Evidencia |
|---|---|---|---|
| 1 | Determinismo | **PASS** | mismo escenario -> 550 bytes idénticos; aislamiento entre agentes; batch barajado == canónico; batch == individual |
| 2 | Divergencia entre perfiles | **PASS** | certeza rígido `0.399665` vs flexible `0.705035`; además se afirma que el perfil estable reduce miedo antes que el inestable |
| 3 | Histéresis | **PASS** | 100 decisiones near-tie: `0` cambios con histéresis vs `99` sin continuidad |
| 4 | Bucle de aprendizaje | **PASS** | expectativa tras éxitos `0.9`; tras fallos `0.1`; `DECIDE` favorece la opción aprendida; contexto especializado |
| 5 | Creencias / reapreciación | **PASS** | contradicción reduce certeza e invierte polaridad; carga emocional ralentiza el cambio; reapreciación no reescribe la proposición |
| 6 | Baseline emocional | **PASS** | seis emociones >= 1%; el tono basal no altera utilidad, no genera urgencia y no se expone como episodio emocional |
| 7 | Explicación coherente | **PASS** | la suma de `weighted` reproduce `CandidateScore::utility`; el ganador de la traza coincide con la selección, incluida histéresis |
| 8 | Entrada hostil | **PASS** | truncado, versión desconocida, longitud absurda, cuotas y pack sobredimensionado devuelven errores; corpus determinista 500/500 rechazado; libFuzzer 5.000 runs sin crash |
| 9 | Tiempo / import | **PASS** | `process_time` regresivo -> `TimeRegression`; `event_time` antiguo aceptado; `RebaseRelative` conserva intervalos entre escalas 10 Hz -> 20 Hz |
| 10 | Sensibilidad de tests | **PASS** | al anular `continuity_bonus` y `switch_threshold`, `audit_behavior` falla con `hysteresis should keep switch count small` |
| 11 | Rendimiento | **PASS como línea base** | throughput publicado para 100 / 1.000 / 10.000 agentes; no se fija todavía un SLA mínimo |

## CTest solicitado por el auditor

Comando ejecutado:

```bash
ctest --test-dir build -C Release --output-on-failure -V
```

Resultado final:

```text
100% tests passed, 0 tests failed out of 14
Total Test time (real) =   0.15 sec
```

La salida verbose completa está en [`MSPA-1.1.2-CTEST-VERBOSE.md`](./MSPA-1.1.2-CTEST-VERBOSE.md).

## Rendimiento observado

Todos los casos ejecutan 100.000 operaciones totales; al crecer la población se reduce el número de rondas para mantener comparable el volumen de trabajo.

| Operación | Agentes | Operaciones | Tiempo (s) | ops/s |
|---|---:|---:|---:|---:|
| OBSERVE | 100 | 100.000 | 2.859249 | 34.974,22 |
| DECIDE | 100 | 100.000 | 2.825029 | 35.397,86 |
| OBSERVE | 1.000 | 100.000 | 2.854302 | 35.034,83 |
| DECIDE | 1.000 | 100.000 | 2.809878 | 35.588,73 |
| OBSERVE | 10.000 | 100.000 | 2.814781 | 35.526,75 |
| DECIDE | 10.000 | 100.000 | 2.831685 | 35.314,66 |

En este runner el throughput se mantiene aproximadamente plano, alrededor de **35k operaciones/s**, entre 100 y 10.000 agentes. Esto es una línea base, no una promesa de rendimiento ni un límite recomendado todavía.

Salida exacta:

```text
BENCH op=OBSERVE agents=100 operations=100000 seconds=2.859249 ops_per_sec=34974.22
BENCH op=DECIDE agents=100 operations=100000 seconds=2.825029 ops_per_sec=35397.86
BENCH op=OBSERVE agents=1000 operations=100000 seconds=2.854302 ops_per_sec=35034.83
BENCH op=DECIDE agents=1000 operations=100000 seconds=2.809878 ops_per_sec=35588.73
BENCH op=OBSERVE agents=10000 operations=100000 seconds=2.814781 ops_per_sec=35526.75
BENCH op=DECIDE agents=10000 operations=100000 seconds=2.831685 ops_per_sec=35314.66
```

## Fuzzing y entrada hostil

El test determinista de entrada hostil produjo:

```text
AUDIT hostile/time PASS fuzz_rejected=500 fuzz_accepted=0 bytes=152
```

El harness libFuzzer se ejecutó 5.000 veces:

```text
INFO: Running with entropic power schedule (0xFF, 100).
INFO: Seed: 528277021
INFO: Loaded 1 modules   (1342 inline 8-bit counters): 1342 [0x5560e9c76308, 0x5560e9c76846), 
INFO: Loaded 1 PC tables (1342 PCs): 1342 [0x5560e9c76848,0x5560e9c7bc28), 
INFO: -max_len is not provided; libFuzzer will not generate inputs larger than 4096 bytes
INFO: A corpus is not provided, starting from an empty corpus
#2	INITED cov: 120 ft: 121 corp: 1/1b exec/s: 0 rss: 44Mb
#5000	DONE   cov: 120 ft: 121 corp: 1/1b lim: 53 exec/s: 0 rss: 46Mb
Done 5000 runs in 0 second(s)
```

Esto demuestra ausencia de crash/UB observado en ese smoke run, no ausencia matemática de vulnerabilidades. El fuzzing continuo sigue siendo útil.

## Mutation testing de histéresis

El workflow reemplazó deliberadamente ambas funciones de `HysteresisPolicy` por retornos `0.0f`, recompiló y ejecutó la prueba conductual. Resultado del test mutado:

```text
FAIL: hysteresis should keep switch count small
```

El workflow interpretó ese fallo como `MUTATION_KILLED: hysteresis regression was detected`, por lo que la prueba de histéresis sí es sensible a la desaparición de la funcionalidad.

## Traza de cuatro perfiles

La ejecución completa estímulo -> inferencia -> appraisal -> emoción/mood -> candidatos -> selección está preservada en [`MSPA-1.1.2-TRACE-FOUR-PROFILES.md`](./MSPA-1.1.2-TRACE-FOUR-PROFILES.md).

Resultados destacados:

| Perfil del escenario de auditoría | Certeza observada | Certeza inferida | Urgencia | Acción elegida |
|---|---:|---:|---:|---:|
| cautious | 0.6014 | 0.5212 | 3 | 1 |
| curious | 0.8379 | 0.7103 | 2 | 1 |
| stable-bold | 0.7160 | 0.6128 | 2 | 1 |
| social-sensitive | 0.6831 | 0.5865 | 3 | 1 |

Que los cuatro elijan la misma acción en este escenario no significa que sean equivalentes: las certezas, appraisal, afecto, utilidad y confianza difieren. El test específico de divergencia usa además perfiles extremo rígido/flexible y exige relaciones relativas, no IDs de acción distintos por obligación.

## Archivos de prueba para revisión externa

- [audit_determinism_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/audit_determinism_tests.cpp)
- [audit_behavior_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/audit_behavior_tests.cpp)
- [audit_security_time_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/audit_security_time_tests.cpp)
- [audit_trace_scenario_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/audit_trace_scenario_tests.cpp)
- [emotion_baseline_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/emotion_baseline_tests.cpp)
- [persistence_tests.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/persistence_tests.cpp)
- [security_tests.cpp](https://glithub.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/tests/security_tests.cpp)
- [audit.yml](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/.github/workflows/audit.yml)
- [benchmark.cpp](https://github.com/glitchaka/MSPA-Lib/blob/9c0a8f740fdac51dba886a19e2c0456d1349d95e/benchmarks/benchmark.cpp)

## Límites de esta auditoría

Esta auditoría **no certifica la capa cultural/social** que el auditor señaló como ausente. A fecha de este corte siguen fuera de la cobertura funcional demostrada:

- aprendizaje observacional del resultado de otros agentes;
- normas percibidas de grupo;
- salida explícita de afirmaciones/rumores para transmisión de creencias;
- métricas poblacionales de deriva cultural, diversidad o polarizacións;
- relaciones cuyo destino no sea un `AgentId`.

Estas capacidades deben figurar como GAP/N/A hasta que exista contrato, implementación y prueba. No se consideran aprobadas por el hecho de que el núcleo psicológico pase esta auditoría.
