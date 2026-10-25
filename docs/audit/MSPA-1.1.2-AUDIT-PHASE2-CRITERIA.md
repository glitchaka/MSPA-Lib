# MSPA 1.1.2 — Auditoría fase 2: criterios congelados

## Alcance y regla de independencia

Este documento congela los criterios **antes de ejecutar** la fase 2. La lista recibida llevaba el rótulo «Auditoría MSPA 1.1.1»; se aplica al estado actual 1.1.2 sin alterar ningún umbral.

Las pruebas de esta fase deben derivarse del README y de los contratos normativos y consumir únicamente headers/API públicos instalables (`include/mspa/**`) o la ABI C (`mspa/c_api.h`). No pueden incluir archivos de `src/`, acceder a símbolos privados ni depender de detalles no publicados para decidir PASS/FAIL.

Si una prueba falla, se corrige la librería o se registra FAIL. Un criterio sólo puede cambiar mediante un commit posterior que explique el motivo; no se modifica silenciosamente después de observar resultados.

Plataformas mínimas: MSVC Release; GCC Release; Clang con ASan+UBSan.

## Grupo A — promesas auditables

### A1 Determinismo

1. Repetir el mismo escenario produce bytes idénticos en `export_agent_bytes`.
2. Añadir/quitar otro agente no altera la secuencia observable ni el estado final del agente bajo prueba.
3. Batch con entradas barajadas produce el mismo resultado que el orden canónico.
4. Batch y llamadas individuales producen el mismo estado final.

### A2 Divergencia entre perfiles

Misma evidencia para cuatro perfiles distintos. Sólo aserciones relativas, nunca números exactos. Debe existir divergencia ordenada en certeza, intensidad emocional y/o recuperación coherente con estabilidad/persistencia/curiosidad/audacia. En particular, mayor estabilidad debe recuperar un episodio negativo antes que menor estabilidad bajo condiciones equivalentes.

### A3 Histéresis

100 `DECIDE` near-tie con perturbación alternante pequeña. Con `ActionContinuityKey` constante:

- cambios de acción <= 10 de 99 transiciones;
- cambios al menos 3 veces menores que sin continuidad.

### A4 Aprendizaje

Éxitos repetidos elevan expectativa y hacen que `DECIDE` favorezca la acción; fallos repetidos la reducen. El mismo `ActionType` con contextos distintos debe aprender expectativas diferenciadas por `ContextTags`.

### A5 Creencias y reapreciación

Evidencia contradictoria creciente reduce certeza y, al superar el soporte previo, invierte polaridad. Mayor rigidez o mayor carga emocional debe ralentizar el cambio. Reapreciar una memoria no puede cambiar su `Proposition` factual.

### A6 Baseline emocional

Tras un `ADVANCE_TO` largo:

- existen las seis emociones base;
- cada intensidad es >= 0.01;
- el tono residual por sí solo no produce urgencia `Soon` o `Immediate`;
- el tono residual por sí solo no cambia la acción elegida frente a un estado equivalente sin episodio emocional.

### A7 Explicabilidad y diagnostics

La suma de contribuciones de cada candidato reproduce su utilidad y la traza justifica el ganador. Activar/desactivar diagnostics no cambia selección, estado exportado ni consumo observable de RNG bajo escenario/seed idénticos.

### A8 Entrada hostil

Truncados, tamaños absurdos, versión desconocida, límites excedidos y corpus de fuzz deben terminar en error controlado, sin crash ni sanitizer finding. El smoke run de libFuzzer mínimo es 5.000 ejecuciones.

### A9 Tiempo e import

- `process_time` regresivo -> `TimeRegression`.
- `event_time` anterior -> aceptado.
- `RebaseRelative` entre escalas distintas conserva edades relativas de todos los timestamps auditados.
- `RekeyForDestination` rederiva RNG.
- dos destinos distintos importados del mismo origen no comparten `RngState`.

### A10 Serialización y migración

Round-trip binario exacto de un agente complejo. Debe existir y probarse una ruta de migración desde al menos una versión anterior soportada hasta `state_schema_version` actual.

### A11 Lifecycle y aislamiento

Runtimes simultáneos no interfieren. Se prueban `DuplicateAgent`, `REMOVE_AGENT(ValidateNoReferences)` y rechazo de `AgentId` perteneciente a otro mundo.

### A12 ABI C

Los escenarios equivalentes de A1, A4 y A9 ejecutados por ABI C deben producir el mismo resultado observable que C++. Ninguna llamada C deja escapar una excepción.

### A13 Cuotas

Se verifica cada campo de `RuntimeLimits`: agentes, values, beliefs, emotions, memories, goals, relationships, learning patterns, proposition args, string bytes, serialized bytes y diagnostics. Exceder el límite aplicable debe producir `QuotaExceeded` o, cuando el propio contrato de creación del límite es inválido, `InvalidArgument`, sin corromper el estado previo. `max_emotions_per_agent < 6` debe rechazarse. Un Domain Pack no puede elevar límites del runtime.

### A14 Rendimiento

Medición, no gate: `OBSERVE` y `DECIDE` con 100, 1.000 y 10.000 agentes. Se registran compilador, plataforma, operaciones, segundos y ops/s. No hay umbral PASS de rendimiento.

### A15 Mutation testing

Se introduce una regresión deliberada, una por vez, en:

1. histéresis;
2. baseline emocional;
3. rebase temporal;
4. validación de cuotas;
5. orden canónico de batch;
6. reapreciación factual.

Cada mutante debe ser detectado por al menos una prueba. Si sobrevive, la fila correspondiente se reporta como `FAIL / SIN COBERTURA`.

## Grupo B — capacidades no implementadas

Estas filas no pueden marcarse PASS por inferencia ni por tests indirectos. Deben aparecer como **NOT IMPLEMENTED** con referencia normativa y sin test ficticio:

- aprendizaje observacional de resultados de otros agentes;
- normas percibidas de grupo;
- afirmaciones/rumores transmisibles y expresión explícita en salida de `DECIDE`;
- relaciones cuyo destino no sea un agente;
- métricas poblacionales de deriva cultural, diversidad o polarización.

## Formato del informe

Por prueba: requisito, escenario, inputs, esperado, obtenido, PASS/FAIL, commit auditado, compilador/plataforma y test automatizado.

Entregable principal: matriz `requisito normativo -> módulo/API -> test -> resultado -> commit`. Requisitos sin cobertura deben figurar como `SIN COBERTURA`; Grupo B debe figurar como `NOT IMPLEMENTED`.
