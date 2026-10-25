# MSPA 1.1.2 — Plan de auditoría conductual externa

Este documento traduce la lista de pruebas solicitada por el auditor externo a evidencia ejecutable dentro del repositorio. La auditoría se ejecuta sobre el código real; no se considera suficiente que una capacidad aparezca en README o documentación.

## Alcance

Se comprueban determinismo, divergencia entre perfiles, histéresis, aprendizaje, actualización de creencias, baseline emocional, coherencia explicativa, resistencia a entrada hostil, tiempo/importación, sensibilidad de la propia suite y rendimiento.

La capa cultural/social señalada por el auditor —aprendizaje observacional, normas percibidas, transmisión explícita de creencias y métricas poblacionales— no se declara cubierta por estas pruebas. Si no existe en la API, aparecerá en la matriz de conformidad como capacidad pendiente, no como PASS ficticio.

## Matriz de pruebas solicitadas

| # | Solicitud | Evidencia ejecutable | Criterio de aceptación |
|---|---|---|---|
| 1 | Determinismo: mismo escenario -> mismos bytes | `tests/audit_determinism_tests.cpp` | Dos runtimes equivalentes producen `export_agent_bytes` idénticos |
| 1 | Otro agente no altera la secuencia/estado | `tests/audit_determinism_tests.cpp` | El estado exportado del agente objetivo es byte-a-byte idéntico con y sin otro agente procesado |
| 1 | Batch barajado == canónico | `tests/audit_determinism_tests.cpp` | Estado final exportado idéntico |
| 1 | Batch == llamadas individuales | `tests/audit_determinism_tests.cpp` | Estado final exportado idéntico |
| 2 | Divergencia entre perfiles | `tests/audit_behavior_tests.cpp` | Misma evidencia produce certezas relativas distintas; perfil estable reduce miedo más rápido |
| 3 | Histéresis | `tests/audit_behavior_tests.cpp` | Near-tie de 100 decisiones: pocos cambios con continuidad y muchos sin ella |
| 4 | Bucle de aprendizaje | `tests/audit_behavior_tests.cpp` | Éxitos elevan expectativa; fallos la reducen; `DECIDE` favorece lo aprendido; contexto especializa patrones |
| 5 | Creencias y reapreciación | `tests/audit_behavior_tests.cpp` | Contradicción reduce certeza y puede invertir polaridad; carga emocional ralentiza; reapreciación no reescribe hechos |
| 6 | Baseline emocional | `tests/audit_behavior_tests.cpp`, `tests/emotion_baseline_tests.cpp` | Seis emociones >= 1%; baseline no genera urgencia, no altera utilidad por sí solo ni se expresa como episodio |
| 7 | Explicación coherente | `tests/audit_behavior_tests.cpp` | Suma de contribuciones = utilidad; candidato vencedor de la traza = acción seleccionada, incluyendo histéresis |
| 8 | Entrada hostil | `tests/audit_security_time_tests.cpp`, fuzzer | Truncado, longitudes absurdas, versión desconocida y cuotas producen errores controlados; fuzz smoke no cae |
| 9 | Tiempo/import | `tests/audit_security_time_tests.cpp` | `process_time` regresivo -> `TimeRegression`; `event_time` antiguo aceptado; `RebaseRelative` conserva intervalos reales |
| 10 | Que las pruebas prueben algo | `.github/workflows/audit.yml` | Se compila una mutación con histéresis anulada y la suite debe fallar; si pasa, el job falla |
| 11 | Rendimiento | `benchmarks/benchmark.cpp` | Publica ops/s de `OBSERVE` y `DECIDE` para 100, 1.000 y 10.000 agentes |

## Evidencia que genera CI

El workflow `.github/workflows/audit.yml` produce:

- `audit-ctest.log`: salida de `ctest --test-dir build -C Release --output-on-failure -V`;
- `audit-benchmark.log`: tasas de `OBSERVE` y `DECIDE`;
- `audit-fuzz.log`: ejecución libFuzzer de 5.000 entradas;
- `audit-mutation.log`: fallo esperado de la suite cuando se elimina deliberadamente la histéresis.

El escenario `mspa_audit_trace_scenario_tests` imprime en la salida verbose:

1. estímulo;
2. personalidad del perfil;
3. resultado de `OBSERVE`;
4. appraisal diagnóstico;
5. creencias, incluida inferencia;
6. emociones con baseline y exceso episódico;
7. mood;
8. candidatos y cada consideración;
9. acción seleccionada y confianza.

## Política de resultados

- **PASS**: el criterio fue ejecutado y se cumplió.
- **FAIL**: el criterio fue ejecutado y no se cumplió; debe corregirse la librería y repetirse.
- **N/A**: la capacidad pedida no existe en el contrato actual y no se presenta como implementada.
- **PENDIENTE**: existe una prueba diseñada, pero aún no hay evidencia de ejecución del commit auditado.

El informe final se escribe después de ejecutar CI, con el SHA exacto, salida relevante y URLs de los tests.
