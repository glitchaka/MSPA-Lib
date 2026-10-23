# MSPA Interaction Contract v0.2

> Contrato de interacción implementado, agnóstico al host.

## 1. Runtime

Cada `Runtime` posee identidad, mundo, `TimeDomain`, seed raíz, límites, registry snapshot, agentes y diagnostics. Varias instancias pueden coexistir. El host serializa mutaciones concurrentes sobre una misma instancia.

## 2. Lifecycle

### `CREATE_AGENT`

Entrada: `AgentId`, `AgentInit`, `process_time`. El ID debe pertenecer al `WorldId` del runtime. Se validan personalidad, cuotas, estado inicial y RNG. `AgentInit::use_registry_default_values` aplica valores iniciales declarados por packs que no hayan sido sobreescritos explícitamente.

### `CREATE_AGENT_DERIVED`

Deriva personalidad/valores desde agentes fuente mediante políticas deterministas y variación seeded. No presupone parentesco.

### `REMOVE_AGENT`

`ForgetRuntimeOnly` elimina el estado activo. `ValidateNoReferences` rechaza la operación si otros estados conservan referencias al agente.

### `HAS_AGENT`

Consulta read-only.

## 3. OBSERVE

Entrada: `ObserveInput` con agente, proposición, `StimulusTime`, certeza percibida, confianza de fuente y procedencia.

Garantías:
- `process_time` monotónico;
- `event_time` puede ser histórico;
- proposición validada contra schema y cuotas;
- quota de creencias/memoria respetada antes de mutar;
- belief update, inferencia, appraisal, afecto y memoria forman el pipeline psicológico.

Salida `ObserveResult`: cambio de estado, recomendación de decisión, urgencia, amenaza a acción actual y token diagnóstico.

## 4. DECIDE

Entrada: agente, `ActionOption[]`, `DecisionContext`, `process_time`, continuidad actual opcional.

Cada opción puede declarar coste, riesgo, progreso, novedad, exposición social, interrupción, estabilidad, tags, efectos esperados y señales. `DecisionContext` puede declarar necesidades genéricas y señales externas.

MSPA selecciona únicamente una instancia recibida. La puntuación puede usar personalidad, valores, metas, relaciones, emociones/mood, aprendizaje y consideraciones del registry. Histéresis aplica continuidad y umbral de cambio.

## 5. REPORT

El host reporta el resultado real: acción, éxito, tags de contexto/resultado, proposiciones resultantes, impactos relacionales y `process_time`. MSPA actualiza aprendizaje y estado psicológico; no inventa el resultado físico.

## 6. ADVANCE_TO

Materializa decaimiento/consolidación perezosos y devuelve si cambió estado o conviene reevaluar decisión. Retroceder el tiempo produce `TimeRegression`.

## 7. REACT

Wrapper de `OBSERVE + DECIDE`; no crea una semántica adicional.

## 8. Batch

`observe_batch`, `decide_batch`, `report_batch`, `advance_batch` ordenan por `(AgentId,input_sequence)`. Duplicados o tiempos incompatibles para el mismo agente producen `BatchConflict`. Cambios de registry se publican fuera del batch.

## 9. Registry / packs

`begin_registry_staging` crea staging desde el snapshot actual. `stage_domain_pack` valida pack y dependencias/versiones. `publish_registry` hace visible el nuevo snapshot. `discard_registry_staging` descarta staging.

## 10. Export / import

Export contiene estado, dominio temporal y `exported_at`. Import valida identidad, límites, schema, tiempo y RNG. Políticas temporales: `RebaseRelative`, `PreserveAbsolute`, `RejectIncompatible`. Políticas RNG: `RekeyForDestination`, `PreserveState`.

La forma binaria usa `export_agent_bytes` / `import_agent_bytes` y serialización versionada.

## 11. Errores

C++ devuelve `Result<T>`. No se propagan excepciones a través de fronteras públicas. C ABI devuelve `mspa_status`.

## 12. Scheduling

MSPA puede recomendar `None`, `Normal`, `Soon` o `Immediate`; el host conserva siempre la decisión de cuándo ejecutar/reprogramar al agente.
